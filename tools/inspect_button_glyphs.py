#!/usr/bin/env python3
"""Read-only font/glyph inventory for the user's local game and Xbox PNG atlases.

The JSON contains game-derived font coordinates: keep it outside Git. No archive
is extracted or rewritten. This is preparation for replacement, not installation.
"""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path


def read_font(data):
    def word(offset):
        if offset < 0 or offset + 4 > len(data):
            raise ValueError('Truncated font word')
        return struct.unpack_from('<I', data, offset)[0]

    if word(0) != 0x11B1 or word(4) != 1:
        raise ValueError('Unsupported font XMLB header')
    string_start = word(8)
    if not 24 <= string_start < len(data):
        raise ValueError('Invalid string table')

    def string(offset):
        if not string_start <= offset < len(data):
            raise ValueError('Invalid string reference')
        end = data.find(b'\0', offset)
        if end < 0:
            raise ValueError('Unterminated string')
        return data[offset:end].decode('ascii')

    seen = set()

    def node(offset):
        if offset in seen or len(seen) >= 4096 or offset < 8 or offset % 4:
            raise ValueError('Invalid/cyclic node graph')
        seen.add(offset)
        count = word(offset + 12)
        if count > 64 or offset + 16 + count * 8 > string_start:
            raise ValueError('Invalid attribute table')
        attributes = {}
        for index in range(count):
            p = offset + 16 + index * 8
            key, value = string(word(p)), string(word(p + 4))
            if key in attributes:
                raise ValueError('Duplicate attribute')
            attributes[key] = value
        return string(word(offset)), word(offset + 4), word(offset + 8), attributes

    name, sibling, child, metrics = node(8)
    if name != 'FONT_TABLE' or sibling != 0xFFFFFFFF:
        raise ValueError('Unexpected font root')
    glyphs = {}
    while child != 0xFFFFFFFF:
        name, sibling, descendant, attributes = node(child)
        if name != 'glyph' or descendant != 0xFFFFFFFF:
            raise ValueError('Unexpected font child')
        number = int(attributes['num'])
        if number in glyphs:
            raise ValueError('Duplicate glyph number')
        glyphs[number] = attributes
        child = sibling
    return {'metrics': metrics, 'glyphs': glyphs}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wad', type=Path, required=True)
    parser.add_argument('--glyph-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    report = {'mode': 'read-only; replacements not applied', 'fonts': {}, 'supplied_pngs': []}
    with zipfile.ZipFile(args.wad) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise ValueError('Duplicate archive entries')
        for name in names:
            if not name.startswith(('ui/fonts/rev_', 'ui/fonts/360_', 'ui/fonts/xbox_')):
                continue
            if not name.endswith('.xmlb'):
                continue
            info = archive.getinfo(name)
            if info.file_size > 1024 * 1024:
                raise ValueError('Unexpectedly large font metadata')
            data = archive.read(info)
            report['fonts'][name] = read_font(data)
            report['fonts'][name]['sha256'] = hashlib.sha256(data).hexdigest()
    for path in sorted(args.glyph_dir.glob('*.png')):
        data = path.read_bytes()
        if len(data) < 33 or data[:8] != b'\x89PNG\r\n\x1a\n' or data[12:16] != b'IHDR':
            raise ValueError(f'Invalid PNG header: {path.name}')
        width, height = struct.unpack_from('>II', data, 16)
        report['supplied_pngs'].append(dict(name=path.name, width=width, height=height,
                                           sha256=hashlib.sha256(data).hexdigest()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(f"Read {len(report['fonts'])} font tables and {len(report['supplied_pngs'])} Xbox atlases")


if __name__ == '__main__':
    main()
