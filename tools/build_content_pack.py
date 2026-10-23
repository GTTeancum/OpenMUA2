#!/usr/bin/env python3
"""Build the release content pack for OpenMUA2.exe's first-run setup.

Compares the player's clean disc archive with the prepared OpenMUA2 archive and
writes only the added or changed members to content.zip. content.manifest lists
every member of the finished archive in order, with its SHA-256, CRC-32, size
and source (d = player's disc, c = content.zip). The launcher rebuilds the
archive from those two sources and checks every member hash.

Members are written with their original compression method. No game archive
member that is unchanged from the disc is placed in the output.
"""
import argparse
import hashlib
import zipfile
from pathlib import Path


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--disc-wad', type=Path, required=True,
                    help='files/z/assetsfb.wad from a clean disc extraction')
    ap.add_argument('--openmua2-wad', type=Path, required=True,
                    help='the prepared OpenMUA2 assetsfb.wad')
    ap.add_argument('--output-dir', type=Path, required=True)
    args = ap.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    pack = args.output_dir / 'content.zip'
    manifest = args.output_dir / 'content.manifest'
    rows = ['OpenMUA2-Content-v1']
    custom = 0
    with zipfile.ZipFile(args.disc_wad) as disc, zipfile.ZipFile(args.openmua2_wad) as mod, \
            zipfile.ZipFile(pack.with_suffix('.partial'), 'w') as out:
        disc_hashes = {i.filename: hashlib.sha256(disc.read(i)).hexdigest() for i in disc.infolist()}
        seen = set()
        for info in mod.infolist():
            if info.filename in seen:
                raise SystemExit(f'duplicate member {info.filename}')
            seen.add(info.filename)
            data = mod.read(info)
            digest = hashlib.sha256(data).hexdigest()
            source = 'd' if disc_hashes.get(info.filename) == digest else 'c'
            if source == 'c':
                entry = zipfile.ZipInfo(info.filename, info.date_time)
                entry.compress_type = info.compress_type
                entry.external_attr = info.external_attr
                out.writestr(entry, data)
                custom += 1
            rows.append(f'{digest}\t{info.CRC:08x}\t{info.file_size}\t{source}\t{info.filename}')
    pack.with_suffix('.partial').replace(pack)
    manifest.write_text('\n'.join(rows) + '\n', encoding='utf-8', newline='\n')
    print(f'{custom} OpenMUA2 members of {len(rows) - 1}; {pack.stat().st_size} bytes -> {pack}')


if __name__ == '__main__':
    main()
