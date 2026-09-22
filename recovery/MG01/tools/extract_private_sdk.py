#!/usr/bin/env python3
"""Authenticate collector packages, then unpack privately; never install host packages.

Requires a Debian host's existing trusted archive keyring, Python 3.12+, gpgv,
and dpkg-deb. --sdk must be empty or owned by this helper's matching state file.
Interrupted extractions can resume. Do not use /usr, /, or a personal home as SDK.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--payload', type=Path, required=True)
    p.add_argument('--sdk', type=Path, required=True)
    a = p.parse_args(); payload = a.payload.resolve(); sdk = a.sdk.resolve()
    if sdk in (Path('/'), Path('/usr'), Path('/opt'), Path('/mnt/data'), Path.home()):
        p.error('SDK must be a dedicated new subdirectory')
    helper = payload/'helpers'
    if not (helper/'install_linux_deps.py').is_file():
        p.error('missing the verified collector package-audit helper')
    sys.path.insert(0, str(helper))
    from install_linux_deps import verify
    from collector_core import write_json
    # Verifies Debian InRelease signatures using the PRE-EXISTING system keyring.
    # Does not call that helper's optional --install branch.
    paths = verify(payload)
    metadata_hash = hashlib.sha256((payload/'DEBIAN_PACKAGES.json').read_bytes()).hexdigest()
    marker = sdk/'.mua2-private-sdk.json'
    state = {'metadata_sha256': metadata_hash, 'completed_packages': [], 'complete': False}
    if sdk.exists() and any(sdk.iterdir()):
        if not marker.is_file():
            p.error('nonempty SDK is not owned by this helper; refusing to overwrite it')
        state = json.loads(marker.read_text())
        if state.get('metadata_sha256') != metadata_hash:
            p.error('SDK belongs to a different snapshot; choose a new directory')
    sdk.mkdir(parents=True, exist_ok=True)
    completed = set(state['completed_packages'])
    write_json(marker, state)
    for i, path in enumerate(paths, 1):
        name = Path(path).name
        if name not in completed:
            print(f'Extract {i}/{len(paths)}: {name}', flush=True)
            subprocess.run(['dpkg-deb', '-x', path, str(sdk)], check=True)
            completed.add(name)
            state['completed_packages'] = sorted(completed)
            write_json(marker, state)
    state['complete'] = True; write_json(marker, state)
    print(f'Private SDK ready: {sdk}. No host packages were installed, upgraded, or removed.')


if __name__ == '__main__':
    main()
