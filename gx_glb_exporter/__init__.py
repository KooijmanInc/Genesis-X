bl_info = {
    "name": "GX GLB Exporter",
    "author": "Kooijman Incorporate Holding B.V.",
    "version": (0, 1, 0),
    "blender": (4, 0, 0),
    "location": "View3D > Sidebar > Genesis-X",
    "description": "Export GLB with optional GX PBR Mixer steps (incremental).",
    "category": "Import-Export",
}

import bpy
from bpy.props import StringProperty, PointerProperty
from bpy.types import Operator, Panel, PropertyGroup
from bpy_extras.io_utils import ExportHelper


class GXExportSettings(PropertyGroup):
    export_path: StringProperty(
        name="Export Path",
        description="Where to write the GLB file",
        default="",
        subtype="FILE_PATH",
    )


class GX_OT_export_glb(Operator, ExportHelper):
    bl_idname = "gx.export_glb"
    bl_label = "Export GX GLB"
    bl_options = {"REGISTER", "UNDO"}

    filename_ext = ".glb"
    filter_glob: StringProperty(default="*.glb", options={"HIDDEN"})

    def execute(self, context):
        # Store chosen path back into scene settings
        context.scene.gx_export_settings.export_path = self.filepath

        # For now: just prove the operator runs
        self.report({"INFO"}, f"Would export to: {self.filepath}")
        print("[GX] Export requested:", self.filepath)

        # Later steps: call glTF exporter here, or run PBR mixer pipeline, etc.
        return {"FINISHED"}


class GX_PT_export_panel(Panel):
    bl_label = "GX GLB Exporter"
    bl_idname = "GX_PT_export_panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Genesis-X"

    def draw(self, context):
        layout = self.layout
        s = context.scene.gx_export_settings

        layout.prop(s, "export_path")
        layout.operator(GX_OT_export_glb.bl_idname, icon="EXPORT")


classes = (
    GXExportSettings,
    GX_OT_export_glb,
    GX_PT_export_panel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.gx_export_settings = PointerProperty(type=GXExportSettings)


def unregister():
    del bpy.types.Scene.gx_export_settings
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
