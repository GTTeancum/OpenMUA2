#!/usr/bin/env python3
"""Compile supplied Xbox X artwork into a PRIVATE candidate QTE HUD archive.

This changes seven motion-prompt cells and the safeguard instruction. Xbox UI
mode also converts menu fonts, thirteen tutorials and ten Options descriptions. It
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
RAPID_X_CROPS = ((300, 910, 360, 960), (300, 860, 360, 910))


def gx_offset(x, y):
    return ((y // 4 * 128 + x // 4) * 16 + y % 4 * 4 + x % 4) * 2


def rgb5a3(rgba):
    r, g, b, a = rgba
    if a == 255:
        return 0x8000 | ((r * 31 + 127) // 255 << 10) | \
               ((g * 31 + 127) // 255 << 5) | ((b * 31 + 127) // 255)
    return ((a * 7 + 127) // 255 << 12) | ((r * 15 + 127) // 255 << 8) | \
           ((g * 15 + 127) // 255 << 4) | ((b * 15 + 127) // 255)


# Shared native HUD button sprites, including their alternate frames. Coordinates
# are bottom-origin in hud_coords_rev.xmlb; texture storage has the opposite Y.
# Preserve action semantics: native Z is block/LB, not grab/X.
HUD_BUTTONS = {
    'A': ((360, 800, 420, 860), ((0, 0, 60, 60), (0, 110, 60, 160), (0, 60, 60, 110))),
    'B': ((240, 800, 300, 860), ((120, 110, 180, 160), (120, 60, 180, 110))),
    'LB': ((31, 572, 60, 597), ((120, 0, 180, 60), (60, 110, 120, 160), (60, 60, 120, 110))),
    'Y': ((420, 800, 480, 860), ((180, 0, 240, 60), (180, 110, 240, 160), (180, 60, 240, 110))),
}


def patch_texture(original, source, rapid_tap=False, xbox_hud_buttons=False):
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
        cell = glyph
        if rapid_tap and (ox, oy) in ((0, 200), (60, 200)):
            # Native sprites 95/96 form the shared rapid-tap animation pair.
            phase = int(ox == 60)
            pose = source.convert('RGBA').crop(RAPID_X_CROPS[phase])
            bounds = pose.getchannel('A').getbbox()
            if bounds is None:
                raise ValueError('Tilted X pose is empty')
            pose = pose.crop(bounds)
            pose.thumbnail((56, 46), Image.Resampling.LANCZOS)
            cell = Image.new('RGBA', (60, 60))
            cell.paste(pose, ((60-pose.width)//2, 2 if phase == 0 else 12))
            # HUD coordinates address this GX atlas from the bottom edge.
            cell = cell.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        for y in range(60):
            for x in range(60):
                p = pixel_start + gx_offset(ox + x, oy + y)
                struct.pack_into('>H', result, p, rgb5a3(cell.getpixel((x, y))))
                touched.update((p, p + 1))
    if xbox_hud_buttons:
        for crop, rectangles in HUD_BUTTONS.values():
            button = source.convert('RGBA').crop(crop)
            bounds = button.getchannel('A').getbbox()
            if bounds is None:
                raise ValueError('Xbox HUD button crop is empty')
            button = button.crop(bounds)
            for left, top, right, bottom in rectangles:
                width, height = right-left, bottom-top
                scale = min((width-4)/button.width, (height-4)/button.height)
                pose = button.resize((max(1, round(button.width*scale)),
                                      max(1, round(button.height*scale))), Image.Resampling.LANCZOS)
                cell = Image.new('RGBA', (width, height))
                cell.paste(pose, ((width-pose.width)//2, (height-pose.height)//2))
                cell = cell.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
                for y in range(height):
                    for x in range(width):
                        p = pixel_start + gx_offset(left+x, 512-bottom+y)
                        if p in touched or p+1 in touched:
                            raise ValueError('Xbox button overlaps a motion prompt')
                        struct.pack_into('>H', result, p, rgb5a3(cell.getpixel((x, y))))
                        touched.update((p, p+1))
    # Verify preservation byte-for-byte, including the complete IGB header.
    if any(a != b and i not in touched for i, (a, b) in enumerate(zip(original, result))):
        raise ValueError('A write escaped the selected glyph cells')
    return bytes(result)


def patch_package(data, source, rapid_tap=False, xbox_hud_buttons=False):
    if data.count(TEXTURE) != 1:
        raise ValueError('HUD texture entry is missing or ambiguous')
    entry = data.index(TEXTURE)
    size = struct.unpack_from('<I', data, entry + 192)[0]
    start = entry + 196
    if size != 529328 or start + size > len(data):
        raise ValueError('Unexpected HUD package entry')
    replacement = patch_texture(data[start:start + size], source, rapid_tap, xbox_hud_buttons)
    return data[:start] + replacement + data[start + size:]



def patch_safeguard_tip(data):
    """Replace the platform-specific instruction without shifting XMLB offsets."""
    def word(p):
        if p < 0 or p + 4 > len(data):
            raise ValueError('Truncated tip table')
        return struct.unpack_from('<I', data, p)[0]
    if word(0) != 0x11B1 or word(4) != 1:
        raise ValueError('Unexpected tip XMLB format')
    string_start = word(8)
    def string(p):
        if not string_start <= p < len(data):
            raise ValueError('Invalid tip string offset')
        end = data.find(b'\0', p)
        if end < 0:
            raise ValueError('Unterminated tip string')
        return data[p:end].decode('cp1252')
    seen, matches = set(), []
    def visit(p):
        if p in seen or p < 8 or p % 4 or len(seen) >= 4096:
            raise ValueError('Invalid tip node graph')
        seen.add(p)
        count = word(p + 12)
        if count > 64 or p + 16 + count * 8 > string_start:
            raise ValueError('Invalid tip attributes')
        attrs = {}
        offsets = {}
        for i in range(count):
            key = string(word(p + 16 + i * 8))
            value = word(p + 20 + i * 8)
            if key in attrs:
                raise ValueError('Duplicate tip attribute')
            attrs[key], offsets[key] = string(value), value
        if attrs.get('id') == '54' and attrs.get('platform') == 'rev':
            if attrs.get('title') != 'Safeguard' or 'Wii Remote' not in attrs.get('text', ''):
                raise ValueError('Unknown/already-modified safeguard tip')
            matches.append((offsets['text'], len(attrs['text'].encode('cp1252'))))
        child = word(p + 8)
        while child != 0xffffffff:
            visit(child)
            child = word(child + 4)
    visit(8)
    if len(matches) != 1:
        raise ValueError('Missing or ambiguous safeguard tip')
    offset, size = matches[0]
    replacement = b'UT: \\nRepeatedly press X to complete the safeguard interaction.\\n'
    if len(replacement) > size:
        raise ValueError('Replacement exceeds original string allocation')
    result = bytearray(data)
    result[offset:offset + size] = replacement.ljust(size, b'\0')
    return bytes(result)


def patch_tip_package(data, patcher=patch_safeguard_tip):
    for name in (b'data/vv_tips.engb\0', b'data/vv_tips.itab\0'):
        if data.count(name) != 1:
            raise ValueError('Missing or ambiguous packaged tips')
        entry = data.index(name)
        size = struct.unpack_from('<I', data, entry + 192)[0]
        start = entry + 196
        if start + size > len(data):
            raise ValueError('Truncated packaged tips')
        data = data[:start] + patcher(data[start:start + size]) + data[start + size:]
    return data

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source-wad', type=Path, required=True)
    ap.add_argument('--glyph-png', type=Path, required=True)
    ap.add_argument('--output-wad', type=Path, required=True)
    ap.add_argument('--xbox-ui', action='store_true',
                    help='Also compile Xbox fonts, 13 tutorials and 10 Options descriptions; requires matching runtime')
    ap.add_argument('--rapid-tap', action='store_true',
                    help='Compile tilted X animation poses; requires the paired rapid-tap runtime')
    ap.add_argument('--xbox-hud-buttons', action='store_true',
                    help='Replace shared A/B/Y/block HUD sprites with Xbox artwork')
    args = ap.parse_args()
    if args.xbox_ui and not (args.xbox_hud_buttons and args.rapid_tap):
        ap.error("The current Xbox menu layout requires --rapid-tap and --xbox-hud-buttons")
    if args.xbox_hud_buttons and not (args.xbox_ui and args.rapid_tap):
        ap.error('--xbox-hud-buttons requires --xbox-ui and --rapid-tap for its paired runtime')
    output = args.output_wad.resolve()
    ui_manifest = output.with_suffix('.xbox-ui.manifest')
    if args.xbox_ui and ui_manifest.exists():
        raise ValueError('Xbox UI manifest output already exists')
    if output in (args.source_wad.resolve(), args.glyph_png.resolve()) or output.exists():
        raise ValueError('Output must be a new path separate from both inputs')
    output.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(args.glyph_png) as image:
        image.load()
        with zipfile.ZipFile(args.source_wad) as source:
            infos = source.infolist()
            if len({i.filename for i in infos}) != len(infos):
                raise ValueError('Duplicate WAD entries')
            replacements = {PACKAGE: patch_package(source.read(PACKAGE), image, args.rapid_tap, args.xbox_hud_buttons)}
            tip_patcher = patch_safeguard_tip
            if args.xbox_ui:
                from xbox_tutorials import patch_tutorials
                from xbox_options_help import patch_options_help
                from xbox_options_layout import PACKAGE as OPTIONS_PACKAGE, patch_options_package
                from build_xbox_menu_fonts import HASHES, TABLE_HASHES, patch_font, patch_font_table, read_font
                tip_patcher = patch_tutorials
                replacements[OPTIONS_PACKAGE] = patch_options_package(source.read(OPTIONS_PACKAGE))
                for extension in ('engb', 'itab', 'xmlb'):
                    name = 'data/strings.' + extension
                    replacements[name] = patch_options_help(source.read(name))
                for suffix in HASHES:
                    name = f'textures/fonts/rev_med{suffix}_eng.igb'
                    table = source.read(f'ui/fonts/rev_med{suffix}.xmlb')
                    if hashlib.sha256(table).hexdigest() != TABLE_HASHES[suffix]:
                        raise ValueError('Unknown font coordinate table')
                    replacements[name], _ = patch_font(source.read(name), read_font(table), image, suffix)
                    replacements[f'ui/fonts/rev_med{suffix}.xmlb'] = patch_font_table(table, suffix)
            for name in ('data/vv_tips.engb', 'data/vv_tips.itab', 'data/vv_tips.xmlb'):
                replacements[name] = tip_patcher(source.read(name))
            tips_package = 'packages/generated/maps/package/permanent.fb'
            replacements[tips_package] = patch_tip_package(source.read(tips_package), tip_patcher)
            handle, temporary = tempfile.mkstemp(prefix='qte-glyph-', suffix='.wad.tmp', dir=output.parent)
            os.close(handle)
            try:
                with zipfile.ZipFile(temporary, 'w') as target:
                    target.comment = source.comment
                    for info in infos:
                        payload = replacements[info.filename] if info.filename in replacements else source.read(info)
                        target.writestr(copy.copy(info), payload)
                with zipfile.ZipFile(temporary) as check:
                    if check.testzip() is not None or check.namelist() != source.namelist():
                        raise ValueError('Candidate archive validation failed')
                    for name, payload in replacements.items():
                        if check.read(name) != payload:
                            raise ValueError('Replacement did not round-trip: ' + name)
                    for info in infos:
                        if info.filename in replacements:
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
    if args.xbox_ui:
        with ui_manifest.open('x', encoding='ascii', newline='\n') as manifest:
            version = 7
            manifest.write(f'OpenMUA2-Xbox-UI-v{version}\n')
            for name, payload in sorted(replacements.items()):
                manifest.write(hashlib.sha256(payload).hexdigest() + '\t' + name + '\n')
    print(json.dumps({'status': 'candidate only; not installed', 'output': str(output),
                      'replaced_motion_cells': len(MOTION_CELLS),
                      'xbox_hud_button_cells': 11 if args.xbox_hud_buttons else 0,
                      'xbox_menu_fonts': 2 if args.xbox_ui else 0,
                      'tutorial_instructions': 13 if args.xbox_ui else 1,
                      'options_control_descriptions': 10 if args.xbox_ui else 0,
                      'unchanged_archive_members': len(infos) - len(replacements),
                      'scope': 'Common Xbox UI candidate; matching runtime required; other Wii prompts remain'
                               if args.xbox_ui else 'QTE motion artwork and safeguard instruction; other Wii prompts remain'}, indent=2))


if __name__ == '__main__':
    main()
