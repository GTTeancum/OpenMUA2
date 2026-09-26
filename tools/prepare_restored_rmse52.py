#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Make a verified persistent RMSE52 extracted tree bootable by Dolphin.

The private persistent archive keeps Wii partition metadata under ``disc-meta/``
so it can be distinguished from normal game files. Dolphin's DirectoryBlob
reader expects the partition files at the extracted-game root and the disc
header/region under ``disc/``. This tool creates only those compatibility
copies/slices; it never changes sys/main.dol, the REL, or disc-meta sources.
"""
from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import tempfile

EXPECTED_DOL_SHA256 = "0857973ed7646eaf1294981295673243546c935cdbd1fd07345b4d1093c62741"
EXPECTED_REL_SHA256 = "5b739b1046b6987897f078c27f54c214cfe7a1b0381bca0ee29754b57c8a6a7f"
REL_PATH = Path("files/Marvel-rev-fin-plf2.rel")
DISC_HEADER_END = 0x100
REGION_START = 0x4E000
REGION_END = 0x4E020


def sha256(path: Path) -> str:
    with path.open("rb") as f:
        return hashlib.file_digest(f, "sha256").hexdigest()


def validate_rmse52(game: Path) -> None:
    dol = game / "sys/main.dol"
    rel = game / REL_PATH
    if not dol.is_file() or not rel.is_file():
        raise ValueError("RMSE52 tree is missing sys/main.dol or Marvel-rev-fin-plf2.rel")
    if sha256(dol) != EXPECTED_DOL_SHA256:
        raise ValueError("RMSE52 main.dol hash does not match the pinned USA build")
    if sha256(rel) != EXPECTED_REL_SHA256:
        raise ValueError("RMSE52 REL hash does not match the pinned USA build")


def _publish_exact(target: Path, data: bytes) -> str:
    if target.exists():
        if not target.is_file():
            raise ValueError(f"compatibility target is not a file: {target}")
        if target.read_bytes() != data:
            raise ValueError(f"refusing to overwrite differing compatibility file: {target}")
        return "verified"
    target.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix=target.name + ".", suffix=".tmp", dir=target.parent)
    temp = Path(temp_name)
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data)
        os.replace(temp, target)
    finally:
        temp.unlink(missing_ok=True)
    return "created"


def repair_layout(game: Path) -> dict[str, str]:
    game = game.resolve()
    meta = game / "disc-meta"
    if not meta.is_dir():
        raise ValueError(f"persistent disc-meta directory is missing: {meta}")

    outputs: dict[str, bytes] = {}
    for name in ("ticket.bin", "tmd.bin", "cert.bin", "h3.bin"):
        source = meta / name
        if not source.is_file():
            raise ValueError(f"persistent metadata file is missing: {source}")
        outputs[name] = source.read_bytes()

    raw_header = meta / "disc-header.bin"
    if not raw_header.is_file():
        raise ValueError(f"persistent disc header is missing: {raw_header}")
    raw = raw_header.read_bytes()
    if len(raw) < REGION_END:
        raise ValueError(
            f"persistent disc header is too short: need at least 0x{REGION_END:X} bytes"
        )
    outputs["disc/header.bin"] = raw[:DISC_HEADER_END]
    outputs["disc/region.bin"] = raw[REGION_START:REGION_END]

    result: dict[str, str] = {}
    for relative, data in outputs.items():
        result[relative] = _publish_exact(game / relative, data)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("game_root", type=Path, help="restored extracted RMSE52 directory")
    args = parser.parse_args()
    game = args.game_root.resolve()
    validate_rmse52(game)
    result = repair_layout(game)
    created = sum(value == "created" for value in result.values())
    verified = sum(value == "verified" for value in result.values())
    print(f"RMSE52 Dolphin layout ready: {created} created, {verified} already verified")
    for path, state in result.items():
        print(f"  {state:8s} {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
