"""Private-data regression: python tools/test_xbox_tutorials.py --source-wad PATH.

Reads the user's original archive; never writes game data or distributes it.
"""
import argparse
from pathlib import Path
import struct
import unittest
from unittest.mock import patch
import zipfile

import xbox_tutorials as target


class TutorialTests(unittest.TestCase):
    source_wad: Path

    @classmethod
    def setUpClass(cls):
        with zipfile.ZipFile(cls.source_wad) as archive:
            cls.tables = {name: archive.read('data/vv_tips.' + name)
                          for name in ('engb', 'itab', 'xmlb')}
            cls.package = archive.read('packages/generated/maps/package/permanent.fb')

    def test_all_twelve_and_other_platforms_preserved(self):
        for extension, original in self.tables.items():
            with self.subTest(extension=extension):
                result = target.patch_tutorials(original)
                self.assertEqual(len(original), len(result))
                old, new = target.read_nodes(original), target.read_nodes(result)
                self.assertEqual(len(old), len(new))
                changed = set()
                for (_, before, _, _), (_, after, _, _) in zip(old, new):
                    ident = before.get('id')
                    if before.get('platform') == 'rev' and ident in target.REPLACEMENTS:
                        changed.add(ident)
                        self.assertEqual(after['text'], target.REPLACEMENTS[ident][1])
                        before = dict(before, text=after['text'])
                    self.assertEqual(before, after)
                self.assertEqual(changed, set(target.REPLACEMENTS))

    def test_shared_fusion_string_is_separated(self):
        for original in self.tables.values():
            result = target.patch_tutorials(original)
            def fusion(data):
                return {a['platform']: (a['text'], loc['text'])
                        for _, a, loc, _ in target.read_nodes(data) if a.get('id') == '12'}
            before, after = fusion(original), fusion(result)
            self.assertEqual(before['rev'][1], before['psp'][1])
            self.assertEqual(before['psp'], after['psp'])
            self.assertNotEqual(after['rev'][1], after['psp'][1])

    def test_unknown_modified_and_truncated_tables_rejected(self):
        for original in self.tables.values():
            modified = bytearray(original)
            modified[-2] ^= 1
            for invalid in (b'', original[:-1], bytes(modified), target.patch_tutorials(original)):
                with self.assertRaisesRegex(ValueError, 'Unknown/already-modified'):
                    target.patch_tutorials(invalid)

    def test_overflow_fails_without_modifying_input(self):
        original = self.tables['engb']
        replacements = dict(target.REPLACEMENTS)
        replacements['3'] = ('Grab', 'X' * len(original))
        with patch.object(target, 'REPLACEMENTS', replacements):
            with self.assertRaisesRegex(ValueError, 'exceeds tutorial allocation'):
                target.patch_tutorials(original)
        self.assertEqual(original, self.tables['engb'])

    def test_embedded_copies_and_package_preservation(self):
        from build_qte_glyph_override import patch_tip_package
        result = patch_tip_package(self.package, target.patch_tutorials)
        self.assertEqual(len(result), len(self.package))
        permitted = set()
        for extension in ('engb', 'itab'):
            entry = self.package.index(('data/vv_tips.' + extension).encode() + b'\0')
            size = struct.unpack_from('<I', self.package, entry + 192)[0]
            start = entry + 196
            self.assertEqual(self.package[start:start + size], self.tables[extension])
            self.assertEqual(result[start:start + size], target.patch_tutorials(self.tables[extension]))
            permitted.update(range(start, start + size))
        self.assertTrue(all(a == b or i in permitted
                            for i, (a, b) in enumerate(zip(self.package, result))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-wad', type=Path, required=True)
    args, rest = parser.parse_known_args()
    TutorialTests.source_wad = args.source_wad
    unittest.main(argv=[__file__, *rest])
