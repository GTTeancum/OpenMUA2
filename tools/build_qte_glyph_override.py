#!/usr/bin/env python3
"""Compile supplied Xbox X artwork into a PRIVATE candidate QTE HUD archive.

This changes only seven motion-prompt cells in the verified Wii HUD atlas. It
neither installs the archive nor claims complete Xbox UI conversion. Input game
archives and supplied images are never modified. Keep all outputs outside Git.
Requires the existing Pillow installation; nothing is downloaded.
"""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import struct
import tempfile
import zipfile
from PIL import Image

PACKAGE = 'packages/generated/maps/package/permanent_rev.fb'
TEXTURE = b'textures/ui/hud_rev.igb\0'
ORIGINAL_SHA256 = '2e4119a6177d39913fc4627c2fb37eff1b2e0f7c7f0edc89abd928ece92c1fde'
# Atlas cell origins; the 60-pixel cells exclude adjacent HUD graphics.
MOTION_CELLS = ((180, 20), (60, 80), (0, 140), (0, 200),
                (60, 200), (120, 200), (180, 200))
X_CROP = (300, 800, 360, 860)  # Large circular X in the supplied Fullkeyset1.png.


def gx_offset(x, y):
    return ((y // 4 * 128 + x // 4) * 16 + y % 4 * 4 + x % 4) * 2


def rgb5a3(rgba):
    r, g, b, a = rgba
    if a == 255:
        return 0x8000 | ((r * 31 + 127) // 255 << 10) | \
               ((g * 31 + 127) // 255 << 5) | ((b * 31 + 127) // 255)
    return ((a * 7 + 127) // 255 << 12) | ((r * 15 + 127) // 255 << 8) | \
           ((g * 15 + 127) // 255 << 4) | ((b * 15 + 127) // 255)


def patch_texture(original, source):
    if hashlib.sha256(original).hexdigest() != ORIGINAL_SHA256:
        raise ValueError('Unknown/already-modified HUD texture; refusing an unverified layout')
    if source.size != (1024, 1024):
        raise ValueError('Expected the supplied 1024x1024 Fullkeyset1 atlas')
    glyph = source.convert('RGBA').crop(X_CROP)
    if glyph.getbbox() is None:
        raise ValueError('X crop is empty')
    result = bytearray(original)
    pixel_start = len(original) - 512 * 512 * 2
    touched = set()
    for ox, oy in MOTION_CELLS:
        for y in range(60):
            for x in range(60):
                p = pixel_start + gx_offset(ox + x, oy + y)
                struct.pack_into('>H', result, p, rgb5a3(glyph.getpixel((x, y))))
                touched.update((p, p + 1))
    # Verify preservation byte-for-byte, including the complete IGB header.
    if any(a != b and i not in touched for i, (a, b) in enumerate(zip(original, result))):
        raise ValueError('A write escaped the selected glyph cells')
    return bytes(result)


def patch_package(data, source):
    if data.count(TEXTURE) != 1:
        raise ValueError('HUD texture entry is missing or ambiguous')
    entry = data.index(TEXTURE)
    size = struct.unpack_from('<I', data, entry + 192)[0]
    start = entry + 196
    if size != 529328 or start + size > len(data):
        raise ValueError('Unexpected HUD package entry')
    replacement = patch_texture(data[start:start + size], source)
    return data[:start] + replacement + data[start + size:]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source-wad', type=Path, required=True)
    ap.add_argument('--glyph-png', type=Path, required=True)
    ap.add_argument('--output-wad', type=Path, required=True)
    args = ap.parse_args()
    output = args.output_wad.resolve()
    if output in (args.source_wad.resolve(), args.glyph_png.resolve()) or output.exists():
        raise ValueError('Output must be a new path separate from both inputs')
    output.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(args.glyph_png) as image:
        image.load()
        with zipfile.ZipFile(args.source_wad) as source:
            infos = source.infolist()
            if len({i.filename for i in infos}) != len(infos):
                raise ValueError('Duplicate WAD entries')
            replacement = patch_package(source.read(PACKAGE), image)
            handle, temporary = tempfile.mkstemp(prefix='qte-glyph-', suffix='.wad.tmp', dir=output.parent)
            os.close(handle)
            try:
                with zipfile.ZipFile(temporary, 'w') as target:
                    target.comment = source.comment
                    for info in infos:
                        payload = replacement if info.filename == PACKAGE else source.read(info)
                        target.writestr(copy.copy(info), payload)
                with zipfile.ZipFile(temporary) as check:
                    if check.testzip() is not None or check.namelist() != source.namelist():
                        raise ValueError('Candidate archive validation failed')
                    if check.read(PACKAGE) != replacement:
                        raise ValueError('Replacement package did not round-trip')
                    for info in infos:
                        if info.filename == PACKAGE:
                            continue
                        other = check.getinfo(info.filename)
                        if (info.CRC, info.file_size) != (other.CRC, other.file_size):
                            raise ValueError('Unrelated archive member changed')
                # Never overwrite an output created concurrently.
                if output.exists():
                    raise ValueError('Output appeared while compiling')
                os.rename(temporary, output)
            finally:
                Path(temporary).unlink(missing_ok=True)
    print(json.dumps({'status': 'candidate only; not installed', 'output': str(output),
                      'replaced_motion_cells': len(MOTION_CELLS),
                      'unchanged_archive_members': len(infos) - 1,
                      'scope': 'QTE motion artwork only; other Wii prompts remain'}, indent=2))


if __name__ == '__main__':
    main()
