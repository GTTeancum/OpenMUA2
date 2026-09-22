#!/usr/bin/env python3
"""Build and run native/reference fctiw tests without changing the workspace.
Requires the pinned dependency source tree, GCC/G++ 14 or compatible Clang,
CMake/Ninja only for the separate upstream-test recipe. No download or install.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
from patch_fctiw import analyze, START, END, ORIGINAL_FUNCTION_SHA256

ROOT = Path(__file__).resolve().parent.parent
SUPPORT = [
    'Source/Core/Core/PowerPC/PowerPC.cpp',
    'Source/Core/Core/PowerPC/PPCCache.cpp',
    'Source/Core/Core/PowerPC/ConditionRegister.cpp',
    'Source/Core/Common/Config/Config.cpp',
]


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(cmd: list[str], log: Path, expected: int = 0, timeout: int = 180) -> dict:
    started = time.monotonic()
    env = os.environ.copy()
    env['ASAN_OPTIONS'] = 'detect_leaks=1:abort_on_error=1'
    env['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
    with log.open('wb') as output:
        process = subprocess.run(cmd, stdout=output, stderr=subprocess.STDOUT,
                                 timeout=timeout, env=env)
    record = {'command': cmd, 'exit_code': process.returncode,
              'expected_exit_code': expected, 'seconds': round(time.monotonic()-started,3),
              'log': log.name}
    if process.returncode != expected:
        raise RuntimeError(f'Command exit {process.returncode}, expected {expected}: {log}\n'
                           + log.read_text(errors='replace')[-5000:])
    return record


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--workspace', type=Path, required=True, help='contains project/')
    parser.add_argument('--output', type=Path, required=True, help='new, empty destination')
    parser.add_argument('--cc', default='gcc')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--optimization', choices=('0','1','2'), default='2')
    parser.add_argument('--sanitize', action='store_true')
    args=parser.parse_args()
    output=args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Output directory must be empty; no previous evidence is overwritten.')
    output.mkdir(parents=True,exist_ok=True)
    workspace=args.workspace.resolve()
    dol=workspace/'project/lib/DolRecomp'
    ref=workspace/'project/lib/ModernGekko/vendor/dolphin'
    commands=[]
    receipt={'status':'INCOMPLETE', 'gameplay_tested':False, 'workspace_modified':False,
             'sanitizers': ['address','undefined (vptr excluded)'] if args.sanitize else [],
             'optimization':args.optimization, 'commands':commands}
    try:
        pins=json.loads((ROOT/'SOURCE_PINS.json').read_text())
        verified=[]
        for item in pins['reference_files']:
            file=ref/item['path']
            if digest(file)!=item['sha256']:
                raise ValueError(f'Reference source differs from pinned fixture: {file}')
            verified.append(item)
        receipt['reference_files']=verified
        cpu=dol/'src/cpu/cpu.c'
        fixed, analysis=analyze(cpu)
        # Reconstruct the exact unpatched function for the negative control;
        # all unrelated source bytes come from the supplied workspace unchanged.
        fixed=fixed.replace(b'\r\n',b'\n')
        start=fixed.index(START); end=fixed.index(END,start)
        old_body=fixed[start:end].replace(b'~0x00060000u',b'~0x00006000u') \
           .replace(b'|= 0x00020000u',b'|= 0x00004000u').replace(b'|= 0x00040000u',b'|= 0x00002000u')
        if hashlib.sha256(old_body).hexdigest()!=ORIGINAL_FUNCTION_SHA256:
            raise ValueError('Negative-control function hash mismatch')
        original=fixed[:start]+old_body+fixed[end:]
        (output/'cpu-original.c').write_bytes(original)
        (output/'cpu-fixed.c').write_bytes(fixed)
        receipt['workspace_cpu']=analysis
        receipt['compilers']={key:subprocess.check_output([tool,'--version'],text=True).splitlines()[0]
                              for key,tool in [('c',args.cc),('cxx',args.cxx)]}
        common=['-O'+args.optimization, '-g', '-ffunction-sections','-fdata-sections',
                '-frounding-math','-fno-fast-math','-ffp-contract=off',
                '-I'+str(dol/'src'),'-I'+str(dol/'src/cpu')]
        if args.sanitize:
            # Virtual dispatch is not exercised: including the whole interpreter
            # TU otherwise retains unrelated RTTI. All other UBSan checks stay on.
            common+=['-fsanitize=address,undefined','-fno-sanitize=vptr','-fno-omit-frame-pointer']
        cflags=['-std=c11']+common
        cppflags=['-std=c++23','-D_M_X86_64=1']+common+[
            '-I'+str(ref/'Source/Core'),'-I'+str(ref/'Externals/fmt/fmt/include'),
            '-I'+str(ref/'GXRuntime/include')]
        objects=[]
        for i,name in enumerate(SUPPORT):
            obj=output/f'support-{i}.o'; objects.append(str(obj))
            commands.append(run([args.cxx,*cppflags,'-c',str(ref/name),'-o',str(obj)],
                                output/f'compile-support-{i}.log'))
        receipt['tests']={}
        for name,expected in [('original',1),('fixed',0)]:
            obj=output/f'cpu-{name}.o'
            commands.append(run([args.cc,*cflags,'-c',str(output/f'cpu-{name}.c'),'-o',str(obj)],
                                output/f'compile-cpu-{name}.log'))
            exe=output/f'fctiw-{name}'
            commands.append(run([args.cxx,*cppflags,str(ROOT/'tests/fctiw_differential.cpp'),
                         str(obj),*objects,'-Wl,--gc-sections','-lm','-pthread','-o',str(exe)],
                                output/f'link-{name}.log'))
            record=run([str(exe)],output/f'test-{name}.log',expected=expected)
            commands.append(record)
            summary=json.loads((output/f'test-{name}.log').read_text().splitlines()[-1])
            if name=='original' and summary['failures']==0:
                raise ValueError('Negative control did not detect the original defect')
            if name=='fixed' and (summary['failures'] or summary['cases']!=1298576):
                raise ValueError('Fixed matrix is incomplete or failing')
            receipt['tests'][name]={**summary,'executable_sha256':digest(exe),
                                    'exit_code':record['exit_code'], 'seconds':record['seconds']}
            print(name,json.dumps(summary),flush=True)
        receipt['status']='PASS_WITH_EXPECTED_FAILING_NEGATIVE_CONTROL'
    except (OSError,ValueError,subprocess.SubprocessError,RuntimeError) as exc:
        receipt['status']='FAILED'
        receipt['error']=str(exc)
        raise
    finally:
        (output/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
    print('Receipt:',output/'receipt.json')


if __name__=='__main__':
    main()
