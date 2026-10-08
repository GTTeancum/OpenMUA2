"""Validate rapid-tap artwork against private stock data; no game data is stored."""
import argparse
import struct
import unittest
import zipfile
from PIL import Image
import build_qte_glyph_override as target

class RapidTapTests(unittest.TestCase):
    def test_two_tilted_poses_preserve_other_cells(self):
        with zipfile.ZipFile(self.source_wad) as archive:
            package=archive.read(target.PACKAGE)
        entry=package.index(target.TEXTURE)
        size=struct.unpack_from('<I',package,entry+192)[0]
        original=package[entry+196:entry+196+size]
        with Image.open(self.glyph_png) as art:
            baseline=target.patch_texture(original,art)
            animated=target.patch_texture(original,art,True)
        self.assertEqual(len(baseline),len(animated))
        start=len(original)-512*512*2
        allowed=set(); bounds=[]; poses=[]
        for ox in (0,60):
            visible=[]; pixels=[]
            for y in range(60):
                for x in range(60):
                    offset=start+target.gx_offset(ox+x,200+y)
                    allowed.update((offset,offset+1))
                    pixel=struct.unpack_from('>H',animated,offset)[0]
                    pixels.append(pixel)
                    if pixel&0x8000 or pixel&0x7000:visible.append((x,59-y))
            self.assertTrue(visible)
            bounds.append((min(y for x,y in visible),max(y for x,y in visible)))
            poses.append(pixels)
        self.assertNotEqual(poses[0],poses[1])
        self.assertGreaterEqual(bounds[1][0]-bounds[0][0],8)
        self.assertTrue(all(0<=top<bottom<60 for top,bottom in bounds))
        self.assertFalse(any(a!=b and i not in allowed for i,(a,b) in enumerate(zip(baseline,animated))))
        with Image.open(self.glyph_png) as art:
            with self.assertRaises(ValueError):target.patch_texture(animated,art,True)

    def test_shared_hud_buttons_preserve_motion_and_other_pixels(self):
        with zipfile.ZipFile(self.source_wad) as archive:
            package = archive.read(target.PACKAGE)
            from xbox_tutorials import read_nodes
            coords = [a for n, a, _, _ in read_nodes(archive.read('data/hud_coords_rev.xmlb')) if n == 'coord']
        entry = package.index(target.TEXTURE)
        size = struct.unpack_from('<I', package, entry+192)[0]
        original = package[entry+196:entry+196+size]
        with Image.open(self.glyph_png) as art:
            baseline = target.patch_texture(original, art, True)
            buttons = target.patch_texture(original, art, True, True)
        # Verify against the actual game's coordinate table, not the compiler's
        # rectangle constants. Text-font icon IDs must not enter this path.
        sprite_groups = {'A': (38,42,43), 'B': (44,45), 'LB': (40,46,47), 'Y': (41,48,49)}
        start = len(original)-512*512*2
        allowed = set()
        for label, sprites in sprite_groups.items():
            for sprite in sprites:
                c = {k:int(v) for k,v in coords[sprite].items()}
                pixels = []
                for y in range(512-c['bottom'],512-c['top']):
                    for x in range(c['left'],c['right']):
                        offset = start+target.gx_offset(x,y)
                        allowed.update((offset,offset+1))
                        pixels.append(struct.unpack_from('>H',buttons,offset)[0])
                self.assertTrue(any(v & 0xf000 for v in pixels), (label,sprite))
                # Cell is fully replaced, with transparent padding around glyph.
                self.assertEqual(pixels[0],0)
                width = c['right']-c['left']
                visible_x = [i % width for i,v in enumerate(pixels) if v & 0xf000]
                self.assertGreaterEqual(max(visible_x)-min(visible_x)+1,
                                        (width if label == 'LB' else min(width,c['bottom']-c['top']))-8)
        self.assertTrue(any(a!=b for a,b in zip(baseline,buttons)))
        self.assertFalse(any(a!=b and i not in allowed for i,(a,b) in enumerate(zip(baseline,buttons))))
        self.assertEqual(len(original),len(buttons))

if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--source-wad',required=True)
    parser.add_argument('--glyph-png',required=True)
    args,remaining=parser.parse_known_args()
    RapidTapTests.source_wad=args.source_wad
    RapidTapTests.glyph_png=args.glyph_png
    unittest.main(argv=['test_rapid_tap_glyphs']+remaining)
