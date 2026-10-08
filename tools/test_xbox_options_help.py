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
                self.assertGreaterEqual(len(result), len(original))
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
                with self.assertRaisesRegex(ValueError, 'exceeds UI text limit'):
                    target.patch_options_help(original)

    def test_complete_control_reference(self):
        from xbox_options_layout import PACKAGES
        for package, (_, menu) in PACKAGES.items():
            with self.subTest(package=package):
                self.check_control_reference(package, menu)

    def check_control_reference(self, package, menu):
        from xbox_options_layout import patch_options_package
        with zipfile.ZipFile(self.source_wad) as archive:
            original = archive.read(package)
        result = patch_options_package(original, package)
        import struct
        def members(data):
            out = {}; offset = 0
            while offset < len(data):
                name = data[offset:offset+192].split(b'\0')[0].decode('ascii')
                size = struct.unpack_from('<I', data, offset+192)[0]
                out[name] = data[offset+196:offset+196+size]
                offset += 196+size
            self.assertEqual(offset, len(data))
            return out
        before, after = members(original), members(result)
        self.assertEqual(before.keys(), after.keys())
        changed = {n for n in before if before[n] != after[n]}
        self.assertEqual(changed, {f'ui/menus/{menu}.engb', f'ui/menus/{menu}.itab',
                                   'ui/menus/cw_pausemenu.igb'})
        for name in sorted(changed - {'ui/menus/cw_pausemenu.igb'}):
            nodes = {a.get('name'): a for _, a, *_ in target.read_nodes(after[name])}
            old_nodes = target.read_nodes(before[name])
            new_nodes = target.read_nodes(after[name])
            self.assertEqual(len(old_nodes), len(new_nodes))
            for old, new in zip(old_nodes, new_nodes):
                widget = old[1].get('name', '')
                if widget not in ('controls', 'controls_list', 'controls_list2') and not widget.startswith(('label_button', 'label_stick', 'label_click')):
                    self.assertEqual(old, new, widget)
            self.assertEqual(nodes['controls']['model'], 'ui/models/m_invis')
            self.assertIn('Powers', nodes['label_button02']['text'])
            self.assertIn('Fusion', nodes['label_button03']['text'])
            self.assertFalse(nodes['label_click01']['text'])
            for i in ([1] + list(range(4,13))):
                self.assertFalse(nodes['label_button%02d' % i].get('text'))
        with self.assertRaises(ValueError):
            patch_options_package(result, package)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-wad', type=Path, required=True)
    args, rest = parser.parse_known_args()
    OptionsHelpTests.source_wad = args.source_wad
    unittest.main(argv=[__file__, *rest])
