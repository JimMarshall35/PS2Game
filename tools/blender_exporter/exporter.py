bl_info = {
    "name": "Custom Exporter",
    "author": "Jim",
    "version": (0, 1, 0),
    "blender": (4, 0, 0),
    "location": "File > Export > Custom",
    "category": "Import-Export",
}

import os
import bpy
import json
from bpy.types import Operator
from bpy.props import StringProperty, BoolProperty
from mathutils import Vector
import subprocess
from pathlib import Path
import struct
import zlib

def get_flat_triangle_data(mesh):
    #eval_obj = obj.evaluated_get(depsgraph)
    #mesh = eval_obj.to_mesh()

    mesh.calc_loop_triangles()
    mesh.calc_normals_split()

    uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None

    verts_out = []  # flat list of (pos, normal, uv) - 3 per triangle

    for tri in mesh.loop_triangles:
        for i in range(3):
            loop_index = tri.loops[i]
            vert_index = tri.vertices[i]

            pos = tuple(mesh.vertices[vert_index].co.copy())
            normal = tuple(Vector(tri.split_normals[i]))
            uv = tuple(uv_layer[loop_index].uv.copy() if uv_layer else (0.0, 0.0))

            verts_out.append({"pos":pos, "normal":normal, "uv":uv})

    #eval_obj.to_mesh_clear()
    return verts_out

def get_color_or_texture(input_socket):
    """Given a color input socket, return either a texture path or an RGBA value."""
    if input_socket.is_linked:
        linked_node = input_socket.links[0].from_node
        if linked_node.type == 'TEX_IMAGE' and linked_node.image:
            return {"texture": bpy.path.abspath(linked_node.image.filepath)}
        else:
            # linked to something else entirely - recurse or just report the node type
            return {"linked_to": linked_node.type}
    else:
        return {"color": tuple(input_socket.default_value)}
    
def get_object_materials(obj):
    materials = []
    for slot in obj.material_slots:
        mat = slot.material
        materials.append(mat)
        #print(slot.link, mat.name if mat else None)
    return materials


def get_material_data(mat):
    data = {"name": mat.name}
    if mat.use_nodes:
        for node in mat.node_tree.nodes:
            if node.type == 'MIX':
                data = data | get_color_or_texture(node.inputs['A'])
                data = data | get_color_or_texture(node.inputs['B'])
            if node.type == "TEX_IMAGE":
                data = data | { "texture" : bpy.path.abspath(node.image.filepath) }      
    else:
        data["color"] = tuple(mat.diffuse_color)
    return data

def make_texture(texture_tool, input_texture_path, output_texture_path, platform):
    "Returns the list of paths that have been created. It could output only a texture or a texture and a CLUT"
    print(f"outputting texture to: {output_texture_path}, platform {platform}")
    cmd = [ texture_tool, "-p", input_texture_path, "-o", output_texture_path ]
    generated_files = [output_texture_path]
    if platform == "PC":
        cmd += "--PC"
        pass
    elif platform == "PS2":
        out_path = Path(output_texture_path)
        cmd += "--outColourLUT"
        cmd += f"{out_path.with_suffix(".clut")}"
        generated_files += f"{out_path.with_suffix(".clut")}"
        pass
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True
    )
    if len(result.stdout) != 0:
        print(result.stdout)
    if len(result.stderr) != 0:
        print(result.stderr)
    
    if result.returncode != 0:
        print("Exit code:", result.returncode)
    return generated_files

def write_textures(output_dir, pak_file_data, materials, texture_tool_path, platform):
    for mat_name in materials:
        mat = materials[mat_name]
        if "texture" in mat:
            tex_path = mat["texture"]
            new_texture_name = Path(tex_path).stem + ".texture"
            output_path = os.path.join(
                output_dir,
                new_texture_name
            )

            # update the material so that it references the name as it will be in the pak file
            mat["texture"] = new_texture_name

            # is there already a texture by this name in the pak file data?
            texture_present = any([x for x in pak_file_data["files"] if x["name"] == new_texture_name])
            if texture_present:
                continue

            new_files = make_texture(
                texture_tool_path,
                tex_path,
                output_path,
                platform
            )
            for f in new_files:
                if not any([x for x in pak_file_data["files"] if x["name"] == Path(f).name]):
                    pak_file_data["files"].append({
                        "path": f,
                        "name": Path(f).name
                    })

def transform_blender_file_name_to_mesh_file_name(blender_file_name):
    return blender_file_name.replace("//", "").replace(".blend", ".mesh")

def write_mesh(mesh, output_path):
    """
        Makes meshes of this format:
                 (HEADER)                                     (FOR EACH VERTEX)
        | u32                                  | f32 | f32 | f32 | f32 | f32 | f32 | f32 | f32 |
        | number of verts (triangle primitive) | X   | Y   | Z   | NX  | NY  | NZ  | U   | V   |
    """
    print(f"outputting mesh to {output_path}")
    data = b""
    data += struct.pack("I", len(mesh)) # number of vertices
    for vert in mesh:
        data += struct.pack("f", vert["pos"][0])
        data += struct.pack("f", vert["pos"][1])
        data += struct.pack("f", vert["pos"][2])
        data += struct.pack("f", vert["normal"][0])
        data += struct.pack("f", vert["normal"][1])
        data += struct.pack("f", vert["normal"][2])
        data += struct.pack("f", vert["uv"][0])
        data += struct.pack("f", vert["uv"][1])
    with open(output_path, "wb") as f:
        f.write(data)

def write_meshes(output_dir, pak_data, meshes):
    for mesh_name in meshes:
        mesh = meshes[mesh_name]
        output_file_name = transform_blender_file_name_to_mesh_file_name(mesh_name)
        output_path = os.path.join(output_dir, output_file_name)
        write_mesh(mesh, output_path)
        if not any([x for x in pak_data["files"] if x["name"] == output_file_name]):
            pak_data["files"].append({
                "path": output_path,
                "name": output_file_name
            })

def write_fixed_size_string(data, string, size):
    assert len(string) < size, f"string {string} to big for buffer of size {size}"
    for i in range(size):
        if i < len(string):
            data += struct.pack("B", ord(string[i]))
        else:
            data += struct.pack("B", ord('\0'))

def hash32(s: str) -> int:
    return zlib.crc32(s.encode("utf-8"))

def write_scene(scene_name, output_dir, pak_data, mesh_instances, materials):
    """
    Header:    | u32 (num instances) |

    (per instance begin)
    Transform:   | f32 posx | f32 posy | f32 posz | f32 quatx | f32 quaty | f32 quatz | f32 quatw | f32 scalex | f32 scaley | f32 scalez
    Mesh:        | u8[32] name (C string)    |
    Material:    | u8[32] name (C string)    |
    Name:        | u8[32] name (C string)    |
    NameHash:    | u32 |
    NumChildren: | u32 |
    Children:    | u32[32] child name hashes |   <- an object can have a maximum of 32 children 
    (per instance end)

    Header:    | u32 (num materials)
    (per material begin)
    Flags:     | u32               | bits: 0: has texture 1: has colour
    Name:      | u8[32] (C string) |
    Texture:   | u8[32] (C string) |
    Colour:    | f32 r | f32 g | f32 b | f32 a |
    (per material end)
    """
    out_file_path = os.path.join(output_dir, f"{scene_name}.scene")
    print(f"outputting scene file to {out_file_path}")
    data = b""
    data += struct.pack("I", len(mesh_instances)) # number of instances
    for instance in mesh_instances:
        # transforms
        data += struct.pack("f", instance["pos"][0])
        data += struct.pack("f", instance["pos"][1])
        data += struct.pack("f", instance["pos"][2])
        data += struct.pack("f", instance["rot"][0])
        data += struct.pack("f", instance["rot"][1])
        data += struct.pack("f", instance["rot"][2])
        data += struct.pack("f", instance["rot"][3])
        data += struct.pack("f", instance["scale"][0])
        data += struct.pack("f", instance["scale"][1])
        data += struct.pack("f", instance["scale"][2])

        # mesh material and name
        write_fixed_size_string(data, instance["mesh"], 32)
        write_fixed_size_string(data, instance["material"], 32)
        write_fixed_size_string(data, instance["name"], 32)
        data += struct.pack("I", hash32(instance["name"]))

        # children
        data += struct.pack("I", len(instance["children"]))
        for i in range(32):
            if i < len(instance["children"]):
                child_hash = hash32(instance["children"][i])
                data += struct.pack("I", child_hash)
            else:
                data += struct.pack("I", 0)
    data += struct.pack("I", len(materials)) # number of materials
    for matname in materials:
        mat = materials[matname]
        flags = 0
        if "texture" in mat:
            flags |= 1
        if "color" in mat:
            flags |= 2
        data += struct.pack("I", flags)
        write_fixed_size_string(data, mat["name"], 32)
        if "texture" in mat:
            write_fixed_size_string(data, mat["texture"], 32)
        else:
            write_fixed_size_string(data, "", 32)
        if "color" in mat:
            data += struct.pack("f", mat["color"][0])
            data += struct.pack("f", mat["color"][1])
            data += struct.pack("f", mat["color"][2])
            data += struct.pack("f", mat["color"][3])
        else:
            data += struct.pack("f", 1.0)
            data += struct.pack("f", 1.0)
            data += struct.pack("f", 1.0)
            data += struct.pack("f", 1.0)
    
    with open(out_file_path, "wb") as f:
        f.write(data)
        
    if not any([x for x in pak_data["files"] if x["name"] == f"{scene_name}.scene"]):
        pak_data["files"].append({
            "path": out_file_path,
            "name": f"{scene_name}.scene"
        })


def write_data_files(output_dir, 
                     platform, 
                     pak_file_json_path, 
                     texture_tool_path, 
                     meshes,
                     materials,
                     mesh_instances,
                     scene_name):
    pak_data = None
    if Path(pak_file_json_path).exists():
        with open(pak_file_json_path) as f:
            pak_data = json.load(f)
    else:
        pak_data = {
            "files": []
        }
    write_textures(output_dir, pak_data, materials, texture_tool_path, platform)
    write_meshes(output_dir, pak_data, meshes)
    write_scene(scene_name, output_dir, pak_data, mesh_instances, materials)
    manifest_str = json.dumps(pak_data, indent=2)
    print(f"\nManifest files: {len(pak_data["files"])}\nManifest:\n\n{manifest_str}\n")
    with open(pak_file_json_path, "w") as f:
        f.write(manifest_str)

def do_export(context=None, directory="./", platform="PC", pak_file_json_path="pak.json", texture_tool_path=None):
    if context == None:
        context = bpy.context
    if texture_tool_path == None:
        script_path = os.path.realpath(__file__)
        dir_path = os.path.dirname(script_path)
        texture_tool_path = os.path.join(dir_path, "../", "texture_maker", "build", "TextureMaker")
    
    meshes = {}
    materials = {}
    mesh_instances = []
    for obj in bpy.context.scene.objects:
        #print(obj.name, obj.type)
        mat = obj.matrix_world
        loc, rot, scale = mat.decompose()
        
        if obj.type == 'MESH':
            mesh = obj.data
            obj_materials = get_object_materials(obj)
            d = get_material_data(obj_materials[0])
            #print(d)
            materials[d["name"]] = d
            if mesh.library:
                meshes[mesh.library.filepath] = mesh
            else:
                assert False, "all meshes in scene must be linked!"
            mesh_instances.append({
                "name": obj.name,
                "pos": tuple(loc),
                "rot": tuple(rot),
                "scale": tuple(scale),
                "children": [child.name for child in obj.children],
                "material": d["name"],
                "mesh": mesh.library.filepath
            })
            
    meshes = {k: get_flat_triangle_data(v) for k, v in meshes.items()}
    #print(f"meshes: {meshes}\n\n")
    #print(f"materials: {materials}\n\n")
    #print(f"mesh_instances: {mesh_instances}\n\n")
    blend_full_path = bpy.data.filepath
    blend_filename = os.path.basename(blend_full_path)              # "foo.blend"
    blend_name_only = os.path.splitext(blend_filename)[0]           # "foo"
    write_data_files(
        directory,
        platform,
        pak_file_json_path,
        texture_tool_path,
        meshes,
        materials,
        mesh_instances,
        blend_name_only
    )



class EXPORT_OT_minimal_folder(Operator):
    bl_idname = "export_scene.minimal_folder"
    bl_label = "Export Minimal Folder"

    # Note: property must be named "directory" - Blender's file browser
    # looks for this specific name to know it should offer folder selection.
    directory: StringProperty(subtype='DIR_PATH')

    # Hides file filtering UI, keeps the browser in "pick a folder" mode
    filter_folder: BoolProperty(default=True, options={'HIDDEN'})

    def execute(self, context):
        print("Exporting to folder:", self.directory)
        do_export(context, self.directory)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


def menu_func_export(self, context):
    self.layout.operator(EXPORT_OT_minimal_folder.bl_idname, text="Custom")


def register():
    bpy.utils.register_class(EXPORT_OT_minimal_folder)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    bpy.utils.unregister_class(EXPORT_OT_minimal_folder)


if __name__ == "__main__":
    register()