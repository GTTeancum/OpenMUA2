"""Read-only, numeric analysis of bounded OpenMUA2 entity-pool probes.

This is diagnostic evidence, not a combat or visual-correctness acceptance gate.
The caller supplies a private, state-specific identity contract. No game strings,
code, memory writes, process access or desktop input are used by this module.
"""
import math
import struct

STRIDE = 0xA38
MAX_SLOTS = 512


def analyze(pool: bytes, generation: int, hero_ids: list[int], opponent_ids: list[int],
            slots: int) -> dict:
    if not 1 <= slots <= MAX_SLOTS or len(pool) != slots * STRIDE:
        raise ValueError('probe length must exactly match the bounded slot count')
    if not 1 <= generation <= 255:
        raise ValueError('invalid expected string-table generation')
    heroes, opponents = set(hero_ids), set(opponent_ids)
    if not heroes or len(heroes) != len(hero_ids) or heroes & opponents:
        raise ValueError('hero identities must be unique and disjoint from opponents')
    if any(not 0 < n < 4096 for n in heroes | opponents):
        raise ValueError('identity outside the inspected string-table bounds')
    rows = []
    rejected = {'empty': 0, 'stale_generation': 0, 'unclassified': 0}
    for slot in range(slots):
        data = memoryview(pool)[slot * STRIDE:(slot + 1) * STRIDE]
        flags = struct.unpack_from('>I', data, 0)[0]
        if not flags:
            rejected['empty'] += 1
            continue
        tag = struct.unpack_from('>I', data, 0x44)[0]
        if tag >> 24 != generation:
            rejected['stale_generation'] += 1
            continue
        identity = tag & 0xffffff
        if identity not in heroes | opponents:
            rejected['unclassified'] += 1
            continue
        position = struct.unpack_from('>3f', data, 0x4c)
        health, regeneration, maximum = struct.unpack_from('>3f', data, 0x2a0)
        if not all(math.isfinite(v) for v in (*position, health, regeneration, maximum)):
            raise ValueError('non-finite field in a classified entity')
        if any(abs(v) > 1e6 for v in position) or not 0 < maximum <= 1e6 or abs(health) > 1e6:
            raise ValueError('classified entity field outside diagnostic bounds')
        rows.append({'slot': slot, 'identity': identity, 'generation': generation,
                     'kind': 'hero' if identity in heroes else 'opponent',
                     'flags': flags, 'position': list(position), 'health': health,
                     'regeneration': regeneration, 'maximum_health': maximum})
    observed = [r['identity'] for r in rows if r['kind'] == 'hero']
    if sorted(observed) != sorted(heroes):
        raise ValueError('missing or duplicate expected hero identity')
    return {'scope': 'Bounded, identity-guarded entity observations; not all enemies, AI activity, '
                     'continuous combat, visual correctness or FPS acceptance.',
            'generation': generation, 'slots_inspected': slots, 'rejected_slots': rejected,
            'heroes_alive': sum(r['kind'] == 'hero' and r['health'] > 0 for r in rows),
            'opponents_with_positive_health': sum(r['kind'] == 'opponent' and r['health'] > 0 for r in rows),
            'entities': rows}


def analyze_guarded(pool: bytes, generation_data: bytes, contract: dict) -> dict:
    if len(generation_data) != 4:
        raise ValueError('generation probe must contain exactly four bytes')
    observed = int.from_bytes(generation_data, 'big')
    if observed != contract['generation']:
        raise ValueError('string-table generation changed; contract must be re-established')
    return analyze(pool, observed, contract['hero_ids'], contract['opponent_ids'], contract['slots'])


def main():
    import argparse
    import json
    from pathlib import Path
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pool', type=Path, required=True)
    parser.add_argument('--generation-file', type=Path, required=True)
    parser.add_argument('--contract', type=Path, required=True,
                        help='Private state-specific JSON: generation, hero_ids, opponent_ids, slots')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = analyze_guarded(args.pool.read_bytes(), args.generation_file.read_bytes(),
                             json.loads(args.contract.read_text()))
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
