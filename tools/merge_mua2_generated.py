#!/usr/bin/env python3
"""Merge the pinned RMSE52 DOL and audited REL DolRecomp C outputs.

This creates a generated directory suitable for the RecompCore module template:
one generated.h dispatcher, one generated.c manifest, one chunks/ folder, the
source main.dol, plus REL metadata used by gen_module_tables.py.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import struct
from pathlib import Path


PROTO_RE = re.compile(r"void func_([0-9A-Fa-f]{8})\(CPUState\* ctx\);")
DISPATCH_PAGE_SHIFT = 12
DISPATCH_PAGE_SIZE = 1 << DISPATCH_PAGE_SHIFT
DISPATCH_PAGE_MASK = DISPATCH_PAGE_SIZE - 1
DISPATCH_MAX_INDEX_PAGES = 65536
RANGE_RE = re.compile(
    r"u32\s+offset\s*=\s*address\s*-\s*(0x[0-9A-Fa-f]+)u\s*;\s*"
    r"if\s*\(\s*offset\s*<\s*(0x[0-9A-Fa-f]+)u"
)


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_ranges(header: str) -> list[tuple[int, int]]:
    ranges = {
        (int(a, 16), int(b, 16))
        for a, b in re.findall(
            r"address >= (0x[0-9A-Fa-f]+)u && address < (0x[0-9A-Fa-f]+)u",
            header,
        )
    }
    for base, span in RANGE_RE.findall(header):
        start = int(base, 16)
        ranges.add((start, start + int(span, 16)))
    if not ranges:
        raise ValueError("no generated code ranges found")
    return sorted(ranges)


def parse_functions(header: str) -> list[int]:
    funcs = sorted({int(match, 16) for match in PROTO_RE.findall(header)})
    if not funcs:
        raise ValueError("no generated function prototypes found")
    return funcs


def chunk_ranges(header: str) -> list[tuple[int, int]]:
    ranges = parse_ranges(header)
    funcs = parse_functions(header)
    chunks: list[tuple[int, int]] = []
    for index, address in enumerate(funcs):
        containing = next((r for r in ranges if r[0] <= address < r[1]), None)
        if containing is None:
            raise ValueError(f"func_{address:08X} is outside all code ranges")
        end = containing[1]
        if index + 1 < len(funcs) and containing[0] <= funcs[index + 1] < containing[1]:
            end = funcs[index + 1]
        chunks.append((address, end))
    return chunks


def split_header(header: str) -> tuple[str, str, str]:
    marker = "// Function entry points\n"
    prefix_marker = "#define DOLRECOMP_ENTRY_POINT"
    find_marker = "static inline DolRecompFunction dolrecomp_find_original"
    footer_marker = "static inline int dolrecomp_call_original"
    if (
        marker not in header
        or prefix_marker not in header
        or find_marker not in header
        or footer_marker not in header
    ):
        raise ValueError("unexpected DolRecomp generated.h layout")
    prelude = header[: header.index(marker) + len(marker)]
    prefix = header[header.index(prefix_marker) : header.index(find_marker)]
    footer = header[header.index(footer_marker) :]
    return prelude, prefix, footer


def copy_chunks(src: Path, dst: Path) -> list[str]:
    names: list[str] = []
    for chunk in sorted((src / "chunks").glob("*.c")):
        target = dst / "chunks" / chunk.name
        if target.exists():
            raise ValueError(f"duplicate chunk filename: {chunk.name}")
        shutil.copy2(chunk, target)
        names.append(chunk.name)
    if not names:
        raise ValueError(f"no chunks found in {src / 'chunks'}")
    return names


def build_dispatch_page_index(
    chunks: list[tuple[int, int]],
) -> tuple[int, list[int], list[int]] | None:
    if not chunks:
        return None
    base = chunks[0][0] & ~DISPATCH_PAGE_MASK
    limit = (chunks[-1][1] + DISPATCH_PAGE_MASK) & ~DISPATCH_PAGE_MASK
    page_count = (limit - base) >> DISPATCH_PAGE_SHIFT
    if page_count <= 0 or page_count > DISPATCH_MAX_INDEX_PAGES:
        return None

    page_first: list[int] = []
    page_end: list[int] = []
    first = 0
    for page in range(page_count):
        page_start = base + page * DISPATCH_PAGE_SIZE
        page_limit = page_start + DISPATCH_PAGE_SIZE
        while first < len(chunks) and chunks[first][1] <= page_start:
            first += 1
        end = first
        while end < len(chunks) and chunks[end][0] < page_limit:
            end += 1
        page_first.append(first)
        page_end.append(end)
    return base, page_first, page_end


def emit_header(dol_header: str, rel_header: str, output: Path) -> None:
    prelude, prefix, footer = split_header(dol_header)
    chunks = sorted(chunk_ranges(dol_header) + chunk_ranges(rel_header))
    for index, (start, end) in enumerate(chunks):
        if start >= end:
            raise ValueError(f"invalid generated code chunk 0x{start:08X}-0x{end:08X}")
        if (start & 3) != 0 or (end & 3) != 0:
            raise ValueError(f"unaligned generated code chunk 0x{start:08X}-0x{end:08X}")
        if index and start < chunks[index - 1][1]:
            raise ValueError("overlapping generated code chunks")
    prototypes = "\n".join(f"void func_{start:08X}(CPUState* ctx);" for start, _ in chunks)
    page_index = build_dispatch_page_index(chunks)
    table = [
        "typedef struct DolRecompDispatchEntry {",
        "    u32 start;",
        "    u32 end;",
        "    DolRecompFunction fn;",
        "} DolRecompDispatchEntry;",
        "",
    ]

    if page_index is not None:
        page_base, page_first, page_end = page_index
        table.extend(
            [
                f"#define DOLRECOMP_MERGED_PAGE_BASE 0x{page_base:08X}u",
                f"#define DOLRECOMP_MERGED_PAGE_SHIFT {DISPATCH_PAGE_SHIFT}u",
                f"#define DOLRECOMP_MERGED_PAGE_COUNT {len(page_first)}u",
                "",
            ]
        )

    table.extend(
        [
            "static inline DolRecompFunction dolrecomp_find_original(u32 address) {",
            "    static const DolRecompDispatchEntry dolrecomp_merged_chunks[] = {",
        ]
    )
    for start, end in chunks:
        table.append(f"        {{0x{start:08X}u, 0x{end:08X}u, func_{start:08X}}},")
    table.append("    };")

    if page_index is not None:
        table.append(
            "    static const u32 dolrecomp_merged_page_first[DOLRECOMP_MERGED_PAGE_COUNT] = {"
        )
        table.extend(f"        {value}u," for value in page_first)
        table.append("    };")
        table.append(
            "    static const u32 dolrecomp_merged_page_end[DOLRECOMP_MERGED_PAGE_COUNT] = {"
        )
        table.extend(f"        {value}u," for value in page_end)
        table.extend(
            [
                "    };",
                "    if ((address & 3u) != 0u) return NULL;",
                "    if (address < DOLRECOMP_MERGED_PAGE_BASE) return NULL;",
                "    u32 page = (address - DOLRECOMP_MERGED_PAGE_BASE) >> DOLRECOMP_MERGED_PAGE_SHIFT;",
                "    if (page >= DOLRECOMP_MERGED_PAGE_COUNT) return NULL;",
                "    u32 lo = dolrecomp_merged_page_first[page];",
                "    u32 hi = dolrecomp_merged_page_end[page];",
                "    if (hi == lo) return NULL;",
                "    if (hi == lo + 1u) {",
                "        const DolRecompDispatchEntry* chunk = &dolrecomp_merged_chunks[lo];",
                "        return address >= chunk->start && address < chunk->end ? chunk->fn : NULL;",
                "    }",
            ]
        )
    else:
        table.extend(
            [
                "    if ((address & 3u) != 0u) return NULL;",
                "    u32 lo = 0;",
                "    u32 hi = (u32)(sizeof(dolrecomp_merged_chunks) / sizeof(dolrecomp_merged_chunks[0]));",
            ]
        )

    table.extend(
        [
            "    while (lo < hi) {",
            "        u32 mid = lo + (hi - lo) / 2u;",
            "        if (address < dolrecomp_merged_chunks[mid].start) {",
            "            hi = mid;",
            "        } else if (address >= dolrecomp_merged_chunks[mid].end) {",
            "            lo = mid + 1u;",
            "        } else {",
            "            return dolrecomp_merged_chunks[mid].fn;",
            "        }",
            "    }",
            "    return NULL;",
            "}",
            "",
        ]
    )
    output.write_text(
        prelude + prototypes + "\n\n" + prefix + "\n".join(table) + footer,
        encoding="utf-8",
    )


def emit_manifest(chunk_names: list[str], output: Path) -> None:
    lines = [
        "// DolRecomp split output",
        '#include "generated.h"',
        "",
        "// Build these C files too:",
    ]
    lines.extend(f"// chunks/{name}" for name in chunk_names)
    lines.extend(["", f"// {len(chunk_names)} C files", ""])
    output.write_text("\n".join(lines), encoding="utf-8")


def be32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def checked(data: bytes, offset: int, size: int) -> bytes:
    if offset < 0 or size < 0 or offset > len(data) or size > len(data) - offset:
        raise ValueError(f"out-of-file range 0x{offset:X}+0x{size:X}")
    return data[offset : offset + size]


def patch(section: bytearray, offset: int, kind: int, target: int, patch_address: int) -> None:
    if kind in (3, 4, 5, 6):
        if offset + 2 > len(section):
            raise ValueError("REL halfword patch outside section")
        if kind in (3, 4):
            value = target
        elif kind == 5:
            value = target >> 16
        else:
            value = (target + 0x8000) >> 16
        struct.pack_into(">H", section, offset, value & 0xFFFF)
        return
    if offset + 4 > len(section):
        raise ValueError("REL word patch outside section")
    if kind == 1:
        struct.pack_into(">I", section, offset, target)
    elif kind == 10:
        delta = target - patch_address
        if delta % 4 or not -0x2000000 <= delta < 0x2000000:
            raise ValueError("REL24 target out of range")
        word = be32(section, offset)
        struct.pack_into(">I", section, offset, (word & 0xFC000003) | (delta & 0x03FFFFFC))
    else:
        raise ValueError(f"unsupported REL relocation type {kind}")


def replay_rel_text(rel_data: bytes, layout: list[dict[str, object]], module_id: int) -> dict[int, bytes]:
    sections = {
        int(section["index"]): bytearray(
            checked(rel_data, int(section["file_offset"]), int(section["size"]))
        )
        for section in layout
        if int(section["file_offset"]) != 0 and int(section["size"]) != 0
    }
    import_offset = be32(rel_data, 0x28)
    import_size = be32(rel_data, 0x2C)
    for cursor in range(import_offset, import_offset + import_size, 8):
        import_module = be32(rel_data, cursor)
        stream = be32(rel_data, cursor + 4)
        current_section: int | None = None
        current_offset = 0
        for pos in range(stream, len(rel_data) - 7, 8):
            delta, kind, target_section, addend = struct.unpack_from(">HBBI", rel_data, pos)
            if kind == 203:
                break
            if kind == 202:
                current_section = target_section
                current_offset = 0
                continue
            current_offset += delta
            if kind in (0, 201):
                continue
            if current_section is None or current_section not in sections:
                raise ValueError("REL relocation writes to absent section")
            if import_module == 0:
                target = addend
            elif import_module == module_id:
                target = int(layout[target_section]["address"]) + addend
            else:
                raise ValueError(f"unsupported external REL import module {import_module}")
            patch(
                sections[current_section],
                current_offset,
                kind,
                target,
                int(layout[current_section]["address"]) + current_offset,
            )
        else:
            raise ValueError("REL relocation stream has no END marker")
    return {
        int(section["index"]): bytes(sections[int(section["index"])])
        for section in layout
        if bool(section["executable"]) and int(section["index"]) in sections
    }


def read_audited_rel(audit: dict[str, object], rel_path: Path) -> bytes:
    if audit.get("status") != "LIVE_TEXT_MATCH":
        raise ValueError("REL audit does not report LIVE_TEXT_MATCH")
    expected_hash = audit.get("source_rel_sha256")
    if not isinstance(expected_hash, str) or re.fullmatch(
        r"[0-9A-Fa-f]{64}", expected_hash
    ) is None:
        raise ValueError("REL audit has no valid source_rel_sha256")
    rel_data = rel_path.read_bytes()
    actual_hash = hashlib.sha256(rel_data).hexdigest()
    if actual_hash.lower() != expected_hash.lower():
        raise ValueError("REL file SHA-256 does not match audited source")
    return rel_data


def verify_replayed_rel_text(rel_text: dict[str, object], text_bytes: bytes) -> None:
    if rel_text.get("mismatch_bytes") != 0:
        raise ValueError("REL audit text section is not an exact live match")
    expected_hash = rel_text.get("expected_sha256")
    observed_hash = rel_text.get("observed_sha256")
    for label, value in (
        ("expected_sha256", expected_hash),
        ("observed_sha256", observed_hash),
    ):
        if not isinstance(value, str) or re.fullmatch(r"[0-9A-Fa-f]{64}", value) is None:
            raise ValueError(f"REL audit text section has no valid {label}")
    if expected_hash.lower() != observed_hash.lower():
        raise ValueError("REL audit expected and observed text hashes differ")
    actual_hash = hashlib.sha256(text_bytes).hexdigest()
    if actual_hash != observed_hash.lower():
        raise ValueError("replayed REL text SHA-256 does not match audited live text")


def emit_rel_metadata(audit_path: Path, rel_path: Path, output: Path) -> None:
    audit = json.loads(audit_path.read_text(encoding="utf-8"))
    rel_data = read_audited_rel(audit, rel_path)
    comparison = next(
        item for item in audit["comparisons"] if item["halfword_write_adjust"] == 0
    )
    if comparison.get("mismatch_bytes") != 0:
        raise ValueError("REL audit zero-adjust comparison is not an exact live match")
    text_sections = [row for row in comparison["text_sections"] if row["bytes"]]
    if len(text_sections) != 1:
        raise ValueError("expected exactly one executable REL text section")
    rel_text = text_sections[0]
    layout = audit["layout"]
    replayed = replay_rel_text(rel_data, layout, audit["module_id"])
    text_bytes = replayed[rel_text["section"]]
    if len(text_bytes) != rel_text["bytes"]:
        raise ValueError("replayed REL text size mismatch")
    verify_replayed_rel_text(rel_text, text_bytes)
    metadata = {
        "schema": 1,
        "modules": [
            {
                "module_id": audit["module_id"],
                "version": 3,
                "section_count": len(layout),
                "section_info_offset": 0x4C,
                "file_size": 4381544,
                "sections": [
                    {
                        "module_id": audit["module_id"],
                        "section_index": rel_text["section"],
                        "linked_start": rel_text["address"],
                        "size": rel_text["bytes"],
                        "bytes": "rel_text_section_1.bin",
                    }
                ],
            }
        ],
    }
    output.joinpath("rel_metadata.json").write_text(
        json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
    )
    output.joinpath("rel_text_section_1.bin").write_bytes(text_bytes)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dol-generated", type=Path, required=True)
    parser.add_argument("--rel-generated", type=Path, required=True)
    parser.add_argument("--rel-audit", type=Path, required=True)
    parser.add_argument("--rel-file", type=Path, required=True)
    parser.add_argument("--main-dol", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if args.output.exists() and any(args.output.iterdir()):
        raise SystemExit(f"refusing to write non-empty output: {args.output}")
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "chunks").mkdir()

    dol_header = read_text(args.dol_generated / "generated.h")
    rel_header = read_text(args.rel_generated / "generated.h")
    chunk_names = copy_chunks(args.dol_generated, args.output)
    chunk_names.extend(copy_chunks(args.rel_generated, args.output))
    emit_header(dol_header, rel_header, args.output / "generated.h")
    emit_manifest(sorted(chunk_names), args.output / "generated.c")
    shutil.copy2(args.main_dol, args.output / "main.dol")

    smc_parts = []
    for source in (args.dol_generated, args.rel_generated):
        smc = source / "generated_smc.txt"
        if smc.exists():
            smc_parts.extend(line for line in smc.read_text(encoding="utf-8").splitlines() if line)
    (args.output / "generated_smc.txt").write_text(
        "\n".join(sorted(set(smc_parts))) + "\n", encoding="utf-8"
    )
    emit_rel_metadata(args.rel_audit, args.rel_file, args.output)
    print(f"merged {len(chunk_names)} chunks into {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())