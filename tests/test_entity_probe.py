import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('entity_probe', Path(__file__).parents[1] / 'tools/analyze_entity_probe.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def fixture(identity, health=20, generation=13, flags=1, x=10):
    data = bytearray(m.STRIDE)
    struct.pack_into('>I', data, 0, flags)
    struct.pack_into('>I', data, 0x44, generation << 24 | identity)
    struct.pack_into('>3f', data, 0x4c, x, 20, 30)
    struct.pack_into('>3f', data, 0x2a0, health, 0.25, 100)
    return data


class EntityProbeTests(unittest.TestCase):
    def test_tags_are_masked_and_health_is_on_entity_not_attribute_pointer(self):
        data = fixture(5) + fixture(8, -2)
        before = bytes(data)
        r = m.analyze(data, 13, [5], [8], 2)
        self.assertEqual(r['heroes_alive'], 1)
        self.assertEqual(r['opponents_with_positive_health'], 0)
        self.assertEqual(r['entities'][0]['regeneration'], 0.25)
        self.assertEqual(bytes(data), before)

    def test_stale_and_freed_slots_do_not_count_as_living_opponents(self):
        data = fixture(5) + fixture(8, generation=12) + fixture(8, flags=0) + fixture(8)
        r = m.analyze(data, 13, [5], [8], 4)
        self.assertEqual(r['opponents_with_positive_health'], 1)
        self.assertEqual(r['rejected_slots']['empty'], 1)
        self.assertEqual(r['rejected_slots']['stale_generation'], 1)

    def test_missing_duplicate_and_stale_heroes_reject(self):
        for data in [fixture(8), fixture(5) + fixture(5), fixture(5, generation=12)]:
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                m.analyze(data, 13, [5], [8], len(data) // m.STRIDE)

    def test_nonfinite_and_invalid_health_fields_reject(self):
        for value in [float('nan'), float('inf'), -float('inf'), 1e10]:
            with self.subTest(value=value), self.assertRaises(ValueError):
                m.analyze(fixture(5, value), 13, [5], [], 1)
        with self.assertRaises(ValueError):
            m.analyze(fixture(5, x=float('nan')), 13, [5], [], 1)

    def test_exact_length_and_bounded_contract_required(self):
        for slots, data in [(1, fixture(5)[:-1]), (1, fixture(5) + b'x'), (0, b''), (513, b'')]:
            with self.subTest(slots=slots), self.assertRaises(ValueError):
                m.analyze(data, 13, [5], [], slots)
        for generation, heroes, opponents in [(0, [5], []), (256, [5], []), (13, [], []),
                                               (13, [5, 5], []), (13, [5], [5]), (13, [4096], [])]:
            with self.subTest(generation=generation, heroes=heroes), self.assertRaises(ValueError):
                m.analyze(fixture(5), generation, heroes, opponents, 1)

    def test_guarded_probe_rejects_generation_change_and_truncated_header(self):
        contract = {'generation': 13, 'hero_ids': [5], 'opponent_ids': [], 'slots': 1}
        self.assertEqual(m.analyze_guarded(fixture(5), (13).to_bytes(4, 'big'), contract)['heroes_alive'], 1)
        for header in [(14).to_bytes(4, 'big'), b'\r', b'\x00' * 5]:
            with self.subTest(header=header), self.assertRaises(ValueError):
                m.analyze_guarded(fixture(5, generation=14), header, contract)

    def test_unclassified_entities_are_not_enemies(self):
        r = m.analyze(fixture(5) + fixture(7), 13, [5], [8], 2)
        self.assertEqual(r['opponents_with_positive_health'], 0)
        self.assertEqual(r['rejected_slots']['unclassified'], 1)


if __name__ == '__main__':
    unittest.main()
