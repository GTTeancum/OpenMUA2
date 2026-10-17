"""Build a private MUA2 WAD with model-only MUA1 alternate costumes.

Inputs are read-only; output must be a new file. No game assets belong in Git.
Preserves MUA2 package contents except the actor model and resource-path aliases.
"""
import argparse
import copy
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path

from xbox_tutorials import read_nodes


def digest(data):
    return hashlib.sha256(data).hexdigest()


def members(blob):
    at = 0
    while at < len(blob):
        assert at + 196 <= len(blob)
        header = blob[at:at + 192]
        name = header[:128].split(b'\0')[0].decode('ascii')
        size = struct.unpack_from('<I', blob, at + 192)[0]
        end = at + 196 + size
        assert end <= len(blob)
        yield name, blob[at + 196:end], header
        at = end
    assert at == len(blob)


def decode(blob):
    read_nodes(blob)
    def word(at):
        return struct.unpack_from('<I', blob, at)[0]
    def string(at):
        return blob[at:blob.index(0, at)].decode('cp1252')
    def node(at):
        children = []
        child = word(at + 8)
        while child != 0xffffffff:
            children.append(node(child))
            child = word(child + 4)
        attrs = {string(word(at + 16 + i * 8)): string(word(at + 20 + i * 8))
                 for i in range(word(at + 12))}
        return [string(word(at)), attrs, children]
    return node(8)


def encode(tree):
    out = bytearray(struct.pack('<II', 0x11b1, 1))
    strings = []
    def node(item):
        name, attrs, children = item
        at = len(out)
        out.extend(struct.pack('<4I', 0, 0xffffffff, 0xffffffff, len(attrs)))
        strings.append((at, name))
        for key, value in attrs.items():
            off = len(out)
            out.extend(bytes(8))
            strings.extend(((off, key), (off + 4, value)))
        previous = None
        for child in children:
            off = node(child)
            struct.pack_into('<I', out, at + 8 if previous is None else previous + 4, off)
            previous = off
        return at
    assert node(tree) == 8
    positions = {}
    for off, value in strings:
        if value not in positions:
            positions[value] = len(out)
            out.extend(value.encode('cp1252') + b'\0')
        struct.pack_into('<I', out, off, positions[value])
    assert decode(bytes(out)) == tree
    return bytes(out)


def heroes(blob):
    return {attrs['name']: attrs for _, attrs, *_ in read_nodes(blob)
            if 'menulocation' in attrs and attrs.get('name') not in ('default', 'edit')}


def clone_package(blob, old, new, model, expected_models):
    out = bytearray()
    changed = 0
    for name, data, header in members(blob):
        if name == f'actors/{old}.igb':
            name = f'actors/{new}.igb'
            data = model
            changed += 1
        elif name == f'HUD/hud_head_{old}.igb':
            # The loader requests the selected skin's portrait path. Retain the
            # MUA2 portrait bytes, only alias its resource name to that skin.
            name = f'HUD/hud_head_{new}.igb'
        raw = name.encode('ascii')
        assert len(raw) < 128
        out.extend(raw + bytes(128 - len(raw)) + header[128:192])
        out.extend(struct.pack('<I', len(data)) + data)
    assert changed == expected_models
    return bytes(out)


def build(mua1, mua2, output, report):
    assert output.resolve() not in (mua1.resolve(), mua2.resolve())
    assert not output.exists(), 'Refusing to overwrite an existing output'
    patches = {}
    records = []
    with zipfile.ZipFile(mua1) as donor, zipfile.ZipFile(mua2) as base:
        assert len(set(base.namelist())) == len(base.namelist())
        first, second = heroes(donor.read('data/herostat.xmlb')), heroes(base.read('data/herostat.xmlb'))
        common = sorted(first.keys() & second.keys())
        assert common
        used = set()
        embedded_herostats = []
        for name in base.namelist():
            used.update(re.findall(r'\d{4,5}', name))
            if name.endswith('.fb'):
                for member, data, _ in members(base.read(name)):
                    used.update(re.findall(r'\d{4,5}', member))
                    if member.startswith('data/herostat.'):
                        embedded_herostats.append((name, member))
        # Fail rather than silently leave a second herostat inconsistent.
        assert not embedded_herostats, embedded_herostats
        for hero in common:
            source, target = first[hero], second[hero]
            assert not any(f'skin_{i:02}' in target for i in range(2, 11)), hero
            skins = [source['skin']] + [source['skin'][:-2] + source[f'skin_{i:02}'] for i in range(2, 5)]
            assert len(set(skins)) == 4
            for slot, old in enumerate(skins, 2):
                new = next(str(i) for i in range(9000 if len(old) == 4 else 90000,
                                                10000 if len(old) == 4 else 100000) if str(i) not in used)
                used.add(new)
                source_path = f'packages/generated/characters/{hero.lower()}_{old}.fb'
                model = next(data for name, data, _ in members(donor.read(source_path)) if name == f'actors/{old}.igb')
                original_hash = digest(model)
                # Fixed-width identity edits leave every IGB offset intact.
                for suffix in (b'\0', b'_skel\0'):
                    needle = old.encode() + suffix
                    assert model.count(needle) == 1, (hero, old, suffix)
                    model = model.replace(needle, new.encode() + suffix)
                packages = []
                for suffix in ('', '_c', '_nc'):
                    template = f"packages/generated/characters/{hero.lower()}_{target['skin']}{suffix}.fb"
                    destination = f'packages/generated/characters/{hero.lower()}_{new}{suffix}.fb'
                    assert destination not in base.namelist()
                    patches[destination] = clone_package(base.read(template), target['skin'], new, model,
                                                         0 if suffix == '_c' else 1)
                    packages.append({'template': template, 'destination': destination})
                records.append(dict(hero=hero, slot=f'skin_{slot:02}', source_model=old,
                                    model=new, donor_sha256=original_hash, model_sha256=digest(model), packages=packages))
        for name in ('data/herostat.xmlb', 'data/herostat.engb', 'data/herostat.itab'):
            original = decode(base.read(name))
            tree = copy.deepcopy(original)
            for row in records:
                node = next(n for n in tree[2] if n[1].get('name') == row['hero'])
                assert row['slot'] not in node[1]
                node[1][row['slot']] = row['model']
            patches[name] = encode(tree)
            restored = decode(patches[name])
            for row in records:
                node = next(n for n in restored[2] if n[1].get('name') == row['hero'])
                assert node[1].pop(row['slot']) == row['model']
            assert restored == original
        with zipfile.ZipFile(output, 'x') as out:
            for item in base.infolist():
                out.writestr(copy.copy(item), patches.get(item.filename, base.read(item.filename)))
            for name, data in patches.items():
                if name not in base.namelist():
                    out.writestr(name, data, compress_type=zipfile.ZIP_DEFLATED)
        with zipfile.ZipFile(output) as result:
            assert result.testzip() is None
            for name in base.namelist():
                if name not in patches:
                    assert result.read(name) == base.read(name), name
            for row in records:
                for pair in row['packages']:
                    before = list(members(base.read(pair['template'])))
                    after = list(members(result.read(pair['destination'])))
                    assert len(before) == len(after)
                    for (n, data, header), (nn, dd, hh) in zip(before, after):
                        assert header[128:] == hh[128:]
                        if n == f"actors/{second[row['hero']]['skin']}.igb":
                            assert nn == f"actors/{row['model']}.igb" and digest(dd) == row['model_sha256']
                        else:
                            assert data == dd, (pair, n)
                            assert nn == n or (n == f"HUD/hud_head_{second[row['hero']]['skin']}.igb" and nn == f"HUD/hud_head_{row['model']}.igb")
            summary = dict(common_characters=common, alternate_models=len(records), new_packages=len(records)*3,
                           costumes_per_character=5, non_model_resources_verified_unchanged=True,
                           unchanged_original_entries=len(base.namelist())-3, records=records,
                           in_game_rendering_verified=False)
            report.write_text(json.dumps(summary, indent=2) + '\n')
            print(json.dumps({k: v for k, v in summary.items() if k != 'records'}, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('mua1', 'mua2', 'output', 'report'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    build(args.mua1, args.mua2, args.output, args.report)
