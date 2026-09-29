#!/usr/bin/env python3
"""Generate the pinned MUA2 DOL C backend, with a conservative resumable receipt."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
import struct
from pathlib import Path
import shutil
import subprocess


def digest(path: Path) -> str:
    h=hashlib.sha256()
    with path.open('rb') as f:
        for b in iter(lambda: f.read(1024*1024), b''): h.update(b)
    return h.hexdigest()


def expected_c_chunks(dol: Path, instructions: int) -> int:
    sizes = struct.unpack_from('>7I', dol.read_bytes(), 0x90)
    return sum((size + instructions * 4 - 1) // (instructions * 4) for size in sizes)


def main() -> None:
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('recompiler','dol','output'): p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--jobs',type=int,default=2)
    p.add_argument('--c-chunk-instructions',type=int,choices=(128,256,512,1024,2048,4096),default=4096)
    p.add_argument('--dispatch-lookup',choices=('indexed','linear'),default='indexed')
    a=p.parse_args()
    if not 1 <= a.jobs <= 128: p.error('jobs must be 1..128')
    a.output=a.output.resolve()
    wanted={'dol_sha256':digest(a.dol),'recompiler_sha256':digest(a.recompiler),
            'cpu':'broadway','backend':'c','game_id':'RMSE52',
            'c_chunk_instructions':a.c_chunk_instructions,'dispatch_lookup':a.dispatch_lookup}
    if wanted['dol_sha256']!='0857973ed7646eaf1294981295673243546c935cdbd1fd07345b4d1093c62741':
        p.error('unsupported main.dol; this checkpoint is pinned to the uploaded RMSE52 build')
    receipt=a.output/'generation-receipt.json'
    if receipt.is_file() and json.loads(receipt.read_text())==wanted:
        gen=a.output/'generated'
        if (gen/'generated.h').is_file() and (gen/'generated.c').is_file() and len(list((gen/'chunks').glob('*.c')))==expected_c_chunks(a.dol,a.c_chunk_instructions):
            print('Reusing generated DOL C from the matching input receipt.'); return
        p.error('receipt exists but generated output is incomplete; use a fresh output directory')
    if a.output.exists() and any(a.output.iterdir()):
        p.error('nonempty output has no matching receipt; refusing to replace edited generated code')
    a.output.mkdir(parents=True,exist_ok=True)
    env=dict(os.environ)
    env['DOLRECOMP_C_CHUNK_INSTRUCTIONS']=str(a.c_chunk_instructions)
    env['DOLRECOMP_DISPATCH_LOOKUP']=a.dispatch_lookup
    subprocess.run([str(a.recompiler.resolve()),'-j',str(a.jobs),'--cpu','broadway','--backend','c',
                    str(a.dol.resolve()),'RMSE52',str(a.output)],check=True,env=env)
    gen=a.output/'generated'
    if not (gen/'generated.h').is_file(): raise RuntimeError('unexpected generator output layout')
    if len(list((gen/'chunks').glob('*.c'))) != expected_c_chunks(a.dol,a.c_chunk_instructions):
        raise RuntimeError('generated DOL chunk count does not match requested size')
    shutil.copy2(a.dol,gen/'main.dol')
    receipt.write_text(json.dumps(wanted,indent=2)+'\n')


if __name__=='__main__': main()
