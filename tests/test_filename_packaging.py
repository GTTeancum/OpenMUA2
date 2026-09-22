# SPDX-License-Identifier: GPL-3.0-or-later
"""LOCAL01b: portable tinygltf fixture and uncompromised manifest checks."""
from __future__ import annotations
from contextlib import redirect_stdout
import hashlib
import importlib.util
import io
import json
from pathlib import Path, PurePosixPath
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'project/lib/ModernGekko/vendor/dolphin/Externals/tinygltf/tinygltf/'
MODEL = PREFIX + 'models/CubeImageUriSpaces/'
NAME = '2x2 image  has multiple      spaces.png'
OLD = MODEL + ' ' + NAME
NEW = MODEL + NAME
EXPECTED_PNG_SHA256 = '00b9839320de4bbd342d092e5cedf05845ab4be733c0240d5dd5466a9565b217'
spec = importlib.util.spec_from_file_location('workspace_filename_tests', ROOT / 'tools/workspace.py')
w = importlib.util.module_from_spec(spec)
spec.loader.exec_module(w)


def fixture_manifest(root: Path, name: str, data: bytes) -> None:
    obj = {'schema': 1, 'files': [{'path': name, 'size': len(data),
                                 'sha256': hashlib.sha256(data).hexdigest()}]}
    (root / 'FILE-MANIFEST.json').write_text(json.dumps(obj), encoding='utf-8')


class FilenamePackagingTests(unittest.TestCase):
    def test_original_png_bytes_are_preserved(self):
        data = (ROOT / NEW).read_bytes()
        self.assertEqual(len(data), 79)
        self.assertEqual(hashlib.sha256(data).hexdigest(), EXPECTED_PNG_SHA256)
        self.assertEqual(data, (ROOT / MODEL / '2x2 image has spaces.png').read_bytes())

    def test_model_reference_resolves_to_portable_fixture(self):
        path = ROOT / MODEL / 'CubeImageUriMultipleSpaces.gltf'
        model = json.loads(path.read_bytes())
        self.assertEqual(model['images'][0]['uri'], NAME)
        self.assertTrue((path.parent / model['images'][0]['uri']).is_file())
        self.assertIn('  ', model['images'][0]['uri'])
        for item in model['buffers']:
            self.assertEqual((path.parent / item['uri']).stat().st_size, item['byteLength'])

    def test_manifest_maps_the_new_name_with_unchanged_digest(self):
        rows = {row['path']: row for row in w.manifest_rows(ROOT)}
        self.assertNotIn(OLD, rows)
        self.assertEqual(rows[NEW]['sha256'], EXPECTED_PNG_SHA256)
        self.assertEqual(rows[NEW]['size'], 79)
        for name in [NEW, MODEL + 'CubeImageUriMultipleSpaces.gltf', PREFIX + 'tests/tester.cc']:
            self.assertEqual(w.sha256(ROOT / name), rows[name]['sha256'])

    def test_all_manifest_components_avoid_edge_whitespace(self):
        bad = []
        for row in w.manifest_rows(ROOT):
            for part in PurePosixPath(row['path']).parts:
                if part.startswith(' ') or part.endswith((' ', '.')):
                    bad.append(row['path'])
                    break
        self.assertEqual(bad, [])

    def test_corresponding_cpp_assertion_uses_the_actual_uri(self):
        text = (ROOT / PREFIX / 'tests/tester.cc').read_text(encoding='utf-8')
        self.assertIn('REQUIRE(model.images[0].uri == "' + NAME + '");', text)
        self.assertIn('TEST_CASE("image-uri-spaces", "[issue-236]")', text)
        self.assertIn('REQUIRE(image_uri == image_uri_saved);', text)

    def test_lost_leading_space_is_reproduced_then_corrected(self):
        with tempfile.TemporaryDirectory(prefix='openmua2 filename ') as folder:
            root = Path(folder)
            data = (ROOT / NEW).read_bytes()
            (root / NAME).write_bytes(data)
            fixture_manifest(root, ' ' + NAME, data)
            before = (root / 'FILE-MANIFEST.json').read_bytes()
            with self.assertRaisesRegex(ValueError, '1 files differ'):
                w.verify_manifest(root)
            self.assertEqual((root / 'FILE-MANIFEST.json').read_bytes(), before)
            fixture_manifest(root, NAME, data)
            with redirect_stdout(io.StringIO()):
                self.assertEqual(w.verify_manifest(root), 1)

    def test_missing_portable_file_is_not_ignored(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture_manifest(root, NAME, b'original')
            with self.assertRaisesRegex(ValueError, '1 files differ'):
                w.verify_manifest(root)

    def test_corruption_is_not_blessed_by_the_patch(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            data = (ROOT / NEW).read_bytes()
            fixture_manifest(root, NAME, data)
            changed = data[:-1] + bytes([data[-1] ^ 1])
            (root / NAME).write_bytes(changed)
            with self.assertRaisesRegex(ValueError, '1 files differ'):
                w.verify_manifest(root)
            self.assertEqual((root / NAME).read_bytes(), changed)


if __name__ == '__main__':
    unittest.main(verbosity=2)
