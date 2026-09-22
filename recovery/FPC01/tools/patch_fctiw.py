#!/usr/bin/env python3
"""Narrow, guarded fctiw/fctiwz flag correction. No whole-file overlay or build.

Pass --workspace to update the two DolRecomp copies in a restored project, or
--file to update one cpu.c. Defaults to a read-only check; --apply is explicit.
Only the exact pinned function or the exact corrected function is accepted.
Unrelated source edits and the original newline convention are preserved.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import stat
import tempfile

ORIGINAL_FUNCTION_SHA256 = '1c370c3431430be5e8023869379ec4de4966a66f5e4f17650dba7a15a6540d5e'
FIXED_FUNCTION_SHA256 = 'e5e2f983011abf146a8c50a7636fb7415a80b363aa1f32f27720d687ca7da245'
START = b'bool ppc_fctiw(CPUState* cpu, f64 value, bool toward_zero, u64* output) {'
END = b'\nvoid ppc_fcmp('


def analyze(path: Path) -> tuple[bytes, dict]:
    if path.is_symlink() or not path.is_file():
        raise ValueError(f'Expected an ordinary source file: {path}')
    raw = path.read_bytes()
    if b'\r' in raw.replace(b'\r\n', b''):
        raise ValueError(f'Unsupported mixed/CR newline format: {path}')
    crlf = b'\r\n' in raw
    if crlf and b'\n' in raw.replace(b'\r\n', b''):
        raise ValueError(f'Mixed newline format: {path}')
    text = raw.replace(b'\r\n', b'\n')
    if text.count(START) != 1:
        raise ValueError(f'Missing or ambiguous ppc_fctiw definition: {path}')
    start = text.index(START)
    end = text.find(END, start)
    if end < 0:
        raise ValueError(f'Missing following function boundary: {path}')
    body = text[start:end]
    digest = hashlib.sha256(body).hexdigest()
    if digest not in (ORIGINAL_FUNCTION_SHA256, FIXED_FUNCTION_SHA256):
        raise ValueError(f'ppc_fctiw has other edits; refusing to overwrite: {path} ({digest})')
    fixed = body
    if digest == ORIGINAL_FUNCTION_SHA256:
        for old, new in ((b'~0x00006000u', b'~0x00060000u'),
                         (b'|= 0x00004000u', b'|= 0x00020000u'),
                         (b'|= 0x00002000u', b'|= 0x00040000u')):
            if fixed.count(old) != 1:
                raise ValueError(f'Unexpected replacement count: {path}')
            fixed = fixed.replace(old, new)
        if hashlib.sha256(fixed).hexdigest() != FIXED_FUNCTION_SHA256:
            raise ValueError('Internal corrected-function digest mismatch')
    output = text[:start] + fixed + text[end:]
    if crlf:
        output = output.replace(b'\n', b'\r\n')
    return output, {
        'file': str(path), 'state': 'already_fixed' if output == raw else 'needs_fix',
        'input_sha256': hashlib.sha256(raw).hexdigest(),
        'output_sha256': hashlib.sha256(output).hexdigest(),
        'function_input_sha256': digest, 'function_output_sha256': FIXED_FUNCTION_SHA256,
    }


def patch_files(paths: list[Path], apply: bool = False) -> list[dict]:
    # Validate every destination before writing either project copy.
    paths = [p.absolute() for p in paths]
    if len(set(paths)) != len(paths):
        raise ValueError('Duplicate destination')
    pending = [(p, *analyze(p)) for p in paths]
    staged: list[tuple[Path, Path]] = []
    try:
        if apply:
            for path, data, info in pending:
                if info['state'] == 'already_fixed':
                    continue
                # Each file is replaced atomically. An I/O failure across multiple
                # files is not a transaction; rerun the idempotent check afterwards.
                fd, name = tempfile.mkstemp(prefix=path.name + '.fctiw-', dir=path.parent)
                temp = Path(name)
                staged.append((path, temp))
                with os.fdopen(fd, 'wb') as stream:
                    stream.write(data)
                    stream.flush()
                    os.fsync(stream.fileno())
                temp.chmod(stat.S_IMODE(path.stat().st_mode))
            for path, temp in staged:
                os.replace(temp, path)
        return [{**info, 'applied': bool(apply and info['state'] == 'needs_fix')}
                for _, _, info in pending]
    finally:
        for _, temp in staged:
            temp.unlink(missing_ok=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--workspace', type=Path, help='directory containing project/')
    group.add_argument('--file', type=Path, action='append', help='specific cpu.c (repeatable)')
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    if args.workspace:
        root = args.workspace / 'project'
        paths = [root / 'lib/DolRecomp/src/cpu/cpu.c',
                 root / 'lib/ModernGekko/vendor/dolphin/DolRecomp/src/cpu/cpu.c']
    else:
        paths = args.file
    try:
        print(json.dumps({'dry_run': not args.apply,
                          'files': patch_files(paths, args.apply)}, indent=2))
    except (OSError, ValueError) as exc:
        parser.exit(2, f'Not applied successfully: {exc}\n')


if __name__ == '__main__':
    main()
