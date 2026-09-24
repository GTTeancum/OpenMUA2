#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Offline OpenMUA2 workspace, build, and backup commands. Python 3.11+.

No downloads, installers, global Git configuration, implicit clean, or game writes.
This workspace began from the LOCAL01 recovered source package; current validation
state is tracked in docs/CURRENT-STATUS.md.
"""
from __future__ import annotations
import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import platform
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import uuid
import zipfile

ROOT = Path(__file__).resolve().parents[1]
GAME_ID = 'RMSE52'
BASELINE = 'CURRENT: reconstructed native DOL+REL source; see docs/CURRENT-STATUS.md'
REL_AUDIT_PATH = 'docs/recovery/live-rel-audit.json'
REL_PATH = 'files/Marvel-rev-fin-plf2.rel'
MSVC_MODULE_OD_CHUNKS = {
    GAME_ID: ['chunk_0201_text1_80326900.c'],
}
PRIVATE_EXT = {'.wbfs', '.wbf1', '.wbf2', '.wbf3', '.iso', '.gcm', '.rvz', '.wia',
               '.dol', '.rel', '.sav', '.raw', '.gci', '.pem', '.key', '.pfx'}
CODE_DIRS = {'project', 'tools', 'tests', 'docs', 'configs', 'cmake', 'patches',
             'locks', 'recovery', 'evidence', '.vscode', '.github'}
ROOT_FILES = {'README.md', 'AGENTS.md', 'LICENSE', 'THIRD-PARTY-NOTICES.md',
              '.gitignore', '.gitattributes', '.editorconfig', 'FILE-MANIFEST.json',
              'OpenMUA2.cmd', 'Setup.cmd', 'Build.cmd', 'Run.cmd', 'Snapshot.cmd',
              'Backup.cmd', 'openmua2.json', 'OpenMUA2.code-workspace'}
BANNER = ('CURRENT RECONSTRUCTED SOURCE: native DOL+REL integration is present; '
          'validation remains bounded. See docs/CURRENT-STATUS.md.')


def sha256(path: Path) -> str:
    with path.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def stamp() -> str:
    return datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')


def safe_rel(value: str) -> Path:
    if not isinstance(value, str) or not value or '\\' in value or '\0' in value or ':' in value:
        raise ValueError(f'Invalid relative path: {value!r}')
    p = PurePosixPath(value)
    if p.is_absolute() or '..' in p.parts or str(p) != value:
        raise ValueError(f'Invalid relative path: {value!r}')
    return Path(*p.parts)


def within(root: Path, rel: str) -> Path:
    path = root / safe_rel(rel)
    # Reject symlinks, including parent directory links, even if they point inward.
    probe = root
    for part in safe_rel(rel).parts:
        probe = probe / part
        if probe.is_symlink():
            raise ValueError(f'Symlink/reparse redirection is not supported: {probe}')
        if hasattr(probe, 'is_junction') and probe.is_junction():
            raise ValueError(f'Junction redirection is not supported: {probe}')
    if not path.resolve().is_relative_to(root.resolve()):
        raise ValueError(f'Path escapes workspace: {path}')
    return path


def write_json(path: Path, obj: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + '.' + uuid.uuid4().hex + '.tmp')
    try:
        with temp.open('x', encoding='utf-8', newline='\n') as out:
            json.dump(obj, out, indent=2, ensure_ascii=True)
            out.write('\n')
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def manifest_rows(root: Path, name: str = 'FILE-MANIFEST.json') -> list[dict]:
    doc = json.loads(within(root, name).read_text(encoding='utf-8'))
    if doc.get('schema') != 1 or not isinstance(doc.get('files'), list):
        raise ValueError('Unsupported file manifest')
    rows = doc['files']
    seen: set[str] = set()
    for row in rows:
        safe_rel(row['path'])
        key = row['path'].casefold()
        if key in seen or not re.fullmatch(r'[0-9a-f]{64}', row.get('sha256', '')):
            raise ValueError('Duplicate path or invalid hash in manifest')
        if not isinstance(row.get('size'), int) or row['size'] < 0:
            raise ValueError('Invalid manifest size')
        seen.add(key)
    return rows


def verify_manifest(root: Path, name: str = 'FILE-MANIFEST.json') -> int:
    rows = manifest_rows(root, name)
    failed = []
    for row in rows:
        p = within(root, row['path'])
        if not p.is_file() or p.stat().st_size != row['size'] or sha256(p) != row['sha256']:
            failed.append(row['path'])
    if failed:
        raise ValueError(f'{len(failed)} files differ from {name}: ' + ', '.join(failed[:12]) +
                         '. Edits may be intentional; this check never overwrites them.')
    print(f'Integrity PASS: {len(rows):,} files ({name}).', flush=True)
    return len(rows)


def invoke(args: list[str | Path], cwd: Path, *, capture: bool = False,
           input_bytes: bytes | None = None, check: bool = True,
           env: dict[str, str] | None = None) -> subprocess.CompletedProcess:
    return subprocess.run([str(x) for x in args], cwd=cwd, input=input_bytes,
                          stdout=subprocess.PIPE if capture else None,
                          stderr=subprocess.PIPE if capture else None,
                          env=env, check=check)


def git(root: Path, *args: str, capture: bool = True, check: bool = True,
        input_bytes: bytes | None = None) -> subprocess.CompletedProcess:
    if not shutil.which('git'):
        raise ValueError('Git is not on PATH. No installer will be run.')
    return invoke(['git', '-c', 'core.quotepath=false', *args], root,
                  capture=capture, input_bytes=input_bytes, check=check)


def git_text(root: Path, *args: str) -> str:
    return git(root, *args).stdout.decode('utf-8', 'strict').strip()


def assert_own_git(root: Path) -> None:
    if not (root / '.git').exists():
        raise ValueError('No local Git repository. Run Setup.cmd first.')
    top = Path(git_text(root, 'rev-parse', '--show-toplevel')).resolve()
    if top != root.resolve():
        raise ValueError('Git resolved a different repository; refusing to modify it.')


def new_git_config(root: Path) -> None:
    git(root, 'config', '--local', 'core.autocrlf', 'false')
    git(root, 'config', '--local', 'core.longpaths', 'true')
    if os.name == 'nt':
        git(root, 'config', '--local', 'core.filemode', 'false')


def commit(root: Path, message: str) -> None:
    opts: list[str] = []
    for key, fallback in [('user.name', 'OpenMUA2 Local Snapshot'),
                          ('user.email', 'local-snapshot@openmua2.invalid')]:
        if git(root, 'config', '--get', key, check=False).returncode != 0:
            opts += ['-c', key + '=' + fallback]
    git(root, *opts, 'commit', '-m', message, capture=False)


def setup(root: Path) -> None:
    print(BANNER, flush=True)
    backup = root / '.backup-manifest.json'
    verify_manifest(root, '.backup-manifest.json' if backup.is_file() else 'FILE-MANIFEST.json')
    marker = root / '.git/openmua2-initializing.json'
    resume = False
    if (root / '.git').exists():
        assert_own_git(root)
        has_head = git(root, 'rev-parse', '--verify', 'HEAD', check=False).returncode == 0
        if not backup.is_file() and marker.is_file() and not has_head:
            record = json.loads(marker.read_text(encoding='utf-8'))
            if record.get('manifest_sha256') != sha256(root / 'FILE-MANIFEST.json'):
                raise ValueError('Interrupted setup marker belongs to a different manifest; review manually.')
            allowed = {r['path'] for r in manifest_rows(root)} | {'FILE-MANIFEST.json'}
            indexed = {n.decode('utf-8') for n in git(root, 'ls-files', '-z').stdout.split(b'\0') if n}
            if indexed - allowed:
                raise ValueError('Interrupted setup has unrelated staged files; refusing to commit them.')
            print("Resuming this package's interrupted initial source commit.")
            resume = True
        else:
            print('Existing repository retained. No staging, commit, config change or reset was performed.')
            return
    if backup.is_file():
        info = json.loads((root / '.backup-info.json').read_text())
        if not re.fullmatch('[0-9a-f]{40,64}', info.get('head', '')):
            raise ValueError('Invalid backup HEAD')
        bundle = within(root, '.recovery-history.bundle')
        git(root, 'init', '-b', 'restored-local', capture=False)
        new_git_config(root)
        git(root, 'bundle', 'verify', str(bundle), capture=False)
        git(root, 'fetch', '--no-tags', str(bundle),
            'refs/heads/*:refs/heads/*', 'refs/tags/*:refs/tags/*', capture=False)
        branch = info.get('branch', '')
        if branch and git(root, 'check-ref-format', '--branch', branch, check=False).returncode == 0:
            git(root, 'symbolic-ref', 'HEAD', 'refs/heads/' + branch)
        # Mixed reset restores the index only, NEVER checks out over the saved worktree.
        git(root, 'reset', '--mixed', info['head'], capture=False)
        print('Offline Git history restored. Saved worktree edits/deletions remain uncommitted.')
        return
    if not resume:
        git(root, 'init', '-b', 'main', capture=False)
        new_git_config(root)
        write_json(marker, {'schema': 1, 'manifest_sha256': sha256(root / 'FILE-MANIFEST.json')})
    names = [r['path'] for r in manifest_rows(root)] + ['FILE-MANIFEST.json']
    payload = b'\0'.join(n.encode('utf-8') for n in names) + b'\0'
    # Exact literal paths, not a glob/pathspec scan over tens of thousands of paths.
    # update-index avoids quadratic pathspec matching; never "git add ." over a WBFS.
    git(root, 'update-index', '--add', '--remove', '-z', '--stdin', input_bytes=payload)
    commit(root, 'OpenMUA2 LOCAL01 recoverable source baseline (MG01 + FPC01)')
    marker.unlink(missing_ok=True)
    print('Local repository and initial commit created. No remote was added; no files were uploaded.')


def discover_image(root: Path, explicit: str | None = None) -> Path:
    if explicit:
        p = Path(explicit)
        if not p.is_absolute():
            p = root / p
        if p.is_symlink() or p.resolve().parent != root.resolve() or p.suffix.lower() != '.wbfs':
            raise ValueError('The image must be an ordinary .wbfs file at the repository root.')
        if not p.is_file():
            raise ValueError('WBFS not found: ' + str(p))
        return p.resolve()
    found = sorted(p for p in root.iterdir() if p.is_file() and p.suffix.lower() == '.wbfs')
    if len(found) != 1:
        raise ValueError(f'Expected one .wbfs at the repository root; found {len(found)}. '
                         'Use --image "exact filename.wbfs" to select between several.')
    return discover_image(root, found[0].name)


def image_record(path: Path) -> list[dict]:
    with path.open('rb') as src:
        if src.read(4) != b'WBFS':
            raise ValueError('The selected file does not have a WBFS header.')
    parts = [path]
    # Wiimms split files are .wbf1, .wbf2, ... alongside the primary .wbfs.
    extras = {int(p.suffix[4:]): p for p in path.parent.iterdir()
              if p.is_file() and p.stem.casefold() == path.stem.casefold()
              and re.fullmatch(r'\.wbf[1-9][0-9]*', p.suffix.lower())}
    if extras and sorted(extras) != list(range(1, max(extras) + 1)):
        raise ValueError('Non-contiguous .wbfN companions')
    parts.extend(extras[i] for i in sorted(extras))
    for p in parts:
        if p.is_symlink():
            raise ValueError('WBFS companion must not be a symlink')
    print('Hashing original WBFS input (read-only)...', flush=True)
    return [{'name': p.name, 'size': p.stat().st_size, 'sha256': sha256(p)} for p in parts]


def is_code_path(name: str, delivered: set[str]) -> bool:
    safe_rel(name)
    if name in delivered:
        return True
    parts = PurePosixPath(name).parts
    p = PurePosixPath(name)
    if p.suffix.lower() in PRIVATE_EXT or re.search(r'\.wbf\d+$', name, re.I):
        return False
    if any(part.lower().startswith('.env') or part.lower() in
           {'.local', '.backups', '.git', '__pycache__', 'node_modules', 'game', 'extracted', 'generated'}
           for part in parts):
        return False
    return name in ROOT_FILES or (len(parts) > 1 and parts[0] in CODE_DIRS)


def working_code(root: Path) -> tuple[list[str], list[str]]:
    assert_own_git(root)
    delivered = {r['path'] for r in manifest_rows(root)} | {'FILE-MANIFEST.json'}
    tracked = [x.decode('utf-8') for x in git(root, 'ls-files', '-z').stdout.split(b'\0') if x]
    forbidden = [n for n in tracked if not is_code_path(n, delivered)]
    if forbidden:
        raise ValueError('Private/unrecognized files are already tracked; review the index: ' + ', '.join(forbidden[:10]))
    others = [x.decode('utf-8') for x in git(root, 'ls-files', '--others', '--exclude-standard', '-z').stdout.split(b'\0') if x]
    allowed = [n for n in others if is_code_path(n, delivered)]
    omitted = [n for n in others if n not in allowed]
    return sorted(set(tracked + allowed)), omitted


def snapshot(root: Path, message: str | None = None) -> None:
    names, omitted = working_code(root)
    if omitted:
        print('Not staged (outside code allowlist): ' + ', '.join(omitted[:10]))
    payload = b'\0'.join(n.encode() for n in names) + b'\0'
    git(root, 'update-index', '--add', '--remove', '-z', '--stdin', input_bytes=payload)
    check = git(root, 'diff', '--cached', '--quiet', check=False)
    if check.returncode == 0:
        print('No staged source changes. Existing commits retained.')
        return
    if check.returncode != 1:
        raise ValueError('Cannot inspect Git index')
    commit(root, message or 'Local OpenMUA2 source checkpoint ' + stamp())
    print('Source snapshot committed locally. Game image, generated files and saves remain excluded.')


def backup(root: Path, destination: str | None = None) -> Path:
    names, omitted = working_code(root)
    head = git_text(root, 'rev-parse', 'HEAD')
    branch_p = git(root, 'symbolic-ref', '--short', 'HEAD', check=False)
    branch = branch_p.stdout.decode().strip() if branch_p.returncode == 0 else ''
    status_before = git(root, 'status', '--porcelain=v1', '-z').stdout
    if (root / '.git/index.lock').exists():
        raise ValueError('Git index is locked. Finish the other Git operation before backing up.')
    out = Path(destination) if destination else within(root, '.backups') / ('OpenMUA2-source-' + stamp() + '.zip')
    if not out.is_absolute():
        out = root / out
    out = out.resolve()
    if out.exists():
        raise ValueError('Backup destination already exists; choose a new filename.')
    out.parent.mkdir(parents=True, exist_ok=True)
    local = within(root, '.local'); local.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='backup-', dir=local) as temp:
        temp = Path(temp)
        bundle = temp / 'history.bundle'
        git(root, 'bundle', 'create', str(bundle), '--all', capture=False)
        git(root, 'bundle', 'verify', str(bundle), capture=False)
        info = {'schema': 1, 'created_utc': stamp(), 'head': head, 'branch': branch,
                'includes_game_or_saves': False, 'omitted_untracked': omitted,
                'note': 'Full local source history plus current source worktree; not a game-data backup.'}
        rows = []
        staging = out.with_name(out.name + '.' + uuid.uuid4().hex + '.partial')
        try:
            with zipfile.ZipFile(staging, 'x', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
                for name in names:
                    p = within(root, name)
                    if not p.exists():
                        continue  # A deleted tracked file stays deleted after restore.
                    if not p.is_file():
                        raise ValueError('Expected ordinary source file: ' + name)
                    data = p.read_bytes()
                    z.writestr(name, data)
                    rows.append({'path': name, 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
                info_data = (json.dumps(info, indent=2) + '\n').encode()
                for name, data in [('.backup-info.json', info_data), ('.recovery-history.bundle', bundle.read_bytes())]:
                    z.writestr(name, data)
                    rows.append({'path': name, 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
                z.writestr('.backup-manifest.json', json.dumps({'schema': 1, 'files': rows}, indent=2) + '\n')
            if git_text(root, 'rev-parse', 'HEAD') != head or git(root, 'status', '--porcelain=v1', '-z').stdout != status_before:
                raise ValueError('Git changed during backup. No final backup was published; retry with editing paused.')
            # Hash every backed-up source again, catching edits that do not change porcelain status.
            for row in rows:
                if row['path'].startswith(('.backup-', '.recovery-history')):
                    continue
                if sha256(within(root, row['path'])) != row['sha256']:
                    raise ValueError('A source file changed during backup; retry with editing paused.')
            with zipfile.ZipFile(staging) as z:
                if z.testzip() is not None:
                    raise ValueError('Backup CRC verification failed')
            os.rename(staging, out)
        finally:
            staging.unlink(missing_ok=True)
    print('Verified source/history backup: ' + str(out))
    print('Restore into an empty folder and run Setup.cmd. Keep the WBFS and .local/user separately.')
    return out


def logged(root: Path, label: str, args: list[str | Path], env: dict[str, str] | None = None) -> None:
    logdir = within(root, '.local/logs'); logdir.mkdir(parents=True, exist_ok=True)
    token = stamp() + '-' + label
    logfile = logdir / (token + '.log')
    argv = [str(x) for x in args]
    print('\n' + subprocess.list2cmdline(argv), flush=True)
    with logfile.open('x', encoding='utf-8', newline='\n') as log:
        with subprocess.Popen(argv, cwd=root, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              text=True, encoding='utf-8', errors='replace') as child:
            try:
                assert child.stdout is not None
                for line in child.stdout:
                    print(line, end='', flush=True); log.write(line); log.flush()
                rc = child.wait()
            except KeyboardInterrupt:
                child.terminate()
                try: child.wait(timeout=10)
                except subprocess.TimeoutExpired: child.kill(); child.wait()
                raise
    write_json(logdir / (token + '.json'), {'argv': argv, 'returncode': rc, 'log': logfile.name,
                                           'baseline': BASELINE, 'gameplay_verified': False})
    if rc:
        raise RuntimeError(f'{label} failed ({rc}); log: {logfile}')


def executable(path: str) -> str:
    found = shutil.which(path)
    if found:
        return found
    p = Path(path)
    if p.is_file():
        return str(p.resolve())
    raise ValueError('Required tool not found: ' + path + '. No installation will be attempted.')


def doctor(root: Path, options: argparse.Namespace) -> None:
    print(BANNER)
    if struct.calcsize('P') != 8 or platform.machine().lower() not in ('amd64', 'x86_64'):
        raise ValueError('The supplied local workflow targets a 64-bit x86 host/Python.')
    for name in ('cmake', 'ctest', 'ninja', 'git'):
        print(name + ': ' + executable(name))
    for name in (options.cc, options.cxx, options.module_cc):
        print('compiler: ' + executable(name))
    needed = ('project/lib/DolRecomp/CMakeLists.txt', 'project/lib/ModernGekko/CMakeLists.txt',
              'project/lib/ModernGekko/vendor/dolphin/module-template/CMakeLists.txt')
    for name in needed:
        if not within(root, name).is_file():
            raise ValueError('Missing source: ' + name)
    print('Host/source preflight passed. This is not a Windows build or gameplay test.')


def build_base(root: Path) -> Path:
    tag = 'windows-x64' if os.name == 'nt' else 'linux-x64'
    p = within(root, '.local/build/' + tag); p.mkdir(parents=True, exist_ok=True)
    return p


def cmake_configure(root: Path, source: Path, out: Path, options: argparse.Namespace,
                    extra: list[str] | None = None, module: bool = False) -> None:
    cc = executable(options.module_cc if module else options.cc)
    args: list[str | Path] = ['cmake', '-S', source, '-B', out, '-G', 'Ninja',
                             '-DCMAKE_BUILD_TYPE=' + options.config,
                             '-DCMAKE_C_COMPILER=' + cc,
                             '-DCMAKE_OBJECT_PATH_MAX=180',
                             '-DFETCHCONTENT_FULLY_DISCONNECTED=ON',
                             '-DFETCHCONTENT_UPDATES_DISCONNECTED=ON']
    if not module:
        args.append('-DCMAKE_CXX_COMPILER=' + executable(options.cxx))
    args += extra or []
    # CMake rejects incompatible cached generator/compiler changes; never deletes a build.
    env = os.environ.copy(); env['CMAKE_NINJA_FORCE_RESPONSE_FILE'] = '1'
    logged(root, out.name + '-configure', args, env)


def cmake_build(root: Path, out: Path, options: argparse.Namespace, retries: int = 0) -> None:
    attempt = 0
    while True:
        try:
            logged(root, out.name + '-build', ['cmake', '--build', out, '--config', options.config,
                                             '--parallel', str(options.jobs)])
            return
        except RuntimeError:
            attempt += 1
            if attempt > retries:
                raise
            print(f'{out.name} build failed; retrying {attempt}/{retries} from Ninja state.', flush=True)


def built_exe(directory: Path, name: str, config: str = 'Release') -> Path:
    suffix = '.exe' if os.name == 'nt' else ''
    choices = [directory / (name + suffix), directory / config / (name + suffix)]
    found = [p for p in choices if p.is_file()]
    if len(found) != 1:
        raise ValueError(f'Expected one {name}{suffix} in {directory}; found {len(found)}.')
    return found[0].resolve()


def build_recompiler(root: Path, options: argparse.Namespace) -> Path:
    out = build_base(root) / 'dolrecomp'
    cmake_configure(root, root / 'project/lib/DolRecomp', out, options,
                    ['-DDOLRECOMP_ENABLE_LLVM=OFF', '-DBUILD_TESTING=ON'])
    cmake_build(root, out, options)
    logged(root, 'dolrecomp-tests', ['ctest', '--test-dir', out, '-C', options.config, '--output-on-failure'])
    return built_exe(out, 'dolrecomp', options.config)


def game_audit(root: Path, game: Path) -> dict:
    sys.path.insert(0, str(root / 'tools'))
    import audit_game
    result = audit_game.audit(game)
    if result['disc_id'] != GAME_ID:
        raise ValueError('Wrong disc ID')
    return result


def extract_game(root: Path, options: argparse.Namespace, recompiler: Path) -> Path:
    image = discover_image(root, options.image)
    records = image_record(image)
    dest = within(root, '.local/game')
    receipt = within(root, '.local/receipts/extraction.json')
    if dest.exists():
        if not receipt.is_file() or json.loads(receipt.read_text()).get('source_parts') != records:
            raise ValueError('Existing extracted game has no matching source receipt. '
                             'It was retained; move it aside explicitly before replacing it.')
        game_audit(root, dest)
        print('Existing extracted game validated; nothing re-extracted.')
        return dest
    wit = executable(options.wit)
    stage = within(root, '.local/game-staging-' + uuid.uuid4().hex)
    logged(root, 'disc-extract', [recompiler, 'extract', '--wit', wit, image, stage])
    audit = game_audit(root, stage)
    # Ensure the image was not edited during extraction.
    if image_record(image) != records:
        raise ValueError('WBFS changed during extraction; staged output retained for inspection.')
    if dest.exists():
        raise ValueError('Game destination appeared during extraction; refusing to overwrite it.')
    stage.rename(dest)
    write_json(receipt, {'schema': 1, 'source_parts': records, 'audit': audit,
                         'original_image_was_written': False})
    print('Game extracted and DOL/REL hashes validated. Original WBFS was not changed.')
    return dest


def generate(root: Path, options: argparse.Namespace, recompiler: Path, game: Path) -> Path:
    dol = game / 'sys/main.dol'
    dispatch_lookup = getattr(options, 'dispatch_lookup', 'indexed')
    key = hashlib.sha256()
    for path in (recompiler, dol, within(root, 'tools/generate_dol_mg01.py')):
        key.update(sha256(path).encode('ascii'))
        key.update(b'\0')
    key.update(('dispatch=' + dispatch_lookup).encode('ascii'))
    out = within(root, '.local/generated-mg01-' + key.hexdigest()[:12])
    # The guarded historical generator refuses to overwrite edits or mismatched receipts.
    generate_env = dict(os.environ)
    generate_env['DOLRECOMP_DISPATCH_LOOKUP'] = dispatch_lookup
    logged(root, 'generate-dol', [sys.executable, root / 'tools/generate_dol_mg01.py',
                                '--recompiler', recompiler, '--dol', dol,
                                '--output', out, '--jobs', str(options.jobs)],
           env=generate_env)
    return out / 'generated'


def rel_layout_bases(root: Path) -> tuple[int, int]:
    audit = json.loads(within(root, REL_AUDIT_PATH).read_text(encoding='utf-8'))
    rel_base = int(audit['observed_rel_base'])
    bss = [section for section in audit['layout'] if section.get('bss') and int(section['size'])]
    if len(bss) != 1:
        raise ValueError('Expected exactly one audited non-empty REL BSS section.')
    return rel_base, int(bss[0]['address'])


def generate_native_rel(root: Path, options: argparse.Namespace, recompiler: Path,
                        game: Path, dol_generated: Path) -> Path:
    rel = game / REL_PATH
    audit_path = within(root, REL_AUDIT_PATH)
    merger = within(root, 'tools/merge_mua2_generated.py')
    rel_base, bss_base = rel_layout_bases(root)
    dispatch_lookup = getattr(options, 'dispatch_lookup', 'indexed')
    key = hashlib.sha256()
    for path in (recompiler, rel, audit_path, merger):
        key.update(sha256(path).encode('ascii'))
        key.update(b'\0')
    key.update(f'{rel_base:08X}:{bss_base:08X}'.encode('ascii'))
    key.update(('dispatch=' + dispatch_lookup).encode('ascii'))
    token = key.hexdigest()[:12]
    rel_root = within(root, f'.local/generated-rel-mg01-{token}')
    combined = within(root, f'.local/generated-combined-mg01-rel-{token}')
    receipt = within(root, f'.local/receipts/generated-native-rel-{token}.json')
    expected = {
        'schema': 1,
        'rel_base': rel_base,
        'rel_bss_base': bss_base,
        'recompiler_sha256': sha256(recompiler),
        'rel_sha256': sha256(rel),
        'rel_audit_sha256': sha256(audit_path),
        'merger_sha256': sha256(merger),
        'dispatch_lookup': dispatch_lookup,
    }
    if receipt.is_file():
        saved = json.loads(receipt.read_text(encoding='utf-8'))
        if all(saved.get(name) == value for name, value in expected.items()):
            if (combined / 'generated.h').is_file() and (combined / 'rel_metadata.json').is_file():
                print('Reusing generated DOL+REL C from matching native REL receipt.')
                return combined
    if combined.exists() and any(combined.iterdir()):
        raise ValueError('Existing native REL generated output has no matching receipt: ' + str(combined))
    if rel_root.exists() and any(rel_root.iterdir()):
        raise ValueError('Existing REL generated output has no matching receipt: ' + str(rel_root))
    generate_env = dict(os.environ)
    generate_env['DOLRECOMP_DISPATCH_LOOKUP'] = dispatch_lookup
    logged(root, 'generate-rel',
           [recompiler, '--rel-base', f'0x{rel_base:08X}', '--rel-bss-base', f'0x{bss_base:08X}',
            '--cpu', 'broadway', '--backend', 'c', rel, GAME_ID, rel_root],
           env=generate_env)
    logged(root, 'merge-dol-rel-generated',
           [sys.executable, merger, '--dol-generated', dol_generated, '--rel-generated', rel_root / 'generated',
            '--rel-audit', audit_path, '--rel-file', rel, '--main-dol', game / 'sys/main.dol',
            '--output', combined])
    expected['created_utc'] = stamp()
    expected['rel_root'] = rel_root.relative_to(root).as_posix()
    expected['combined'] = combined.relative_to(root).as_posix()
    write_json(receipt, expected)
    return combined


def build_runtime(root: Path, options: argparse.Namespace) -> Path:
    out = build_base(root) / 'runtime'
    extra = ['-DBUILD_TESTING=ON', '-DDOLRECOMP_ENABLE_LLVM=OFF',
             '-DMODERNGEKKO_ENABLE_DOLPHIN_RUNTIME=ON', '-DUSE_SYSTEM_LIBS=OFF']
    if options.sdk:
        if os.name == 'nt':
            raise ValueError('--sdk is a Linux-only optional private SDK, not a Windows toolchain.')
        extra.append('-DMODERNGEKKO_FRONTEND_SDK=' + str(Path(options.sdk).resolve()))
    cmake_configure(root, root / 'project/lib/ModernGekko', out, options, extra)
    cmake_build(root, out, options)
    stage_runtime_sys_resources(root, out)
    logged(root, 'runtime-tests', ['ctest', '--test-dir', out, '-C', options.config,
                                  '--output-on-failure', '--timeout', '90', '-R', '^moderngekko[.]'])
    return built_exe(out, 'moderngekko-run', options.config)


def module_build_name(options: argparse.Namespace) -> str:
    suffix = getattr(options, 'module_suffix', None)
    rel = '-rel' if getattr(options, 'native_rel', False) else ''
    if suffix:
        if not re.fullmatch(r'[A-Za-z0-9_.-]+', suffix):
            raise ValueError('--module-suffix may only contain letters, numbers, dot, underscore, and dash')
        return 'module-mg01' + rel + '-' + suffix
    if getattr(options, 'module_ipo', False):
        return f'module-mg01{rel}-o{options.module_opt}-ipo'
    if options.module_opt:
        return f'module-mg01{rel}-o{options.module_opt}'
    return 'module-mg01' + rel


def stage_runtime_sys_resources(root: Path, runtime_build: Path) -> None:
    target = runtime_build / 'Sys' / 'GC'
    target.mkdir(parents=True, exist_ok=True)
    source_roots = [
        root / '.local/runtime-resources/Sys/GC',
        root / 'project/lib/ModernGekko/vendor/dolphin/Data/Sys/GC',
    ]
    staged: dict[str, str] = {}
    missing: list[str] = []
    for name in ('font_western.bin', 'font_japanese.bin'):
        destination = target / name
        source = next((candidate / name for candidate in source_roots
                       if (candidate / name).is_file()), None)
        if source:
            if not destination.is_file() or sha256(destination) != sha256(source):
                shutil.copy2(source, destination)
            staged[name] = str(source.relative_to(root))
        elif destination.is_file():
            staged[name] = str(destination.relative_to(root))
        else:
            missing.append(name)
    if missing:
        raise RuntimeError(
            'Missing Dolphin Sys/GC font resources: ' + ', '.join(missing) +
            '. Place locally sourced copies under .local/runtime-resources/Sys/GC; '
            'the build will stage them automatically from there.')
    write_json(root / '.local/receipts/runtime-sys-resources.json',
               {'schema': 1, 'created_utc': stamp(), 'target': str(target.relative_to(root)),
                'resources': staged})


def build_module(root: Path, options: argparse.Namespace, generated: Path, game: Path) -> Path:
    core = root / 'project/lib/ModernGekko/vendor/dolphin'
    out = build_base(root) / module_build_name(options)
    cmake_configure(root, core / 'module-template', out, options,
                    ['-DGAME_ID=RMSE52', '-DGENERATED_DIR=' + str(generated),
                     '-DRECOMPCORE_MODULE_ENABLE_IPO=' + ('ON' if options.module_ipo else 'OFF'),
                     '-DRECOMPCORE_MODULE_OPT_LEVEL=' + str(options.module_opt),
                     '-DRECOMPCORE_MODULE_MSVC_OD_CHUNKS=' +
                     ';'.join(MSVC_MODULE_OD_CHUNKS.get(GAME_ID, []))], module=True)
    cmake_build(root, out, options, retries=getattr(options, 'module_build_retries', 0))
    module = out / ('gRMSE52_recomp.dll' if os.name == 'nt' else 'gRMSE52_recomp.so')
    if not module.is_file():
        raise ValueError('Expected native module was not produced: ' + str(module))
    audit_dir = build_base(root) / 'native-audit'
    cmake_configure(root, root / 'tools/native-audit', audit_dir, options,
                    ['-DRECOMPCORE_SOURCE=' + str(core)], module=True)
    cmake_build(root, audit_dir, options)
    audit_exe = built_exe(audit_dir, 'openmua2-verify-module', options.config)
    audit_args = [audit_exe, module, game / 'sys/main.dol']
    if (generated / 'rel_metadata.json').is_file():
        audit_args.append(generated)
    logged(root, 'module-abi-hashes', audit_args)
    return module


def build(root: Path, options: argparse.Namespace) -> None:
    doctor(root, options)
    recompiler = build_recompiler(root, options)
    game = extract_game(root, options, recompiler)
    generated = generate(root, options, recompiler, game)
    if options.native_rel:
        generated = generate_native_rel(root, options, recompiler, game, generated)
    runner = build_runtime(root, options)
    module = build_module(root, options, generated, game)
    receipt = {'schema': 1, 'baseline': BASELINE, 'created_utc': stamp(),
               'platform': platform.platform(), 'game': game.relative_to(root).as_posix(),
               'runner': runner.relative_to(root).as_posix(), 'runner_sha256': sha256(runner),
               'module': module.relative_to(root).as_posix(), 'module_sha256': sha256(module),
               'gameplay_verified': False, 'native_rel_integrated': bool(options.native_rel),
               'module_opt': options.module_opt, 'dispatch_lookup': options.dispatch_lookup}
    write_json(within(root, '.local/receipts/build.json'), receipt)
    print('\nOpenMUA2 diagnostic build completed. No gameplay test was performed.\n' + BANNER)


def run_game(root: Path, options: argparse.Namespace) -> None:
    print(BANNER)
    record_path = within(root, '.local/receipts/build.json')
    if not record_path.is_file():
        raise ValueError('No successful local build receipt. Run Build.cmd first.')
    record = json.loads(record_path.read_text())
    runner, module = within(root, record['runner']), within(root, record['module'])
    for p, key in [(runner, 'runner_sha256'), (module, 'module_sha256')]:
        if not p.is_file() or sha256(p) != record[key]:
            raise ValueError('Build output changed after verification; rebuild/verify before running: ' + str(p))
    game = within(root, record['game']); game_audit(root, game)
    user = within(root, '.local/user'); user.mkdir(parents=True, exist_ok=True)
    args = [runner, '--game', game, '--module', module, '--user-dir', user]
    if options.graphics:
        args += ['--graphics', options.graphics]
    if options.audio:
        args += ['--audio', options.audio]
    # Existing user controller settings are intentionally not overwritten.
    logged(root, 'runtime-session', args)


def parse_status_file(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    try:
        for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
            key, separator, value = line.partition('=')
            if separator:
                result[key] = value
    except FileNotFoundError:
        pass
    return result


def benchmark(root: Path, options: argparse.Namespace) -> None:
    print(BANNER)
    record_path = within(root, '.local/receipts/build.json')
    if not record_path.is_file():
        raise ValueError('No successful local build receipt. Run Build.cmd first.')
    record = json.loads(record_path.read_text())
    runner = within(root, record['runner'])
    module = within(root, record['module'])
    for p, key in [(runner, 'runner_sha256'), (module, 'module_sha256')]:
        if not p.is_file() or sha256(p) != record[key]:
            raise ValueError('Build output changed after verification; rebuild/verify before benchmarking: ' + str(p))
    game = within(root, record['game'])
    game_audit(root, game)
    user = within(root, options.user_dir or '.local/user')
    user.mkdir(parents=True, exist_ok=True)
    token = stamp() + '-benchmark'
    automation = within(root, '.local/automation/' + token)
    for child in ('commands', 'processed', 'failed'):
        (automation / child).mkdir(parents=True, exist_ok=True)
    logfile = within(root, '.local/logs/' + token + '.log')
    args = [str(runner), '--game', str(game), '--module', str(module), '--user-dir', str(user),
            '--automation-dir', str(automation)]
    if options.graphics:
        args += ['--graphics', options.graphics]
    if options.audio:
        args += ['--audio', options.audio]
    if options.headless:
        args += ['--headless']
    print('\n' + subprocess.list2cmdline(args), flush=True)
    samples: list[dict[str, object]] = []
    started = time.monotonic()
    ready_at: float | None = None
    deadline = started + options.timeout
    with logfile.open('x', encoding='utf-8', newline='\n') as log:
        child = subprocess.Popen(args, cwd=root, stdout=log, stderr=subprocess.STDOUT,
                                 text=True, encoding='utf-8', errors='replace')
        try:
            while time.monotonic() < deadline:
                if child.poll() is not None:
                    raise RuntimeError(f'benchmark runtime exited early with code {child.returncode}; log: {logfile}')
                status = parse_status_file(automation / 'status.txt')
                now = time.monotonic()
                if status.get('booted') == '1' and status.get('state') == 'running':
                    if ready_at is None:
                        ready_at = now
                    if now - ready_at >= options.warmup:
                        sample: dict[str, object] = {'elapsed': now - ready_at}
                        for key in ('fps', 'vps', 'speed', 'frame_count', 'present_count'):
                            if key in status:
                                try:
                                    sample[key] = float(status[key])
                                except ValueError:
                                    sample[key] = status[key]
                        samples.append(sample)
                        if now - ready_at >= options.warmup + options.seconds:
                            break
                time.sleep(options.interval)
            else:
                raise RuntimeError('benchmark timed out waiting for enough runtime samples')
        finally:
            stop_file = automation / 'commands' / '999-stop.txt'
            stop_file.write_text('command=stop\n', encoding='utf-8', newline='\n')
            try:
                child.wait(timeout=15)
            except subprocess.TimeoutExpired:
                child.terminate()
                try:
                    child.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    child.kill()
                    child.wait()
    fps_values = [float(sample['fps']) for sample in samples if isinstance(sample.get('fps'), float)]
    speed_values = [float(sample['speed']) for sample in samples if isinstance(sample.get('speed'), float)]
    frame_values = [float(sample['frame_count']) for sample in samples
                    if isinstance(sample.get('frame_count'), float)]
    duration = samples[-1]['elapsed'] - samples[0]['elapsed'] if len(samples) >= 2 else 0.0
    guest_fps = ((frame_values[-1] - frame_values[0]) / duration
                 if len(frame_values) >= 2 and duration > 0 else None)
    result = {
        'schema': 1,
        'created_utc': stamp(),
        'baseline': BASELINE,
        'runner': record['runner'],
        'module': record['module'],
        'module_sha256': record['module_sha256'],
        'headless': options.headless,
        'graphics': options.graphics,
        'audio': options.audio,
        'warmup_seconds': options.warmup,
        'sample_seconds': options.seconds,
        'sample_count': len(samples),
        'fps_avg': sum(fps_values) / len(fps_values) if fps_values else None,
        'fps_min': min(fps_values) if fps_values else None,
        'fps_max': max(fps_values) if fps_values else None,
        'speed_avg': sum(speed_values) / len(speed_values) if speed_values else None,
        'guest_frame_fps': guest_fps,
        'samples': samples,
        'log': str(logfile.relative_to(root)),
        'automation_dir': str(automation.relative_to(root)),
        'returncode': child.returncode,
    }
    out = within(root, '.local/benchmarks/' + token + '.json')
    write_json(out, result)
    print(json.dumps({k: result[k] for k in ('fps_avg', 'fps_min', 'fps_max', 'speed_avg',
                                             'guest_frame_fps', 'sample_count', 'log')},
                     indent=2))
    print('Benchmark written to ' + str(out))


def controller_diagnostics(root: Path, options: argparse.Namespace) -> None:
    print(BANNER)
    record_path = within(root, '.local/receipts/build.json')
    if not record_path.is_file():
        raise ValueError('No successful local build receipt. Run Build.cmd first.')
    record = json.loads(record_path.read_text())
    runner = within(root, record['runner'])
    if not runner.is_file() or sha256(runner) != record['runner_sha256']:
        raise ValueError('Build output changed after verification; rebuild/verify before controller diagnostics: ' +
                         str(runner))
    args = [runner, '--controller-diagnostics',
            '--controller-diagnostics-seconds', str(options.seconds)]
    if options.rumble:
        args.append('--controller-diagnostics-rumble')
    logged(root, 'controller-diagnostics', args)


def make_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest='action', required=True)
    for action in ('setup', 'verify', 'status', 'test'):
        sub.add_parser(action)
    s = sub.add_parser('snapshot'); s.add_argument('-m', '--message')
    s = sub.add_parser('backup'); s.add_argument('--destination')
    for action in ('doctor', 'build', 'build-tools', 'extract', 'generate'):
        s = sub.add_parser(action)
        s.add_argument('--jobs', type=int, default=2)
        s.add_argument('--config', choices=('Debug', 'Release', 'RelWithDebInfo'), default='Release')
        s.add_argument('--cc', default=os.environ.get('CC', 'cl' if os.name == 'nt' else 'gcc'))
        s.add_argument('--cxx', default=os.environ.get('CXX', 'cl' if os.name == 'nt' else 'g++'))
        s.add_argument('--module-cc', default='cl' if os.name == 'nt' else ('clang' if shutil.which('clang') else 'gcc'))
        s.add_argument('--module-opt', type=int, choices=(0, 1, 2, 3), default=2)
        s.add_argument('--dispatch-lookup', choices=('indexed', 'linear'), default='indexed',
                       help='Generated native dispatch lookup; indexed is the performance default.')
        s.add_argument('--module-ipo', action='store_true')
        s.add_argument('--module-suffix')
        s.add_argument('--module-build-retries', type=int, default=0)
        s.add_argument('--native-rel', action='store_true',
                       help='Generate and package the audited RMSE52 gameplay REL as native code.')
        s.add_argument('--wit', default=os.environ.get('WIT', 'wit'))
        s.add_argument('--image', help='Exact .wbfs filename at repository root (optional if unique)')
        s.add_argument('--sdk', help='Optional Linux-only private SDK root')
    s = sub.add_parser('run'); s.add_argument('--graphics'); s.add_argument('--audio')
    s = sub.add_parser('benchmark')
    s.add_argument('--seconds', type=float, default=20.0)
    s.add_argument('--warmup', type=float, default=5.0)
    s.add_argument('--interval', type=float, default=0.5)
    s.add_argument('--timeout', type=float, default=90.0)
    s.add_argument('--graphics')
    s.add_argument('--audio')
    s.add_argument('--headless', action='store_true')
    s.add_argument('--user-dir')
    s = sub.add_parser('controller-diagnostics')
    s.add_argument('--seconds', type=float, default=5.0)
    s.add_argument('--rumble', action='store_true')
    return p


def main(argv: list[str] | None = None) -> int:
    options = make_parser().parse_args(argv)
    if hasattr(options, 'jobs') and not 1 <= options.jobs <= 128:
        raise ValueError('--jobs must be 1..128')
    action = options.action
    if action == 'setup': setup(ROOT)
    elif action == 'verify': verify_manifest(ROOT)
    elif action == 'status':
        print(BANNER)
        print((ROOT / 'docs/CURRENT-STATUS.md').read_text(encoding='utf-8'))
    elif action == 'snapshot': snapshot(ROOT, options.message)
    elif action == 'backup': backup(ROOT, options.destination)
    elif action == 'test':
        logged(ROOT, 'local-workspace-tests', [sys.executable, '-m', 'unittest', 'discover', '-s', ROOT / 'tests', '-v'])
    elif action == 'doctor': doctor(ROOT, options)
    elif action == 'build-tools': doctor(ROOT, options); build_recompiler(ROOT, options)
    elif action in ('extract', 'generate'):
        doctor(ROOT, options)
        tool = build_recompiler(ROOT, options); game = extract_game(ROOT, options, tool)
        if action == 'generate':
            generated = generate(ROOT, options, tool, game)
            if options.native_rel:
                generate_native_rel(ROOT, options, tool, game, generated)
    elif action == 'build': build(ROOT, options)
    elif action == 'run': run_game(ROOT, options)
    elif action == 'benchmark': benchmark(ROOT, options)
    elif action == 'controller-diagnostics': controller_diagnostics(ROOT, options)
    return 0


if __name__ == '__main__':
    try:
        if sys.version_info < (3, 11):
            raise ValueError('Python 3.11 or newer is required')
        raise SystemExit(main())
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as exc:
        print('STOPPED: ' + str(exc), file=sys.stderr)
        if isinstance(exc, subprocess.CalledProcessError) and exc.stderr:
            print(exc.stderr.decode('utf-8', 'replace'), file=sys.stderr)
        raise SystemExit(1)
    except KeyboardInterrupt:
        print('Interrupted. Existing sources, game image and saves were retained.', file=sys.stderr)
        raise SystemExit(130)
