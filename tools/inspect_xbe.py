# SPDX-License-Identifier: GPL-3.0-or-later
"""Inspect an original-Xbox XBE: memory image, sections, XDK libraries, budget.

Used by the original-Xbox plan (docs/ORIGINAL-XBOX-PORT-PLAN.md):

* P1.1: measure a legally owned MUA1 Xbox XBE locally. It shows how much code and
  data a shipped Alchemy 3.2 game fits into 64 MiB, which XDK libraries it links,
  and its load-time image size. Only the measurements are kept. The XBE itself is
  never committed or uploaded.
* CI: check every XBE this branch builds against the maintainer's < 10 MB budget.

Only header structures are parsed (the publicly documented XBE layout). No code or
data is extracted, decrypted or modified.

    python tools/inspect_xbe.py path/to/default.xbe [--json] [--budget-bytes N]

The exit status is 0 when the XBE parses and is within budget, 1 when it is over
budget, and 2 when the file is malformed.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path

MAGIC = b"XBEH"
DEFAULT_BUDGET_BYTES = 10 * 1000 * 1000  # maintainer requirement: XBE < 10 MB

# Entry point / kernel thunk XOR keys (retail, debug), from the public XBE docs.
ENTRY_KEYS = {"retail": 0xA8FC57AB, "debug": 0x94859D4B}
THUNK_KEYS = {"retail": 0x5B6D40B6, "debug": 0xEFB1F152}

SECTION_FLAGS = {
    0x01: "writable",
    0x02: "preload",
    0x04: "executable",
    0x08: "inserted_file",
    0x10: "head_page_ro",
    0x20: "tail_page_ro",
}


class XbeError(ValueError):
    pass


@dataclass
class Section:
    name: str
    flags: list[str]
    virtual_address: int
    virtual_size: int
    raw_address: int
    raw_size: int


@dataclass
class Library:
    name: str
    version: str
    flags: int


@dataclass
class XbeInfo:
    path: str
    file_size: int
    base_address: int
    size_of_headers: int
    size_of_image: int
    timestamp: int
    init_flags: int
    entry_point: int | None
    build_kind: str | None
    title_id: int | None
    title_name: str | None
    stack_commit: int
    heap_reserve: int
    heap_commit: int
    sections: list[Section] = field(default_factory=list)
    libraries: list[Library] = field(default_factory=list)
    totals: dict[str, int] = field(default_factory=dict)


def _u32(data: bytes, off: int) -> int:
    if off < 0 or off + 4 > len(data):
        raise XbeError(f"read past end of file at 0x{off:x}")
    return struct.unpack_from("<I", data, off)[0]


def _cstr(data: bytes, off: int, limit: int = 64) -> str:
    if off < 0 or off >= len(data):
        raise XbeError(f"string offset 0x{off:x} outside file")
    end = data.find(b"\0", off, off + limit)
    raw = data[off:end if end >= 0 else off + limit]
    return raw.decode("ascii", "replace")


def parse(data: bytes, path: str = "<memory>") -> XbeInfo:
    if len(data) < 0x178 or data[:4] != MAGIC:
        raise XbeError("not an XBE (missing XBEH magic or truncated header)")
    base = _u32(data, 0x104)
    size_of_headers = _u32(data, 0x108)

    def off(vaddr: int) -> int:
        """Translate a header-area virtual address to a file offset."""
        o = vaddr - base
        if vaddr == 0 or o < 0 or o >= min(len(data), max(size_of_headers, 0x178)):
            raise XbeError(f"header pointer 0x{vaddr:08x} outside header area")
        return o

    entry_enc = _u32(data, 0x128)
    thunk_enc = _u32(data, 0x158)
    build_kind = entry = None
    # The entry point must decode into the image, and the thunk into the kernel
    # import area; the key pair that satisfies both identifies retail vs debug.
    image_end = base + _u32(data, 0x10C)
    for kind in ("retail", "debug"):
        e = entry_enc ^ ENTRY_KEYS[kind]
        t = thunk_enc ^ THUNK_KEYS[kind]
        if base <= e < image_end and base <= t < image_end:
            build_kind, entry = kind, e
            break

    info = XbeInfo(
        path=path,
        file_size=len(data),
        base_address=base,
        size_of_headers=size_of_headers,
        size_of_image=_u32(data, 0x10C),
        timestamp=_u32(data, 0x114),
        init_flags=_u32(data, 0x124),
        entry_point=entry,
        build_kind=build_kind,
        title_id=None,
        title_name=None,
        stack_commit=_u32(data, 0x130),
        heap_reserve=_u32(data, 0x134),
        heap_commit=_u32(data, 0x138),
    )

    cert_addr = _u32(data, 0x118)
    if cert_addr:
        c = off(cert_addr)
        info.title_id = _u32(data, c + 0x8)
        name_raw = data[c + 0xC:c + 0xC + 80]
        info.title_name = name_raw.decode("utf-16-le", "replace").split("\0", 1)[0]

    count = _u32(data, 0x11C)
    if count > 512:
        raise XbeError(f"implausible section count {count}")
    sh = off(_u32(data, 0x120)) if count else 0
    for i in range(count):
        h = sh + i * 0x38
        flags_raw = _u32(data, h)
        name_addr = _u32(data, h + 0x14)
        sec = Section(
            name=_cstr(data, off(name_addr)) if name_addr else f"#{i}",
            flags=[n for bit, n in SECTION_FLAGS.items() if flags_raw & bit],
            virtual_address=_u32(data, h + 0x4),
            virtual_size=_u32(data, h + 0x8),
            raw_address=_u32(data, h + 0xC),
            raw_size=_u32(data, h + 0x10),
        )
        if sec.raw_address + sec.raw_size > len(data):
            raise XbeError(f"section {sec.name} raw data runs past end of file")
        info.sections.append(sec)

    lib_count = _u32(data, 0x160)
    lib_addr = _u32(data, 0x164)
    if lib_count and lib_addr:
        if lib_count > 256:
            raise XbeError(f"implausible library count {lib_count}")
        lo = off(lib_addr)
        for i in range(lib_count):
            o = lo + i * 16
            if o + 16 > len(data):
                raise XbeError("library version table runs past end of file")
            name = data[o:o + 8].split(b"\0", 1)[0].decode("ascii", "replace")
            major, minor, build, flags = struct.unpack_from("<4H", data, o + 8)
            info.libraries.append(Library(name, f"{major}.{minor}.{build}", flags))

    exe = [s for s in info.sections if "executable" in s.flags]
    info.totals = {
        "code_virtual": sum(s.virtual_size for s in exe),
        "data_virtual": sum(s.virtual_size for s in info.sections if "executable" not in s.flags),
        "raw_total": sum(s.raw_size for s in info.sections),
        "preload_virtual": sum(s.virtual_size for s in info.sections if "preload" in s.flags),
        # Bytes with no file backing (BSS-like): virtual beyond raw.
        "zero_fill": sum(max(0, s.virtual_size - s.raw_size) for s in info.sections),
    }
    return info


def _mib(n: int) -> str:
    return f"{n / (1 << 20):.2f} MiB"


def render(info: XbeInfo, budget: int) -> str:
    lines = [
        f"XBE: {info.path}",
        f"  title: {info.title_name!r} id=0x{info.title_id:08x}" if info.title_id is not None
        else "  title: (no certificate)",
        f"  build: {info.build_kind or 'unknown key'}  base=0x{info.base_address:08x}"
        + (f"  entry=0x{info.entry_point:08x}" if info.entry_point is not None else ""),
        f"  file size:     {info.file_size:>10} B  ({_mib(info.file_size)})  budget < {budget} B: "
        + ("OK" if info.file_size < budget else "OVER"),
        f"  size of image: {info.size_of_image:>10} B  ({_mib(info.size_of_image)})  memory at load",
        f"  code (exec):   {info.totals['code_virtual']:>10} B  ({_mib(info.totals['code_virtual'])})",
        f"  data (other):  {info.totals['data_virtual']:>10} B  ({_mib(info.totals['data_virtual'])})",
        f"  zero-fill:     {info.totals['zero_fill']:>10} B",
        f"  stack commit {info.stack_commit}  heap reserve {info.heap_reserve}  heap commit {info.heap_commit}",
        "  sections:",
    ]
    for s in info.sections:
        lines.append(f"    {s.name:<12} va=0x{s.virtual_address:08x} vsize={s.virtual_size:>9} "
                     f"raw={s.raw_size:>9} [{','.join(s.flags)}]")
    if info.libraries:
        lines.append("  libraries: " + ", ".join(f"{l.name} {l.version}" for l in info.libraries))
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("xbe", type=Path)
    ap.add_argument("--json", action="store_true", help="emit JSON instead of text")
    ap.add_argument("--budget-bytes", type=int, default=DEFAULT_BUDGET_BYTES)
    args = ap.parse_args(argv)
    try:
        info = parse(args.xbe.read_bytes(), str(args.xbe))
    except (OSError, XbeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    over = info.file_size >= args.budget_bytes
    if args.json:
        out = asdict(info)
        out["budget_bytes"] = args.budget_bytes
        out["within_budget"] = not over
        print(json.dumps(out, indent=2))
    else:
        print(render(info, args.budget_bytes))
    return 1 if over else 0


if __name__ == "__main__":
    sys.exit(main())
