"""Convert the ten native Options control descriptions in private game data.

Use only verified stock tables. Reuse existing string allocations where possible;
append longer descriptions and update only their pointers. No game data is distributed.
"""
import hashlib
import struct

from xbox_tutorials import read_nodes

STOCK_HASHES = frozenset((
    '3eff2cfff34f3286197ec722ab34660c1417dc104084613fe0b2976355b53fc7',
    'd590d2a1586b0392645b3bf35dc7878f9b172df56193dedb6f44560d0148f5f5',
    '96f283836634eb8aa0edd19f01a1de7240f88f54ab5d51b06ad23425caffb55b',
))
REPLACEMENTS = {
    '1071': '$MenuAssignPowers Assign',
    '5515': '$XL Move',
    '5516': '$XR Camera',
    '5517': '$XA Light attack',
    '5518': '$XY Jump',
    '5519': '$XB Heavy attack',
    '5520': '$XX Grab / Use',
    '5521': '$Block Block',
    '5522': '$Pause Pause',
    '5523': '$XD Change hero',
    '5524': '$XV Heroes',
}


def patch_options_help(data):
    if hashlib.sha256(data).hexdigest() not in STOCK_HASHES:
        raise ValueError('Unknown/already-modified Options string table')
    nodes = read_nodes(data)
    selected = {}
    slots = []
    for _, attrs, locations, fields in nodes:
        ident = attrs.get('id')
        if ident not in REPLACEMENTS:
            continue
        if ident in selected or 'text' not in locations:
            raise ValueError('Missing/ambiguous Options description')
        start = locations['text']
        size = len(attrs['text'].encode('cp1252')) + 1
        # Never overwrite text shared with unrelated nodes or interior pointers.
        refs = sum(start <= p < start + size for _, _, loc, _ in nodes
                   for p in loc.values())
        if refs != 1:
            raise ValueError('Shared Options description allocation')
        if any(lo < start + size and start < hi for lo, hi in slots):
            raise ValueError('Overlapping Options descriptions')
        slots.append((start, start + size))
        selected[ident] = fields['text']
    if selected.keys() != REPLACEMENTS.keys():
        raise ValueError('Missing Options descriptions')
    # Long power/fusion chords fit larger, reclaimed description allocations.
    # Pair longest with largest; only pointers for these ten nodes may change.
    strings = sorted(((value.encode('ascii') + b'\0', key)
                      for key, value in REPLACEMENTS.items()),
                     key=lambda pair: (-len(pair[0]), pair[1]))
    slots.sort(key=lambda span: (-(span[1] - span[0]), span[0]))
    result = bytearray(data)
    allowed = set()
    for (text, ident), (start, end) in zip(strings, slots):
        if len(text) > 128:
            raise ValueError('Options description exceeds UI text limit')
        if len(text) > end - start:
            target = len(result)
            result.extend(text)
        else:
            target = start
            result[start:end] = text.ljust(end - start, b'\0')
            allowed.update(range(start, end))
        struct.pack_into('<I', result, selected[ident], target)
        allowed.update(range(selected[ident], selected[ident] + 4))
    if any(a != b and i not in allowed
                                     for i, (a, b) in enumerate(zip(data, result))):
        raise ValueError('Options write escaped selected descriptions')
    return bytes(result)
