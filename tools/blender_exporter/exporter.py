bl_info = {
    "name": "Custom Exporter",
    "author": "Jim",
    "version": (0, 1, 0),
    "blender": (4, 0, 0),
    "location": "File > Export > Custom",
    "category": "Import-Export",
}

import bpy
from bpy.types import Operator
from bpy.props import StringProperty, BoolProperty


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
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


def menu_func_export(self, context):
    self.layout.operator(EXPORT_OT_minimal_folder.bl_idname, text="Custo")


def register():
    bpy.utils.register_class(EXPORT_OT_minimal_folder)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    bpy.utils.unregister_class(EXPORT_OT_minimal_folder)


if __name__ == "__main__":
    register()