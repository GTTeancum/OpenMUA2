"""Regression coverage for cross-platform Alchemy memory alignment."""
import copy
import struct
from types import SimpleNamespace as NS
import unittest

from pc_menu_geometry import alignment_names, set_memory_alignment, transfer_menu_graph


def table(names):
    body = struct.pack('<2I', 1, len(names))
    body += struct.pack('<' + 'I' * len(names), *(len(n) + 1 for n in names))
    body += b''.join(n + b'\0' for n in names)
    return struct.pack('<I', len(body) + 4) + body


def writer(names, alignment=-1):
    return NS(alignment_data=table(names), objects=[NS()],
              ref_info=[dict(is_object=False, align_type_idx=alignment, mem_pool_handle=-1)],
              entries=[NS(type_index=0, raw_bytes=b'old', field_values=[alignment, -1])],
              index_map=[0], memory_pool_names=[],
              meta_objects=[NS(fields=[NS(slot=12, short_name=b'Int'),
                                       NS(slot=13, short_name=b'Int')])])


class AlignmentTest(unittest.TestCase):
    def test_pc_image_requests_native_image_alignment(self):
        w = writer([b'VertexArrayData', b'ImageData', b'VertexData'])
        set_memory_alignment(w, 0, b'ImageData')
        self.assertEqual(w.ref_info[0]['align_type_idx'], 1)
        self.assertEqual(w.entries[w.index_map[0]].field_values, [1, -1])
        self.assertIsNone(w.entries[w.index_map[0]].raw_bytes)
        self.assertEqual(w.entries[0].field_values, [-1, -1])

    def test_copy_uses_alignment_name_not_source_table_index(self):
        source = writer([b'GCVertexData', b'ClutData', b'ImageData', b'VertexArrayData'], 0)
        destination = writer([b'VertexArrayData', b'ImageData', b'VertexData'])
        # The copier receives the rewritten directory and ref-info together.
        def copy_without_alignment(dest, src, index):
            result = len(dest.objects)
            dest.objects.append(copy.deepcopy(src.objects[index]))
            dest.ref_info.append(copy.deepcopy(src.ref_info[index]))
            dest.ref_info[-1]['align_type_idx'] = -1
            entry = copy.deepcopy(src.entries[src.index_map[index]])
            entry.field_values[0] = -1
            dest.index_map.append(len(dest.entries)); dest.entries.append(entry)
            return result
        graph = NS(transfer_graph=copy_without_alignment)
        copied = transfer_menu_graph(destination, source, 0, graph)
        names = alignment_names(destination)
        index = destination.ref_info[copied]['align_type_idx']
        self.assertEqual(names[index], b'GCVertexData')
        self.assertEqual(destination.entries[destination.index_map[copied]].field_values[0], index)
        self.assertEqual(names[:3], [b'VertexArrayData', b'ImageData', b'VertexData'])
        self.assertEqual(source.ref_info[0]['align_type_idx'], 0)
        self.assertEqual(source.entries[0].raw_bytes, b'old')


if __name__ == '__main__':
    unittest.main()
