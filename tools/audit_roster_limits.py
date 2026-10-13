"""Read-only inventory of verified native roster boundaries; never edits game data.

Run against the authoritative extracted main.dol, not a code-generation composite.
Only hashes, addresses and capacity findings are emitted; no game bytes are copied.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct


# These identify complete inspected instruction spans, not global number matches.
SPANS = (
    ('character_info_loader', 0x800B2FF4, 40,
     'a05a11e9d8df89421ef7127195edcb33b612dfe1b5a8c722d16e3a452167917c'),
    ('character_info_singleton', 0x800A0E74, 484,
     '532c58ce7a7c673a71ec039fba2a8b1e9eb4142e7f5c4c39830b9b9f1877e2b4'),
    ('hero_snapshot_constructors', 0x800A0750, 144,
     'e1ff38cfecc213eaf0eef90fb3ed4867467e8bb31c5f4fa4ce0a6609c2ff23e4'),
    ('hero_snapshot_save_load', 0x80099C24, 660,
     '1767731fded5fa7f940e325869e40c6554e4d2d2959b61587f6f01255c65405f'),
    ('hero_auxiliary_allocation', 0x8009F060, 192,
     'cf940b99e2085e4ef527a00d30413d6d324d87322e79c037dae966d724330a7e'),
    ('full_stats_pool_constructor', 0x800A0C34, 560,
     'e00482f57846d3c2f20b77d5db2758ca9071e0d8a7d4326e9613b3c933414b35'),
)


def read_dol_span(data, address, size):
    if len(data) < 0x100 or size <= 0:
        raise ValueError('Invalid DOL header or span')
    matches = []
    for i in range(18):
        offset, start, length = (struct.unpack_from('>I', data, base + 4*i)[0]
                                 for base in (0, 0x48, 0x90))
        if start <= address and address + size <= start + length:
            position = offset + address - start
            if offset < 0x100 or offset + length > len(data):
                raise ValueError('DOL section extends outside the file')
            matches.append(data[position:position + size])
    if len(matches) != 1:
        raise ValueError(f'Unmapped or ambiguous DOL span at {address:08x}')
    return matches[0]


def audit(data):
    checks = []
    for name, address, size, expected in SPANS:
        actual = hashlib.sha256(read_dol_span(data, address, size)).hexdigest()
        checks.append(dict(name=name, address=f'{address:08x}', size=size,
                           sha256=actual, matches_supported_code=actual == expected))
    verified = all(check['matches_supported_code'] for check in checks)
    return dict(
        schema=1, read_only=True, supported_spans_verified=verified,
        executable_sha256=hashlib.sha256(data).hexdigest(), checks=checks,
        # Findings apply only if all the inspected spans match.
        capacities=(dict(character_info=36, hero_snapshot_banks=2,
                         hero_snapshots_per_bank=33, hero_snapshot_stride=800,
                         hero_auxiliary_pool=33, full_stats_pool=44)
                    if verified else None),
        limitations=['Static inventory only; no expansion or runtime validation.',
                     'Selection REL, save compatibility and pool headroom require separate checks.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dol', required=True, type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = audit(args.dol.read_bytes())
    text = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(text.encode('utf-8'))
    else:
        print(text, end='')
    return 0 if result['supported_spans_verified'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
