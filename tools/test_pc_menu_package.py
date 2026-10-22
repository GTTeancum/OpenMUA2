"""Regression for the native XMLB attribute lookup contract."""
import struct
import unittest
import xml.etree.ElementTree as ET

from package_pc_menu_preview import encode_xml


def native_attribute(data, node, key):
    """Independent equivalent of MUA2's 801f1c70 binary search."""
    def string(offset):
        return data[offset:data.index(0, offset)].decode('cp1252')
    lower, upper = -1, struct.unpack_from('<I', data, node + 12)[0]
    while upper - lower > 1:
        middle = (lower + upper) // 2
        name, value = struct.unpack_from('<II', data, node + 16 + middle * 8)
        actual = string(name).lower()
        if key.lower() == actual:
            return string(value)
        if key.lower() < actual:
            upper = middle
        else:
            lower = middle
    return None


class NativeXmlLookupTest(unittest.TestCase):
    def test_authored_fields_remain_accessible_after_appending_properties(self):
        root = ET.Element('menu', {'type': 'OPTIONS_MENU', 'igb': 'x2m_options'})
        attrs = {'name': 'view_angle', 'style': 'STYLE_SEL_SMALL_WHITE',
                 'textalignx': 'TEXT_ALIGN_RIGHT', 'enabled': 'false',
                 'type': 'MENU_ITEM_BINARY', 'gamevar': 'view_shake',
                 'neverfocus': 'true'}
        ET.SubElement(root, 'item', attrs)
        data = encode_xml(root)
        child = struct.unpack_from('<I', data, 8 + 8)[0]
        for key, value in attrs.items():
            self.assertEqual(native_attribute(data, child, key), value)
        self.assertEqual(native_attribute(data, child, 'GAMEVAR'), 'view_shake')
        self.assertIsNone(native_attribute(data, child, 'missing'))
        self.assertEqual(native_attribute(data, 8, 'igb'), 'x2m_options')


if __name__ == '__main__':
    unittest.main()
