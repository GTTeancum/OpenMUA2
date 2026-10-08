"""Read-only tests against private stock font data and supplied Xbox artwork."""
import argparse
import struct
import unittest
import zipfile

from PIL import Image
import build_xbox_menu_fonts as target


class SharedFontTests(unittest.TestCase):
    def test_shared_metrics_preserve_other_characters_and_uvs(self):
        with zipfile.ZipFile(self.source_wad) as archive:
            for suffix in target.HASHES:
                with self.subTest(font=suffix):
                    original = archive.read(f'ui/fonts/rev_med{suffix}.xmlb')
                    patched = target.patch_font_table(original, suffix)
                    self.assertGreaterEqual(len(patched), len(original))
                    before, after = target.read_font(original), target.read_font(patched)
                    for number, glyph in before['glyphs'].items():
                        expected = dict(glyph)
                        if number in target.EXTRA_CHARACTERS:
                            expected = target.font_layout(before)[number]
                        if number in target.CROPS:
                            for field in ('width', 'height', 'horizadvance', 'baseline'):
                                expected[field] = before['glyphs'][164][field]
                        self.assertEqual(after['glyphs'][number], expected)
                    for invalid in (b'', original[:-1], original + b'bad'):
                        with self.assertRaises(ValueError):
                            target.patch_font_table(invalid, suffix)

    def test_trimmed_triggers_fill_cell_height_and_preserve_other_pixels(self):
        with zipfile.ZipFile(self.source_wad) as archive, Image.open(self.glyph_png) as art:
            for suffix in target.HASHES:
                with self.subTest(font=suffix):
                    original = archive.read(f'textures/fonts/rev_med{suffix}_eng.igb')
                    table = target.read_font(archive.read(f'ui/fonts/rev_med{suffix}.xmlb'))
                    patched, cells = target.patch_font(original, table, art, suffix)
                    start = len(original) - 256 * 256 * 2
                    allowed = set()
                    for cell in cells:
                        left, top, right, bottom = cell['rectangle']
                        visible_rows = set()
                        for y in range(top, bottom):
                            for x in range(left, right):
                                offset = start + ((y//4*64+x//4)*16+y%4*4+x%4)*2
                                allowed.update((offset, offset+1))
                                pixel = struct.unpack_from('>H', patched, offset)[0]
                                if pixel & 0x8000 or pixel & 0x7000:
                                    visible_rows.add(y)
                        if cell['character'] in (225, 226):
                            self.assertGreaterEqual(len(visible_rows), bottom-top-1)
                    self.assertEqual(len(original), len(patched))
                    self.assertTrue(all(a == b or i in allowed
                                        for i, (a, b) in enumerate(zip(original, patched))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-wad', required=True)
    parser.add_argument('--glyph-png', required=True)
    args = parser.parse_args()
    SharedFontTests.source_wad = args.source_wad
    SharedFontTests.glyph_png = args.glyph_png
    unittest.main(argv=[__file__], verbosity=2)
