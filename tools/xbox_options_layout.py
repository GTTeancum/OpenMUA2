"""Compile the private Options page into a readable Xbox control reference.

Reuse the native scene anchors and list widgets. No runtime overlay or original
game assets are distributed. Only the verified retail package is accepted.
"""
import hashlib
import struct
from xbox_tutorials import read_nodes

PACKAGE = 'packages/generated/maps/package/menus/options_rev.fb'
STOCK_HASH = '959320faf08481590a0c7b0a9ff98ae7fe1a4263483c6eb8135aa8e6e1ba4544'


def patch_scene(data):
    # Translation vectors of the four named native scene anchors. Keep the
    # list bounds, fonts, animation channels and the entire settings panel.
    # Their association was checked against the loaded igTransform nodes.
    anchors = ((12822, 0, 140), (17678, 24, 140),
               (26878, 0, -140), (22762, 0, -140))
    result = bytearray(data)
    for offset, dx, dz in anchors:
        x, y, z = struct.unpack_from('<3f', data, offset)
        if not (250 < x < 430 and -201 < y < -199 and 150 < z < 345):
            raise ValueError('Unexpected native Options anchor')
        struct.pack_into('<f', result, offset, x + dx)
        struct.pack_into('<f', result, offset + 8, z + dz)
    return bytes(result)


def patch_layout(data):
    result = bytearray(data)
    matched = set()
    def put(field, value):
        offset = len(result)
        result.extend(value.encode('cp1252') + b'\0')
        struct.pack_into('<I', result, field, offset)
    for _, attrs, _, fields in read_nodes(data):
        name = attrs.get('name', '')
        if name == 'controls':
            put(fields['model'], 'ui/models/m_invis')
            matched.add(name)
        elif name.startswith(('label_button', 'label_stick', 'label_click')):
            if 'text' in fields:
                descriptions = {
                    'label_button02': '\xe2 + \xa4/\xea/\xa5/\xa6  Powers',
                    'label_button03': '\xe1 + \xa4/\xa5/\xea/\xa6  Fusion',
                }
                put(fields['text'], descriptions.get(name, ''))
                if name in descriptions:
                    put(fields['textalignx'], 'TEXT_ALIGN_LEFT')
        elif name in ('controls_list', 'controls_list2'):
            put(fields['spacing'], '22')
            matched.add(name)
    if matched != {'controls', 'controls_list', 'controls_list2'}:
        raise ValueError('Missing native Options widgets')
    read_nodes(result)
    return bytes(result)


def patch_options_package(data):
    if hashlib.sha256(data).hexdigest() != STOCK_HASH:
        raise ValueError('Unknown/already modified Options package')
    result = bytearray()
    offset = 0
    changed = set()
    targets = {'ui/menus/cw_pausemenu.igb': patch_scene,
               'ui/menus/options_rev.engb': patch_layout,
               'ui/menus/options_rev.itab': patch_layout}
    while offset < len(data):
        header = data[offset:offset + 196]
        if len(header) != 196:
            raise ValueError('Truncated FB entry')
        name = header[:192].split(b'\0')[0].decode('ascii')
        size = struct.unpack_from('<I', header, 192)[0]
        payload = data[offset + 196:offset + 196 + size]
        if len(payload) != size:
            raise ValueError('Truncated FB payload')
        if name in targets:
            if name in changed:
                raise ValueError('Duplicate FB target')
            payload = targets[name](payload)
            changed.add(name)
        result.extend(header[:192] + struct.pack('<I', len(payload)) + payload)
        offset += 196 + size
    if changed != targets.keys():
        raise ValueError('Missing FB targets')
    return bytes(result)
