"""Private Xbox tutorial conversion for the verified stock Rev tip tables.

No game text or generated table is distributed. Replacements fit existing string
allocations, preserving the XMLB graph and containing FB entry sizes.
"""
import hashlib
import struct

STOCK_HASHES = frozenset((
    '523cbfb5e0ffd26c1bf35cc1a0100a1f3d500c60e442db16b4cdd7b13b14334f',
    '829cdd6c486178ba85abb4a8090a3751d0d7f86d4882bfdc21c09ee80a1a6357',
    'e3ebcaf05dbac2b51f7b86d902149f23db77940da8b78a130642a7f2d3ba516a',
))

# Explicit Xbox tokens require the paired guarded prompt resolver/font pack.
REPLACEMENTS = {
    '3': ('Grab', 'UT: Press $ACTION to grab an enemy.'),
    '5': ('SwitchToNextHero', 'UT: Use $XD to select a hero.'),
    '9': ('SuperPowers', 'UT: Hold $XRT + $XA/$XX/$XB/$XY for powers 1/2/3/4.'),
    '12': ('Fusion', 'UT: Hold $XLT + $XA/$XB/$XX/$XY for Fusion.'),
    # Gameplay provider has fixed power slots; LB+D-pad blocks/selects a hero.
    '30': ('Quickassign', 'UT: Hold $XRT + $XA/$XX/$XB/$XY to use powers.'),
    '31': ('RotCam', 'UT: Move $XR left or right to rotate the camera.'),
    '47': ('Fusionattack', r'UT: \nHold $XLT + $XA/$XB/$XX/$XY to choose a Fusion partner.\n'),
    '48': ('Projectiles', r'UT: \nHold $XRT + $XA/$XX/$XB/$XY to use a ranged power.\n'),
    '51': ('SwitchingHeroes', r'UT: \nSelect a hero with $XD Up, Right, Down or Left.\n'),
    '53': ('Powers', r'UT: \nHold $XRT + $XA/$XX/$XB/$XY for powers 1/2/3/4.\n'),
    '54': ('Safeguard', r'UT: \nRepeatedly press $XX to complete the safeguard interaction.\n'),
    '56': ('GrabBasics', r'UT: \nPress $ACTION to grab enemies or objects. Tap $ATTACK to punch a grabbed enemy.\n'),
    '59': ('CameraControl', r'UT: \nIn some areas, move $XR left or right to rotate the camera.\n'),
}


def read_nodes(data):
    """Read bounded XMLB nodes and retain string offsets for in-place edits."""
    def word(offset):
        if offset < 0 or offset + 4 > len(data):
            raise ValueError('Truncated XMLB word')
        return struct.unpack_from('<I', data, offset)[0]
    if word(0) != 0x11b1 or word(4) != 1:
        raise ValueError('Unexpected XMLB format')
    strings = word(8)
    if not 24 <= strings < len(data):
        raise ValueError('Invalid XMLB string table')
    def string(offset):
        if not strings <= offset < len(data):
            raise ValueError('Invalid XMLB string offset')
        end = data.find(b'\0', offset)
        if end < 0:
            raise ValueError('Unterminated XMLB string')
        return data[offset:end].decode('cp1252')
    pending, seen, nodes = [8], set(), []
    while pending:
        offset = pending.pop()
        if offset in seen or offset < 8 or offset % 4 or len(seen) >= 4096:
            raise ValueError('Invalid XMLB node graph')
        seen.add(offset)
        count = word(offset + 12)
        if count > 64 or offset + 16 + count * 8 > strings:
            raise ValueError('Invalid XMLB attributes')
        name = string(word(offset))
        attrs, locations, fields = {}, {}, {}
        for index in range(count):
            key = string(word(offset + 16 + index * 8))
            value = word(offset + 20 + index * 8)
            if key in attrs:
                raise ValueError('Duplicate XMLB attribute')
            attrs[key], locations[key] = string(value), value
            fields[key] = offset + 20 + index * 8
        nodes.append((name, attrs, locations, fields))
        for link in (word(offset + 4), word(offset + 8)):
            if link != 0xffffffff:
                pending.append(link)
    return nodes


def patch_tutorials(data):
    if hashlib.sha256(data).hexdigest() not in STOCK_HASHES:
        raise ValueError('Unknown/already-modified tutorial table')
    nodes = read_nodes(data)
    result, matched, spans = bytearray(data), set(), []
    spare, shared, relocated = [], [], {}
    for _, attrs, locations, fields in nodes:
        ident = attrs.get('id')
        if attrs.get('platform') != 'rev' or ident not in REPLACEMENTS:
            continue
        title, text = REPLACEMENTS[ident]
        if ident in matched or attrs.get('title') != title:
            raise ValueError('Missing/ambiguous tutorial identity')
        matched.add(ident)
        start = locations['text']
        size = len(attrs['text'].encode('cp1252'))
        replacement = text.encode('cp1252')
        if len(replacement) > size:
            raise ValueError('Replacement exceeds tutorial allocation: ' + ident)
        if any(lo < start + size and start < hi for lo, hi in spans):
            raise ValueError('Overlapping tutorial allocations')
        # Do not accidentally change another node sharing this string.
        references = sum(start <= p < start + size for _, _, loc, _ in nodes for p in loc.values())
        if references != 1:
            shared.append((ident, fields['text'], replacement))
            continue
        spans.append((start, start + size))
        result[start:start + size] = replacement.ljust(size, b'\0')
        spare.append([start + len(replacement) + 1, start + size + 1])
    for ident, pointer, replacement in shared:
        allocation = next((s for s in spare if s[1] - s[0] >= len(replacement) + 1), None)
        if allocation is None:
            raise ValueError('No space to separate a shared tutorial string')
        start = allocation[0]
        result[start:start + len(replacement) + 1] = replacement + b'\0'
        struct.pack_into('<I', result, pointer, start)
        spans.append((pointer, pointer + 4))
        spans.append((start, start + len(replacement) + 1))
        relocated[ident] = start
        allocation[0] += len(replacement) + 1
    if matched != REPLACEMENTS.keys():
        raise ValueError('Missing Xbox tutorial targets')
    # Validate semantic structure after writing, including every untouched node.
    after = read_nodes(result)
    for (name, attrs, loc, fields), (other_name, other_attrs, other_loc, other_fields) in zip(nodes, after, strict=True):
        expected_loc = dict(loc)
        expected = dict(attrs)
        if attrs.get('platform') == 'rev' and attrs.get('id') in REPLACEMENTS:
            expected['text'] = REPLACEMENTS[attrs['id']][1]
            if attrs['id'] in relocated:
                expected_loc['text'] = relocated[attrs['id']]
        if (name, expected, expected_loc, fields) != (other_name, other_attrs, other_loc, other_fields):
            raise ValueError('Unexpected tutorial structure change')
    if len(result) != len(data) or any(a != b and not any(lo <= i < hi for lo, hi in spans)
                                     for i, (a, b) in enumerate(zip(data, result))):
        raise ValueError('Write escaped tutorial text allocations')
    return bytes(result)
