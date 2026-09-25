bl_info = {
    "name": "Custom Exporter",
    "author": "Jim",
    "version": (0, 1, 0),
    "blender": (4, 0, 0),
    "location": "File > Export > Custom",
    "category": "Import-Export",
}

import bpy
import json
from bpy.types import Operator
from bpy.props import StringProperty, BoolProperty
from mathutils import Vector

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
        print(slot.link, mat.name if mat else None)
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

def do_export(context=None, directory="./", platform="PC", pak_file_json_path="pak.json"):
    if context == None:
        context = bpy.context
    meshes = {}
    materials = {}
    mesh_instances = []
    for obj in bpy.context.scene.objects:
        print(obj.name, obj.type)
        mat = obj.matrix_world
        loc, rot, scale = mat.decompose()
        
        if obj.type == 'MESH':
            mesh = obj.data
            obj_materials = get_object_materials(obj)
            d = get_material_data(obj_materials[0])
            print(d)
            materials[d["name"]] = d
            if mesh.library:
                meshes[mesh.library.filepath] = mesh
            else:
                assert False, "all meshes in scene must be linked!"
            mesh_instances.append({
                "pos": tuple(loc),
                "rot": tuple(rot),
                "scale": tuple(scale),
                "children": [child.name for child in obj.children],
                "material": d["name"],
                "mesh": mesh.library.filepath
            })
            
    meshes = {k: get_flat_triangle_data(v) for k, v in meshes.items()}
    print(f"meshes: {meshes}\n\n")
    print(f"materials: {materials}\n\n")
    print(f"mesh_instances: {mesh_instances}\n\n")


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