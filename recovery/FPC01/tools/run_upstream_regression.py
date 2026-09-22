#!/usr/bin/env python3
"""Copy a DolRecomp source tree, test before/after repair, and keep logs.
The passed source is never edited. C backend only; no game/runtime build.
Requires CMake, Ninja, a C compiler and Python 3.11 or newer. No network access.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import time
from patch_fctiw import analyze, patch_files, START, END, ORIGINAL_FUNCTION_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    source=args.source.resolve(); output=args.output.resolve()
    if output.exists(): parser.error('Output must not already exist')
    if output.is_relative_to(source): parser.error('Output must be outside the source tree')
    if not (source/'CMakeLists.txt').is_file(): parser.error('Source is not a DolRecomp tree')
    # Validate candidate before creating any build.
    fixed,_=analyze(source/'src/cpu/cpu.c')
    fixed=fixed.replace(b'\r\n',b'\n'); start=fixed.index(START); end=fixed.index(END,start)
    body=fixed[start:end].replace(b'~0x00060000u',b'~0x00006000u') \
        .replace(b'|= 0x00020000u',b'|= 0x00004000u').replace(b'|= 0x00040000u',b'|= 0x00002000u')
    if hashlib.sha256(body).hexdigest()!=ORIGINAL_FUNCTION_SHA256:
        parser.error('Cannot construct exact original control')
    output.mkdir(parents=True)
    clone=output/'source'
    shutil.copytree(source,clone,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
    # The validation source can already have the patch: force the known ORIGINAL
    # body only in the independent clone, so the control really is before-fix.
    (clone/'src/cpu/cpu.c').write_bytes(fixed[:start]+body+fixed[end:])
    records=[]
    receipt={'status':'IN_PROGRESS','gameplay_tested':False,'commands':records}
    def save(): (output/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
    def run(cmd,name):
        started=time.monotonic()
        with (output/name).open('wb') as log:
            proc=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=300)
        records.append({'command':cmd,'exit_code':proc.returncode,'log':name,
                        'seconds':round(time.monotonic()-started,3)})
        save()
        if proc.returncode: raise RuntimeError(f'{name} failed')
    try:
        build=output/'build'
        run(['cmake','-S',str(clone),'-B',str(build),'-G','Ninja',
             '-DCMAKE_BUILD_TYPE=Release','-DDOLRECOMP_ENABLE_LLVM=OFF'],'configure.log')
        run(['cmake','--build',str(build),'-j2'],'build-original.log')
        run(['ctest','--test-dir',str(build),'--output-on-failure'],'ctest-original.log')
        receipt['patch']=patch_files([clone/'src/cpu/cpu.c'],True)
        run(['cmake','--build',str(build),'-j2'],'build-fixed.log')
        run(['ctest','--test-dir',str(build),'--output-on-failure'],'ctest-fixed.log')
        receipt['status']='PASS'
    except Exception as exc:
        receipt['status']='FAILED'; receipt['error']=str(exc)
        raise
    finally: save()
    print(output/'receipt.json')

if __name__=='__main__':main()
