#!/usr/bin/env python3
"""Run a bounded, evidence-producing MUA2 diagnostic boot. Not a playability test.

Uses ModernGekko's existing file-based automation; does not patch game memory.
The output directory must be new/empty. DISPLAY is required unless --headless.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for b in iter(lambda: f.read(1024 * 1024), b''):
            h.update(b)
    return h.hexdigest()


def read_status(path: Path) -> dict[str, str]:
    try:
        return dict(line.split('=', 1) for line in path.read_text().splitlines() if '=' in line)
    except (OSError, ValueError):
        return {}


def write_command(root: Path, number: int, text: str) -> None:
    # A partial file in commands/ is already visible to the runtime. Stage outside it.
    target = root / 'commands' / f'{number:06d}.txt'
    target.parent.mkdir(parents=True, exist_ok=True)
    temp = root / f'.{number:06d}.pending'
    temp.write_text(text.rstrip() + '\n', encoding='utf-8')
    temp.replace(target)


def seed_probe_profile(user: Path) -> None:
    """A dedicated probe profile: stock resolution, emulated Wiimote + Nunchuk."""
    config = user / 'Config'
    config.mkdir(parents=True, exist_ok=True)
    (user / 'config.ini').write_text('[Video]\nresolution=640x528\nbackend=OGL\nshow_fps_in_title=false\n', encoding='utf-8')
    (config / 'WiimoteNew.ini').write_text(
        '[Wiimote1]\nSource = 1\nExtension = Nunchuk\n'
        'Options/Sideways Wiimote = False\nOptions/Upright Wiimote = False\n'
        '[Wiimote2]\nSource = 0\n[Wiimote3]\nSource = 0\n'
        '[Wiimote4]\nSource = 0\n[BalanceBoard]\nSource = 0\n', encoding='utf-8')
    (config / 'Logger.ini').write_text(
        '[Options]\nWriteToConsole = True\nWriteToFile = True\nVerbosity = 3\n'
        '[Logs]\nBOOT = True\nCORE = True\nCOMMON = True\nVideo = True\n'
        'Host GPU = True\nOSREPORT = True\nOSREPORT_HLE = True\nPowerPC = True\n', encoding='utf-8')



def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('runtime', 'module', 'game', 'output'):
        p.add_argument('--' + name, type=Path, required=True)
    p.add_argument('--seconds', type=float, default=60)
    p.add_argument('--headless', action='store_true')
    p.add_argument('--backend', choices=('OGL', 'Vulkan'), default='OGL')
    p.add_argument('--press-start', action='store_true', help='Send brief Plus and A presses, then release.')
    p.add_argument('--capture-memory', action='store_true', help='Read MEM1/MEM2 for REL diagnosis; never writes guest memory.')
    a = p.parse_args()
    if not 5 <= a.seconds <= 1800:
        p.error('--seconds must be between 5 and 1800')
    a.runtime = a.runtime.resolve(); a.module = a.module.resolve()
    a.game = a.game.resolve(); a.output = a.output.resolve()
    for path in (a.runtime, a.module, a.game/'sys/main.dol'):
        if not path.is_file():
            p.error(f'missing input: {path}')
    if a.output.exists() and any(a.output.iterdir()):
        p.error('output directory must be empty (evidence is never overwritten)')
    if not a.headless and not os.environ.get('DISPLAY'):
        p.error('graphical test needs DISPLAY; use xvfb-run or select --headless')
    a.output.mkdir(parents=True, exist_ok=True)
    user = a.output/'user'; control = a.output/'automation'
    seed_probe_profile(user); control.mkdir()
    env = os.environ.copy()
    env['STATICRECOMP_TRACE_FILE'] = str(a.output/'native-trace.csv')
    env['STATICRECOMP_DISPATCH_SAMPLES'] = '1'
    env['LIBGL_ALWAYS_SOFTWARE'] = '1'
    command = [str(a.runtime), '--game', str(a.game), '--module', str(a.module),
               '--user-dir', str(user), '--title', 'MUA2 MG01 diagnostic',
               '--automation-dir', str(control), '--graphics', 'Null' if a.headless else a.backend,
               '--audio', 'No Audio Output', '--mute', '--no-mods']
    command += ['--headless'] if a.headless else ['--x11']
    receipt = {'command': command, 'game_dol_sha256': digest(a.game/'sys/main.dol'),
               'module_sha256': digest(a.module), 'runtime_sha256': digest(a.runtime),
               'backend': 'Null' if a.headless else a.backend, 'headless': a.headless, 'software_rendering_requested': not a.headless,
               'maximum_seconds': a.seconds, 'press_start_requested': a.press_start,
               'native_rel_claimed': False, 'playability_claimed': False}
    (a.output/'invocation.json').write_text(json.dumps(receipt, indent=2)+'\n')
    started = time.monotonic(); index = 0; sent: set[str] = set(); states = []
    forced = False; interrupted = False; ready_started = None
    with (a.output/'runtime.log').open('w') as log:
        process = subprocess.Popen(command, stdin=subprocess.DEVNULL, stdout=log,
                                   stderr=subprocess.STDOUT, env=env, start_new_session=True)
        try:
            while process.poll() is None and time.monotonic()-started < a.seconds:
                elapsed = time.monotonic()-started
                status = read_status(control/'status.txt')
                if status and (not states or elapsed-states[-1]['elapsed_seconds'] >= 1):
                    states.append({'elapsed_seconds': round(elapsed, 3), **status})
                ready = status.get('state') in ('running', 'paused')
                if ready and ready_started is None:
                    ready_started = elapsed
                active_elapsed = elapsed-ready_started if ready_started is not None else 0
                actions: list[tuple[str, str]] = []
                if ready and not a.headless:
                    for sec in (5, 20, 40, 75, 110):
                        if active_elapsed >= sec:
                            actions.append((f'screen-{sec}', f'command=screenshot\npath=screen-{sec}.png'))
                if ready and a.press_start:
                    for sec, name, text in (
                        (10, 'plus', 'command=pad\nport=0\nwii_plus=1'),
                        (10.3, 'release-plus', 'command=clear_pad\nport=0'),
                        (15, 'a', 'command=pad\nport=0\nwii_a=1'),
                        (15.3, 'release-a', 'command=clear_pad\nport=0')):
                        if active_elapsed >= sec:
                            actions.append((name, text))
                if ready and a.capture_memory and elapsed >= max(5, a.seconds-10):
                    for base, length in ((0x80000000, 0x1800000), (0x90000000, 0x4000000)):
                        for offset in range(0, length, 0x1000000):
                            address = base+offset; size = min(0x1000000, length-offset)
                            actions.append((f'memory-{address:08X}',
                                f'command=read_memory\naddress=0x{address:08X}\nsize={size}\npath=memory-{address:08X}.bin'))
                for name, text in actions:
                    if name not in sent:
                        index += 1; write_command(control, index, text); sent.add(name)
                time.sleep(.1)
        except KeyboardInterrupt:
            interrupted = True
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=15)
                except subprocess.TimeoutExpired:
                    forced = True; os.killpg(process.pid, signal.SIGKILL); process.wait(timeout=5)
    text = (a.output/'runtime.log').read_text(errors='replace')
    shutdown = re.findall(r'\[staticrecomp\] shutdown:([^\n]+)', text)
    counters = {k: int(v) for k, v in re.findall(r'(\w+)=(\d+)', shutdown[-1])} if shutdown else {}
    images = []
    for path in sorted(control.glob('*.png')):
        b = path.read_bytes()
        images.append({'file': str(path.relative_to(a.output)), 'bytes': len(b), 'sha256': digest(path),
                       'valid_png_signature': b.startswith(b'\x89PNG\r\n\x1a\n')})
    result = {**receipt, 'process_exit_code': process.returncode,
              'elapsed_seconds': round(time.monotonic()-started, 3), 'forced_kill': forced,
              'interrupted': interrupted, 'shutdown_counters': counters,
              'runtime_reported_video_failure': 'Failed to initialize video backend' in text,
              'native_dispatch_observed': counters.get('native', 0) > 0,
              'screenshots': images, 'last_status': read_status(control/'status.txt'),
              'processed_commands': len(list((control/'processed').glob('*'))),
              'failed_commands': len(list((control/'failed').glob('*'))),
              'counter_note': 'Native and fallback counters are not comparable instruction counts; no percentage is inferred.',
              'visual_result': 'Not assessed by this script. Inspect actual screenshots.'}
    (a.output/'status-history.json').write_text(json.dumps(states, indent=2)+'\n')
    (a.output/'boot-result.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result, indent=2))
    return 0 if process.returncode == 0 and not forced and counters.get('native', 0)>0 and 'Failed to initialize video backend' not in text else 1


if __name__ == '__main__':
    raise SystemExit(main())
