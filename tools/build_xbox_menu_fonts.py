#!/usr/bin/env python3
"""Build PRIVATE Xbox menu font candidates; does not install or alter game data.

Requires the matching opt-in Xbox prompt resolver. Keep generated IGBs and the
supplied artwork outside Git. The exact stock English fonts are hash guarded.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zipfile
from PIL import Image
from build_qte_glyph_override import rgb5a3
from inspect_button_glyphs import read_font

HASHES = {
    '': '27a007d6850ef0417531ec1a5b95822fd1b23867671730db1f7bd7de69c2f64a',
    '_ws': '4e35bbc0dbfac3be8d1de16c78dbeae653eb35795b7b770953fca9a23f2bdb0d',
}
TABLE_HASHES = {
    '': 'ce185b67c873a76f610641009491ab5b984ac2820090316c0931eacb1e448b22',
    '_ws': 'c47e707183ad3953e5a45e6342dad16507d89a321fb57ffae0ba5b43a9b968ee',
}
# Native text character = 162 + icon ID. ID 6 becomes contextual Xbox X.
CROPS = {
    163: ('Start', (92, 572, 120, 597)),
    164: ('A', (360, 800, 420, 860)),
    165: ('B', (240, 800, 300, 860)),
    166: ('Y', (420, 800, 480, 860)),
    167: ('LB', (31, 572, 60, 597)),
    168: ('X', (300, 800, 360, 860)),
}


def patch_font(original, font_table, source, suffix):
    if suffix not in HASHES or hashlib.sha256(original).hexdigest() != HASHES[suffix]:
        raise ValueError('Unknown/already-modified menu font')
    if source.size != (1024, 1024):
        raise ValueError('Expected supplied 1024x1024 Fullkeyset1 artwork')
    source = source.convert('RGBA')
    result = bytearray(original)
    pixel_start = len(original) - 256 * 256 * 2
    touched = set()
    cells = []
    for character, (label, crop) in CROPS.items():
        g = font_table['glyphs'][character]
        left, right = (round(float(g[k]) * 256) for k in ('s', 's2'))
        top, bottom = (round((1 - float(g[k])) * 256) for k in ('t2', 't'))
        if not (0 <= left < right <= 256 and 0 <= top < bottom <= 256):
            raise ValueError('Invalid font cell bounds')
        width, height = right - left, bottom - top
        glyph = source.crop(crop)
        if glyph.getbbox() is None:
            raise ValueError('Supplied glyph crop is empty')
        glyph.thumbnail((width, height), Image.Resampling.LANCZOS)
        cell = Image.new('RGBA', (width, height))
        cell.paste(glyph, ((width-glyph.width)//2, (height-glyph.height)//2))
        # Native font UVs use bottom-origin coordinates.
        cell = cell.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        for y in range(height):
            for x in range(width):
                ax, ay = left+x, top+y
                p = pixel_start + ((ay//4*64+ax//4)*16+ay%4*4+ax%4)*2
                if p in touched:
                    raise ValueError('Overlapping font glyph cells')
                struct.pack_into('>H', result, p, rgb5a3(cell.getpixel((x, y))))
                touched.update((p, p+1))
        cells.append(dict(character=character, icon_id=character-162, label=label,
                          rectangle=[left, top, right, bottom]))
    if any(a != b and i not in touched for i, (a, b) in enumerate(zip(original, result))):
        raise ValueError('Write escaped selected glyph cells')
    return bytes(result), cells


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-wad', type=Path, required=True)
    parser.add_argument('--glyph-png', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    if args.output_dir.exists():
        parser.error('Output directory must not already exist')
    prepared = []
    with Image.open(args.glyph_png) as source, zipfile.ZipFile(args.source_wad) as archive:
        for suffix in HASHES:
            name = f'textures/fonts/rev_med{suffix}_eng.igb'
            original = archive.read(name)
            table_bytes = archive.read(f'ui/fonts/rev_med{suffix}.xmlb')
            if hashlib.sha256(table_bytes).hexdigest() != TABLE_HASHES[suffix]:
                raise ValueError('Unknown font coordinate table')
            table = read_font(table_bytes)
            result, cells = patch_font(original, table, source, suffix)
            prepared.append((name, result, cells))
    args.output_dir.mkdir(parents=True)
    receipt = {'scope': 'private candidates; runtime mapping required; not installed', 'textures': []}
    for name, data, cells in prepared:
        (args.output_dir / Path(name).name).write_bytes(data)
        receipt['textures'].append(dict(member=name, sha256=hashlib.sha256(data).hexdigest(), cells=cells))
    (args.output_dir / 'receipt.json').write_text(json.dumps(receipt, indent=2)+'\n')
    print(json.dumps(receipt, indent=2))


if __name__ == '__main__':
    main()
