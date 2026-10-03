"""Read-only private-data regression for Options Xbox control descriptions."""
import argparse
from pathlib import Path
import unittest
from unittest.mock import patch
import zipfile

import xbox_options_help as target


class OptionsHelpTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with zipfile.ZipFile(cls.source_wad) as archive:
            cls.tables = {ext: archive.read('data/strings.' + ext)
                          for ext in ('engb', 'itab', 'xmlb')}

    def test_controls_and_unrelated_strings(self):
        for ext, original in self.tables.items():
            with self.subTest(extension=ext):
                result = target.patch_options_help(original)
                self.assertEqual(len(original), len(result))
                before, after = target.read_nodes(original), target.read_nodes(result)
                self.assertEqual(len(before), len(after))
                matched = set()
                for (name, old, loc, fields), (newname, new, newloc, newfields) in zip(before, after):
                    self.assertEqual(name, newname)
                    self.assertEqual(fields, newfields)
                    ident = old.get('id')
                    if ident in target.REPLACEMENTS:
                        self.assertEqual(new['text'], target.REPLACEMENTS[ident])
                        old = dict(old, text=new['text'])
                        matched.add(ident)
                    else:
                        self.assertEqual(loc, newloc)
                    self.assertEqual(old, new)
                self.assertEqual(matched, set(target.REPLACEMENTS))

    def test_unknown_and_modified_tables_rejected(self):
        for original in self.tables.values():
            changed = bytearray(original)
            changed[-1] ^= 1
            for invalid in (b'', original[:-1], bytes(changed), target.patch_options_help(original)):
                with self.assertRaisesRegex(ValueError, 'Unknown/already-modified'):
                    target.patch_options_help(invalid)

    def test_overflow_rejected(self):
        replacement = dict(target.REPLACEMENTS, **{'5517': 'X' * 1000})
        with patch.object(target, 'REPLACEMENTS', replacement):
            for original in self.tables.values():
                with self.assertRaisesRegex(ValueError, 'exceeds available allocation'):
                    target.patch_options_help(original)

    def test_long_chords_relocate_without_changing_other_nodes(self):
        original = self.tables['engb']
        result = target.patch_options_help(original)
        def location(data):
            return next(loc['text'] for _, attrs, loc, _ in target.read_nodes(data)
                        if attrs.get('id') == '5517')
        self.assertNotEqual(location(original), location(result))
        self.assertIn(b'$XRT + $XA/$XX/$XB/$XY Powers\0', result)
        self.assertIn(b'$XLT + $XA/$XB/$XX/$XY Fusion\0', result)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-wad', type=Path, required=True)
    args, rest = parser.parse_known_args()
    OptionsHelpTests.source_wad = args.source_wad
    unittest.main(argv=[__file__, *rest])
