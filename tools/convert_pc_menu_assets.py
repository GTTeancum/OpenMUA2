"""Author private red/Wii menu candidates with igb-blender's complete graph writer.

Retain object identities, names, geometry, animation and package paths. Convert
v6 inline names to native v8, the transform animation field, image payloads and
format fields, and declared UI tint streams. Output is an
authoring candidate, not a claim of native rendering or an installed game pack.
"""
import argparse
import colorsys
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys


def field(writer, index, slot):
    return next(value for key, value, _ in writer.objects[index].raw_fields if key == slot)


def put(writer, index, slot, value):
    obj = writer.objects[index]
    if not any(key == slot for key, _, _ in obj.raw_fields):
        raise ValueError(f'Missing field {slot} in object {index}')
    obj.raw_bytes = None
    obj.raw_fields = [(key, value if key == slot else old, kind)
                      for key, old, kind in obj.raw_fields]


def memory(writer, index, data):
    obj = writer.objects[index]
    obj.data = data
    obj.raw_data = None
    entry = copy.deepcopy(writer.entries[writer.index_map[index]])
    entry.raw_bytes = None
    for i, definition in enumerate(writer.meta_objects[entry.type_index].fields):
        if definition.slot == 7:
            entry.field_values[i] = len(data)
    writer.index_map[index] = len(writer.entries)
    writer.entries.append(entry)
    writer.ref_info[index]['mem_size'] = len(data)


def red(rgb):
    hue, saturation, value = colorsys.rgb_to_hsv(*(c / 255 for c in rgb))
    if saturation < .18 or not 170 / 360 <= hue <= 260 / 360:
        return tuple(rgb)
    return tuple(round(c * 255) for c in colorsys.hsv_to_rgb(0, saturation, value))


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--writer-root', type=Path, required=True)
    p.add_argument('--input', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--native-models', type=Path, required=True)
    p.add_argument('--reference-scripts', type=Path, required=True)
    a = p.parse_args()
    repo = Path(__file__).resolve().parents[1]
    source, output, addon = a.input.resolve(strict=True), a.output.resolve(), a.writer_root.resolve(strict=True)
    native_models = a.native_models.resolve(strict=True)
    for path in (source, output, addon, native_models):
        if not path.is_relative_to(repo / '.local'):
            raise ValueError('Conversion inputs, writer and output must stay in repository/.local')
    if output.is_relative_to(source):
        raise ValueError('Output must be separate from the input')
    if output.exists():
        # Rebuild only our previously verified candidate, never arbitrary files
        # or user edits. No removal or backup copy is needed for an incremental run.
        previous = json.loads((output/'conversion-manifest.json').read_text())
        expected = {'conversion-manifest.json'} | {r['path'] for r in previous}
        if {p.relative_to(output).as_posix() for p in output.rglob('*') if p.is_file()} != expected:
            raise ValueError('Unexpected files in output directory')
        for row in previous:
            if hashlib.sha256((output/row['path']).read_bytes()).hexdigest() != row['output_sha256']:
                raise ValueError('Candidate was changed outside this converter')
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(addon))
    from igb_format.igb_reader import IGBReader
    from igb_format.igb_writer import from_reader
    graph = load_module('menu_graph_reference', a.reference_scripts.resolve(strict=True) / 'menu_igb_graph.py')
    from pc_menu_geometry import convert_geometry
    decoder = load_module('menu_image_decoder', addon / 'utils/image_convert.py')
    encoder = load_module('menu_wii_encoder', addon / 'utils/wii_rgb5a3.py')
    manifest = []
    for path in sorted(source.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in ('.igb', '.eng', '.pkgb'):
            continue
        if not path.resolve().is_relative_to(source):
            raise ValueError('Input resource escapes source tree')
        relative = path.relative_to(source)
        destination = output / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        original = path.read_bytes()
        receipt = {'path': relative.as_posix(), 'source_sha256': hashlib.sha256(original).hexdigest(),
                   'images': 0, 'vertex_tints': 0}
        if path.suffix.lower() != '.igb':
            destination.write_bytes(original)
        else:
            reader = IGBReader(str(path)); reader.read()
            writer = from_reader(reader)
            if writer.version != 6 or writer.endian != '<':
                raise ValueError(f'Unsupported graph format: {relative}')
            # MUA2 Wii assets use v8's name-pool string representation. All
            # objects must be reserialized; retaining a v6 raw object would
            # leave inline strings in a file declaring pooled names.
            writer.version = 8
            writer.name_pool = []
            writer._intern_name(b'')
            for obj in writer.objects:
                if hasattr(obj, 'raw_fields'):
                    obj.raw_bytes = None
            # Retail MUA2 v6 igTransform places the same igTransformSequence1_5
            # reference in slot 10; XML2 PC uses 11. Other shared scene schemas
            # matched the retail layout. Convert the declared field and every
            # instance together, preserving the animation object/reference.
            for meta in writer.meta_objects:
                if meta.name == b'igTransform':
                    if meta.slot_count != 12:
                        raise ValueError('Unexpected PC transform slot count')
                    meta.slot_count = 11
                    for definition in meta.fields:
                        if definition.slot == 11:
                            if definition.short_name != b'ObjectRef':
                                raise ValueError('Unexpected transform animation field')
                            definition.slot = 10
            for obj in writer.objects:
                if hasattr(obj,'raw_fields') and writer.meta_objects[obj.type_index].name == b'igTransform':
                    obj.raw_bytes = None
                    obj.raw_fields = [(10 if k==11 else k,v,t) for k,v,t in obj.raw_fields]
            # Reference-only button/controller artwork retains its original colors.
            tint = path.stem.startswith(('m_options_', 'm_pda_', 'm_share_', 'm_team_', 'x2m_'))
            changes = {}
            for index, obj in enumerate(writer.objects):
                if not hasattr(obj, 'raw_fields'):
                    continue
                kind = writer.meta_objects[obj.type_index].name
                if kind == b'igImage':
                    width, height, fmt = (field(writer, index, n) for n in (2, 3, 11))
                    ref = field(writer, index, 13)
                    raw = bytes(writer.objects[ref].data)
                    if width % 4 or height % 4 or width <= 0 or height <= 0:
                        raise ValueError('Image does not contain complete Wii tiles')
                    if fmt == 7:
                        if len(raw) != width * height * 4:
                            raise ValueError('Unexpected RGBA image size')
                        rgba = bytearray(raw)
                    elif fmt == 15:
                        if len(raw) != width * height:
                            raise ValueError('Unexpected DXT3 image size')
                        rgba = bytearray(width * height * 4)
                        for by in range(height // 4):
                            for bx in range(width // 4):
                                pixels = decoder.decode_dxt3_block(raw, (by * (width // 4) + bx) * 16)
                                for n, pixel in enumerate(pixels):
                                    offset = ((by * 4 + n // 4) * width + bx * 4 + n % 4) * 4
                                    rgba[offset:offset + 4] = bytes(pixel)
                    else:
                        raise ValueError(f'Unsupported image format {fmt}')
                    if tint:
                        for offset in range(0, len(rgba), 4):
                            rgba[offset:offset + 3] = bytes(red(rgba[offset:offset + 3]))
                    encoded = encoder.encode_rgb5a3(rgba, width, height)
                    if ref in changes and changes[ref] != encoded:
                        raise ValueError('Conflicting shared image data')
                    changes[ref] = encoded
                    # Same native image layout used by the tested Wii skin exporter.
                    for slot, value in ((4,4),(5,0),(6,103),(7,5),(8,5),(9,5),(10,1),
                                        (11,21),(12,len(encoded)),(14,-1),(15,1),(16,0),
                                        (17,-1),(18,0),(19,width*2),(20,1),(21,0)):
                        put(writer,index,slot,value)
                    receipt['images'] += 1
                elif tint and kind == b'igVertexArray1_1':
                    streams = bytes(writer.objects[field(writer,index,2)].data)
                    ref, = struct.unpack_from('<I',streams,8)
                    if ref == 0xffffffff or ref in changes:
                        continue
                    rgba = bytearray(writer.objects[ref].data)
                    if len(rgba) != field(writer,index,3)*4:
                        raise ValueError('Unexpected vertex color stream size')
                    for offset in range(0,len(rgba),4):
                        before=bytes(rgba[offset:offset+3])
                        rgba[offset:offset+3]=bytes(red(before))
                        receipt['vertex_tints'] += before != rgba[offset:offset+3]
                    changes[ref]=bytes(rgba)
            for index,data in changes.items():
                memory(writer,index,data)
            writer, receipt['native_meshes'] = convert_geometry(
                writer, native_models, graph, IGBReader, from_reader)
            writer.write(str(destination))
            check=IGBReader(str(destination));check.read()
            restored=from_reader(check)
            for entry in restored.entries:
                for definition, value in zip(restored.meta_objects[entry.type_index].fields, entry.field_values):
                    if definition.short_name == b'String' and not 0 <= value < len(restored.name_pool):
                        raise ValueError('Serialized directory name is outside its name pool')
            if len(restored.objects) != len(writer.objects):
                raise ValueError('Graph object count changed on serialization')
            for index,obj in enumerate(writer.objects):
                other=restored.objects[index]
                if hasattr(obj,'raw_fields') and ([(k,v) for k,v,_ in obj.raw_fields] !=
                                                  [(k,v) for k,v,_ in other.raw_fields]):
                    raise ValueError(f'Graph field mismatch after serialization: {relative} object {index}')
                if hasattr(obj,'data') and bytes(obj.data) != bytes(other.data):
                    raise ValueError(f'Graph memory mismatch after serialization: {relative} object {index}')
        receipt['output_sha256']=hashlib.sha256(destination.read_bytes()).hexdigest()
        manifest.append(receipt)
    (output/'conversion-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'Authored {len(manifest)} private candidate files; native rendering remains unverified')


if __name__ == '__main__':
    main()
