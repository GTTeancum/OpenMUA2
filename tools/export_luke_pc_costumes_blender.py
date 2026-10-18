"""Export Luke's four PC costumes through the patched igb-blender add-on.

Run with Blender --background --factory-startup --python this_file -- arguments.
"""
import argparse
import importlib.util
import json
import sys
from pathlib import Path

import bpy

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--addon', type=Path, required=True)
parser.add_argument('--actors', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
addon = args.addon.resolve()
output = args.output.resolve()
repo = Path(__file__).resolve().parents[1]
if not output.is_relative_to(repo) or not addon.is_relative_to(repo):
    raise ValueError('Add-on checkout and output must remain inside this repository')
output.mkdir(parents=True, exist_ok=True)
(output / 'temp').mkdir(exist_ok=True)
bpy.context.preferences.filepaths.temporary_directory = str(output / 'temp')

spec = importlib.util.spec_from_file_location(
    'igb_blender', addon / '__init__.py', submodule_search_locations=[str(addon)])
module = importlib.util.module_from_spec(spec)
sys.modules['igb_blender'] = module
spec.loader.exec_module(module)
module.register()
from igb_blender.actor.actor_import import import_actor
from igb_blender.exporter.skin_export import export_skin

records = []
for index in range(4):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    source = args.actors / f'1570{index + 1}.igb'
    result = import_actor(
        bpy.context, str(source), skin_filepaths=[('default', str(source))],
        options={'import_animations': False, 'import_materials': True,
                 'game_preset': 'auto'})
    if result is None:
        raise RuntimeError(f'Actor import failed: {source}')
    armature, skins, actions = result
    meshes = [(obj, False) for name, obj in skins]
    if not meshes:
        raise RuntimeError(f'No meshes imported: {source}')
    path = output / f'{90000 + index}.igb'
    if not export_skin(
        str(path), meshes, armature, swap_rb=False, max_texture_size=256,
        igb_format='V6', texture_mode='wii_rgb5a3', use_normal_maps=False,
        use_specular_maps=False, use_gloss_map=False, use_mipmaps=False):
        raise RuntimeError(f'Skin export failed: {path}')
    records.append({
        'source': str(source), 'output': str(path),
        'bones': len(armature.data.bones), 'meshes': len(meshes),
        'vertices': sum(len(obj.data.vertices) for obj, _ in meshes),
        'polygons': sum(len(obj.data.polygons) for obj, _ in meshes),
        'bytes': path.stat().st_size,
    })
    (output / 'export.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
print('OPENMUA2_EXPORT_COMPLETE', flush=True)
