"""Replay process-local combat commands in an isolated OpenMUA2 profile."""
import argparse
import configparser
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


class _WindowsAffinity:
    def __init__(self):
        if os.name != "nt":
            raise OSError("Explicit logical processor selection requires Windows")
        import ctypes
        from ctypes import wintypes
        self.ctypes = ctypes
        self.api = ctypes.WinDLL("kernel32", use_last_error=True)
        self.api.GetCurrentProcess.restype = wintypes.HANDLE
        self.api.GetProcessAffinityMask.argtypes = [wintypes.HANDLE,
            ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
        self.api.GetProcessAffinityMask.restype = wintypes.BOOL
        self.api.SetProcessAffinityMask.argtypes = [wintypes.HANDLE, ctypes.c_size_t]
        self.api.SetProcessAffinityMask.restype = wintypes.BOOL
        self.handle = self.api.GetCurrentProcess()

    def get_mask(self):
        allowed, system = self.ctypes.c_size_t(), self.ctypes.c_size_t()
        if not self.api.GetProcessAffinityMask(self.handle, self.ctypes.byref(allowed),
                                               self.ctypes.byref(system)):
            raise self.ctypes.WinError(self.ctypes.get_last_error())
        return allowed.value

    def set_mask(self, mask):
        if not self.api.SetProcessAffinityMask(self.handle, mask):
            raise self.ctypes.WinError(self.ctypes.get_last_error())


def launch_on_processor(command, processor=None, *, affinity=None, **kwargs):
    """Child inherits one CPU at creation; immediately restore this harness.

    The runner independently pins/verifies all its threads at startup. No global
    policy or unrelated process changes. Restore failure terminates the child.
    """
    if processor is None:
        return subprocess.Popen(command, **kwargs)
    if type(processor) is not int or not 0 <= processor < 64:
        raise ValueError("logical processor must be an integer from 0 through 63")
    affinity = affinity if affinity is not None else _WindowsAffinity()
    original = affinity.get_mask()
    selected = 1 << processor
    if original & selected != selected:
        raise ValueError("requested logical processor is outside the allowed mask")
    affinity.set_mask(selected)
    child = None
    try:
        child = subprocess.Popen(command, **kwargs)
    finally:
        try:
            affinity.set_mask(original)
        except Exception:
            if child is not None:
                child.terminate()
                child.wait(timeout=10)
            raise
    return child


def sha(path):
    with path.open("rb") as f:
        return hashlib.file_digest(f,"sha256").hexdigest()



def publish_command(root, name, item, timeout=5.0):
    # The runtime consumes every regular file in commands, regardless of suffix.
    # Stage outside the watched directory, then publish a complete closed file.
    staging = root / "staging"
    staging.mkdir(exist_ok=True)
    tmp = staging / (name + ".tmp")
    tmp.write_text("\n".join(f"{k}={v}" for k,v in item.items())+"\n", encoding="utf-8")
    deadline = time.monotonic() + timeout
    while True:
        try:
            tmp.rename(root / "commands" / name)
            return
        except OSError as exc:
            if getattr(exc, "winerror", None) not in (32, 33) or time.monotonic() >= deadline:
                raise
            time.sleep(.05)



def publish_route(root, commands, start_index):
    """Queue complete files before waiting, avoiding a host round trip per action.

    The runtime still applies frame-bounded commands; this is not a deterministic
    input movie. Advisory status can lag while the native queue is draining.
    """
    pending = []
    for offset, item in enumerate(commands, start_index + 1):
        name = f"{offset:06d}.txt"
        publish_command(root, name, item)
        pending.append((name, item))
    return pending


def validate_command_receipts(root, count):
    expected = {f"{i:06d}.txt" for i in range(1, count + 1)}
    processed = {p.name for p in (root / "processed").iterdir() if p.is_file()}
    failed = [p.name for p in (root / "failed").iterdir() if p.is_file()]
    if failed or processed != expected:
        raise RuntimeError(f"command receipt mismatch: failed={failed}, missing={sorted(expected-processed)}, unexpected={sorted(processed-expected)}")


def command_failure_detail(root, name):
    try:
        detail = (root / "errors" / name).read_text(encoding="utf-8").strip()
    except OSError:
        detail = ""
    return detail or f"automation command {name} failed; inspect runtime.log"


def configure_benchmark_profile(user, enabled):
    # Only the copied benchmark profile is changed. Explicitly disable inherited
    # profiling for ordinary runs so a diagnostic cannot contaminate acceptance.
    path = user / "Config/Dolphin.ini"
    config = configparser.ConfigParser(interpolation=None, strict=False)
    config.optionxform = str
    config.read(path, encoding="utf-8")
    if not config.has_section("Core"):
        config.add_section("Core")
    config.set("Core", "CPUThread", "False")
    for section, key in (("Interface", "DebugModeEnabled"), ("Debug", "JitEnableProfiling")):
        if not config.has_section(section):
            config.add_section(section)
        config.set(section, key, "True" if enabled else "False")
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as output:
        config.write(output)


def configure_audio_profile(env, enabled, root):
    # Never inherit intrusive profiling into an ordinary performance run.
    env.pop("OPENMUA2_AUDIO_PROFILE", None)
    if enabled:
        env["OPENMUA2_AUDIO_PROFILE"] = str(root / "audio-profile.csv")


def configure_audio_capture(env, enabled, root):
    env.pop('OPENMUA2_AUDIO_CAPTURE', None)
    if enabled: env['OPENMUA2_AUDIO_CAPTURE'] = str(root / 'mixed-audio.wav')



def validate_audio_capture(root):
    import wave
    with wave.open(str(root / 'mixed-audio.wav'), 'rb') as capture:
        if capture.getnchannels() != 2 or capture.getsampwidth() != 2 or not 0 < capture.getnframes() <= capture.getframerate() * 60:
            raise RuntimeError('invalid bounded stereo audio capture')
        frames = capture.getnframes()
        if len(capture.readframes(frames)) != frames * 4:
            raise RuntimeError('truncated audio capture')


def validate_audio_profile(root, log):
    import csv
    if "audio backend: Cubeb" not in log.splitlines():
        raise RuntimeError("audio profile did not confirm Cubeb")
    path = root / "audio-profile.csv"
    if not path.is_file():
        raise RuntimeError("audio profile was not flushed after callbacks stopped")
    with path.open() as stream:
        rows = list(csv.DictReader(stream))
    if not any(row.get("channel") == "11" and int(row.get("calls", 0)) > 0 for row in rows):
        raise RuntimeError("audio profile contains no Cubeb callbacks")


def validate_formatter(log, mode):
    lines = [line for line in log.splitlines() if line.startswith('Simple formatter: ')]
    if not mode:
        if lines: raise RuntimeError('unexpected formatter experiment in ordinary run')
        return
    if len(lines) != 1: raise RuntimeError('missing or ambiguous formatter summary')
    fields = dict(item.split('=', 1) for item in lines[0].split()[2:])
    if fields.get('mode') != mode: raise RuntimeError('formatter mode mismatch')
    try:
        values = {key: int(fields[key]) for key in ('eligible', 'replaced', 'compared',
                  'mismatches', 'fpscr_mismatches', 'abi_mismatches', 'abandoned', 'pending')}
    except (KeyError, ValueError) as exc:
        raise RuntimeError('incomplete formatter counters') from exc
    if values['eligible'] <= 0 or any(values[k] != 0 for k in
            ('mismatches', 'fpscr_mismatches', 'abi_mismatches', 'abandoned')):
        raise RuntimeError('formatter comparison failed or no eligible calls')
    active, inactive = ('compared', 'replaced') if mode == 'shadow' else ('replaced', 'compared')
    # Shutdown may interrupt an original call. Account for it explicitly without
    # treating that pending call as a completed comparison.
    pending = values['pending']
    if not 0 <= pending <= (16 if mode == 'shadow' else 0):
        raise RuntimeError('invalid formatter pending count')
    if values[active] <= 0 or values[active] + pending != values['eligible'] or values[inactive] != 0:
        raise RuntimeError('formatter counters do not reconcile')


def validate_timed_inputs(root, commands, sequence_start=None):
    seen = set()
    previous_sequence_end = None
    previous_actual_end = None
    for command in commands:
        if command.get('command') != 'xbox_time':
            continue
        path = (root / command['path']).resolve()
        if path in seen:
            raise RuntimeError('timed input receipts must have unique paths')
        seen.add(path)
        try:
            fields = dict(line.split('=', 1) for line in path.read_text().splitlines())
            start, end, duration, hz, late, completed, release = (
                int(fields[k]) for k in ('start_ticks', 'end_ticks', 'duration_ticks',
                                        'ticks_per_second', 'cycles_late', 'completed', 'release'))
        except (OSError, KeyError, ValueError) as exc:
            raise RuntimeError('missing or invalid timed input receipt') from exc
        try:
            scheduled_start = int(fields.get('scheduled_start_ticks', start))
            scheduled_end = int(fields.get('scheduled_end_ticks', start + duration))
        except ValueError as exc:
            raise RuntimeError('invalid sequence deadline') from exc
        sequence = 'scheduled_start_ticks' in fields or 'scheduled_end_ticks' in fields
        if sequence_start is not None and not sequence:
            raise RuntimeError('missing sequence deadlines')
        if sequence and (not {'scheduled_start_ticks', 'scheduled_end_ticks'} <= fields.keys() or
                         (previous_sequence_end is not None and scheduled_start != previous_sequence_end) or
                         (previous_actual_end is not None and start != previous_actual_end) or
                         (previous_sequence_end is None and sequence_start not in (None, 0) and
                          scheduled_start != sequence_start)):
            raise RuntimeError('sequence schedule contains a gap or incomplete receipt')
        if sequence:
            previous_sequence_end = scheduled_end
            previous_actual_end = end
        if (completed != 1 or hz <= 0 or start < 0 or duration <= 0 or late < 0 or
                scheduled_start < 0 or scheduled_end - scheduled_start != duration or end - scheduled_end != late or
                not 0 <= start - scheduled_start <= hz // 1000 or end <= start or late > hz // 1000 or
                duration != hz * int(command['milliseconds']) // 1000 or
                release != int(command.get('release', 1))):
            raise RuntimeError('timed input incomplete, inconsistent, or more than 1ms late')


def validate_runtime_settings(log, cpu, logical_processor=None):
    import re
    expected_cpu = "CPU backend: " + ("JIT" if cpu == "jit" else "StaticRecomp")
    if expected_cpu not in log.splitlines():
        raise RuntimeError("runtime did not confirm the requested CPU backend")
    video = [line for line in log.splitlines() if line.startswith("Effective video: ")]
    if not video or any(not line.startswith("Effective video: dual_core=0 ") for line in video):
        raise RuntimeError("runtime did not confirm serial CPU/GPU execution")
    affinity = [line for line in log.splitlines() if line.startswith("Host CPU affinity:")]
    if not affinity:
        raise RuntimeError("runtime did not confirm whole-process CPU affinity")
    for line in affinity:
        match = re.fullmatch(r"Host CPU affinity: logical_processors=1 mask=0x([0-9a-fA-F]+)", line)
        if not match or int(match[1], 16).bit_count() != 1:
            raise RuntimeError("runtime is not restricted to one host logical processor")
        if logical_processor is not None and int(match[1], 16) != 1 << logical_processor:
            raise RuntimeError("runtime did not confirm the requested logical processor")


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ("runner","game","user","state","route","output"):
        p.add_argument("--"+name,required=True,type=Path)
    p.add_argument("--module",type=Path,help="Required only for static recompilation.")
    p.add_argument("--cpu",choices=("jit","staticrecomp"),help="Defaults to jit; selective native diagnostics imply staticrecomp.")
    p.add_argument("--windowed",action="store_true",help="Create the game window; inputs remain process-local.")
    p.add_argument("--audio",default="No Audio Output",help="Runtime backend name, e.g. Cubeb; requires --windowed or --profile-audio for sound.")
    p.add_argument("--profile-audio",action="store_true",help="Opt-in numeric audio diagnostics; permits Cubeb without a game window. Not presentation/FPS acceptance.")
    p.add_argument("--profile-runtime",action="store_true",help="Bounded throttle/GPU/presentation spans; diagnostic overhead, not acceptance.")
    p.add_argument("--capture-audio",action="store_true",help="Private pre-volume stereo PCM tail (60s); requires --profile-audio and Cubeb. Not device playback.")
    p.add_argument("--resolution",default="1920x1080")
    p.add_argument("--timeout",type=float,default=600)
    p.add_argument("--logical-processor", type=int, help="Windows: select one allowed logical CPU for the whole child game process; restore the harness immediately.")
    p.add_argument("--no-trace",action="store_true")
    p.add_argument("--guest-sequence-start", type=int, help="Preload timed inputs/read-only probes into one guest-clock sequence; absolute start tick, or 0 for now. No screenshots, saves, writes or frame-based inputs allowed.")
    p.add_argument("--queue-route", action="store_true", help="Publish the whole route before waiting; removes per-action host round trips. Advisory status may lag.")
    p.add_argument("--trace-presentation",action="store_true",help="Diagnostic copy/before/after events; copy counts are not FPS.")
    p.add_argument("--present-queue",action="store_true",help="Experimental half-refresh FIFO presentation; adds latency, logs shortages/overflow.")
    p.add_argument("--no-screenshots",action="store_true")
    p.add_argument("--jit-diagnostic",action="store_true")
    p.add_argument("--profile-dispatch",action="store_true")
    p.add_argument("--jit-block-profile",action="store_true",help="Intrusive full-JIT resident-block counters; diagnostic only, never release-FPS evidence.")
    p.add_argument("--jit-profile-callers",help="With --jit-block-profile: up to 32 comma-separated aligned hexadecimal block addresses; private bounded caller/argument samples.")
    p.add_argument("--simple-format", choices=("shadow", "on"), help="Default-off simple formatter experiment; shadow compares original output/state, on replaces supported calls.")
    p.add_argument("--jit-ranges",help="Diagnostic: comma-separated hexadecimal start-end ranges use JIT within the native core.")
    p.add_argument('--jit-emission-address', type=lambda value: int(value, 0), help='Slow instruction timing (>100us) for one JIT block, or 0xffffffff for all; requires --profile-runtime.')
    args=p.parse_args()
    if args.logical_processor is not None and not 0 <= args.logical_processor < 64:
        p.error("--logical-processor must be between 0 and 63")
    if args.jit_emission_address is not None and (not args.profile_runtime or not 0 < args.jit_emission_address <= 0xffffffff):
        p.error('--jit-emission-address requires --profile-runtime and a nonzero 32-bit address')
    if args.capture_audio and (not args.profile_audio or args.audio != 'Cubeb'):
        p.error('--capture-audio requires --profile-audio --audio Cubeb')
    if args.jit_ranges and args.jit_diagnostic:
        p.error("--jit-ranges and --jit-diagnostic are mutually exclusive")
    cpu = args.cpu or ("staticrecomp" if args.jit_ranges or args.profile_dispatch else "jit")
    if args.jit_diagnostic and cpu != "jit":
        p.error("--jit-diagnostic requires the jit CPU backend")
    if args.jit_block_profile and cpu != "jit":
        p.error("--jit-block-profile requires the jit CPU backend")
    if args.simple_format and cpu != "jit":
        p.error("--simple-format requires the jit CPU backend")
    if args.jit_profile_callers:
        import re
        addresses = args.jit_profile_callers.split(',')
        if not args.jit_block_profile or len(addresses) > 32 or any(
                not re.fullmatch(r'[0-9a-fA-F]{1,8}', item) or int(item,16) % 4
                for item in addresses):
            p.error("--jit-profile-callers requires --jit-block-profile and 1..32 aligned hexadecimal addresses")
    if (args.jit_ranges or args.profile_dispatch) and cpu != "staticrecomp":
        p.error("native profiling and selective JIT require --cpu staticrecomp")
    if cpu == "staticrecomp" and args.module is None:
        p.error("--cpu staticrecomp requires --module")
    if args.jit_ranges:
        import re
        for item in args.jit_ranges.split(","):
            if not re.fullmatch(r"[0-9a-fA-F]{1,8}-[0-9a-fA-F]{1,8}", item) or int(item.split("-")[0],16) >= int(item.split("-")[1],16):
                p.error("--jit-ranges requires valid hexadecimal start-end ranges")
    if args.profile_audio and args.audio != "Cubeb":
        p.error("--profile-audio currently requires --audio Cubeb")
    if not args.windowed and not args.profile_audio and args.audio not in ("Null", "No Audio Output"):
        p.error("audio output requires --windowed: the runtime forces silent audio when headless")
    root=args.output.resolve(); root.mkdir(parents=True,exist_ok=False)
    for name in ("commands","processed","failed","shots"):(root/name).mkdir()
    shutil.copytree(args.user,root/"user")
    configure_benchmark_profile(root/"user", args.jit_block_profile)
    cfg=root/"user/config.ini"
    text=cfg.read_text(); text="\n".join("resolution="+args.resolution if line.startswith("resolution=") else line for line in text.splitlines())+"\n"
    cfg.write_text(text)
    route=json.loads(args.route.read_text())
    if args.no_screenshots:
        route["commands"]=[x for x in route["commands"] if x.get("command")!="screenshot"]
    execution_commands = route["commands"]
    if args.guest_sequence_start is not None:
        if args.guest_sequence_start < 0: p.error('--guest-sequence-start must be nonnegative')
        allowed = {'xbox_time', 'read_memory', 'read_timing', 'check_memory'}
        if any(item.get('command') not in allowed for item in execution_commands):
            p.error('guest sequence permits only timed Xbox input and read-only probes/guards')
        sequence_dir = root / 'sequence-inputs'
        sequence_dir.mkdir()
        for number, item in enumerate(execution_commands):
            item = dict(item)
            if 'path' in item: item['path'] = str((root / item['path']).resolve())
            (sequence_dir / f'{number:06d}.txt').write_text(
                '\n'.join(f'{key}={value}' for key, value in item.items()) + '\n', encoding='utf-8')
        execution_commands = [{'command': 'xbox_sequence', 'path': str(sequence_dir),
                               'start_ticks': args.guest_sequence_start}]
    env=os.environ.copy()
    configure_audio_profile(env, args.profile_audio, root)
    configure_audio_capture(env, args.capture_audio, root)
    env.pop('OPENMUA2_JIT_EMISSION_ADDRESS', None)
    if args.jit_emission_address is not None: env['OPENMUA2_JIT_EMISSION_ADDRESS'] = format(args.jit_emission_address, 'x')
    env.pop("OPENMUA2_RUNTIME_SPANS", None)
    if args.profile_runtime: env["OPENMUA2_RUNTIME_SPANS"] = str(root / "runtime-spans.csv")
    env.pop("MODERNGEKKO_SIMPLE_FORMAT",None)
    if args.simple_format:env["MODERNGEKKO_SIMPLE_FORMAT"]=args.simple_format
    env.pop("MODERNGEKKO_JIT_PROFILE_CALLERS",None)
    if args.jit_profile_callers:env["MODERNGEKKO_JIT_PROFILE_CALLERS"]=args.jit_profile_callers
    env.pop("MODERNGEKKO_PRESENT_QUEUE",None)
    if args.present_queue:env["MODERNGEKKO_PRESENT_QUEUE"]="1"
    env.pop("MODERNGEKKO_PRESENT_TIMES",None)
    if args.trace_presentation:env["MODERNGEKKO_PRESENT_TIMES"]=str(root/"presentation.csv")
    for key in ("STATICRECOMP_DISPATCH_SAMPLES","STATICRECOMP_FALLBACK_SAMPLES","STATICRECOMP_PROFILE_DISPATCH","STATICRECOMP_PROFILE_GATE_FILE","STATICRECOMP_TRACE_FILE","MODERNGEKKO_FRAME_TIMES","MODERNGEKKO_STATICRECOMP","STATICRECOMP_FALLBACK_RANGES","STATICRECOMP_FALLBACK_USE_JIT"):
        env.pop(key,None)
    if not args.no_trace:env["MODERNGEKKO_FRAME_TIMES"]=str(root/"frames.csv")
    if args.jit_ranges:
        env["STATICRECOMP_FALLBACK_RANGES"]=args.jit_ranges
        env["STATICRECOMP_FALLBACK_USE_JIT"]="1"
    if args.profile_dispatch:
        env["STATICRECOMP_PROFILE_DISPATCH"]="1"
        env["STATICRECOMP_PROFILE_GATE_FILE"]=str(root/"profile.enabled")
    cmd=[str(args.runner.resolve()),"--game",str(args.game.resolve()),"--cpu",cpu,"--user-dir",str(root/"user"),"--automation-dir",str(root),"--graphics","Vulkan","--audio",args.audio,"--load-state",str(args.state.resolve())]
    if cpu == "staticrecomp":cmd += ["--module",str(args.module.resolve())]
    if not args.windowed:cmd.append("--headless")
    metadata={"command":cmd,"cpu":cpu,"runner_sha256":sha(args.runner),"module_sha256":sha(args.module) if cpu == "staticrecomp" else None,"state_sha256":sha(args.state),"route":route,"resolution":args.resolution,"trace":not args.no_trace,"jit_diagnostic":args.jit_diagnostic,"jit_ranges":args.jit_ranges,"profile_dispatch":args.profile_dispatch,"profile_scope":"after restored frame threshold" if args.profile_dispatch else None,"screenshots":not args.no_screenshots,"headless":not args.windowed,"requested_audio_backend":args.audio}
    metadata["queued_route"] = args.queue_route
    metadata["guest_sequence_start"] = args.guest_sequence_start
    metadata["execution_commands"] = execution_commands
    metadata["timeline_status_scope"] = "advisory, possibly stale while native queue drains" if args.queue_route else "advisory"
    metadata["presentation_trace"] = args.trace_presentation
    metadata["audio_profile"] = args.profile_audio
    metadata["audio_capture"] = args.capture_audio
    metadata["runtime_profile"] = args.profile_runtime
    metadata["jit_emission_address"] = args.jit_emission_address
    metadata["presentation_queue"] = args.present_queue
    metadata["jit_block_profile"] = args.jit_block_profile
    metadata["jit_profile_callers"] = args.jit_profile_callers
    metadata["single_core_required"] = True
    metadata["requested_logical_processor"] = args.logical_processor
    metadata["host_logical_processors_required"] = 1
    metadata["simple_format"] = args.simple_format or "off"
    metadata["jit_block_profile_scope"] = "resident blocks after restored frame threshold; intrusive, invalidated blocks excluded" if args.jit_block_profile else None
    (root/"run.json").write_text(json.dumps(metadata,indent=2))
    started=time.monotonic(); index=0; timeline=[]
    with (root/"runtime.log").open("w") as log:
        child=launch_on_processor(cmd,args.logical_processor,stdout=log,stderr=subprocess.STDOUT,env=env,creationflags=subprocess.CREATE_NO_WINDOW if os.name=="nt" else 0)
        (root/"pid.txt").write_text(str(child.pid))
        def check():
            if child.poll() is not None:raise RuntimeError(f"runtime exited {child.returncode}")
            if time.monotonic()-started>args.timeout:raise TimeoutError("benchmark deadline")
        def read_status():
            # Windows can temporarily deny sharing while the runtime replaces
            # this advisory file. Command acknowledgments are authoritative.
            try:return (root/"status.txt").read_text()
            except OSError:return ""
        def wait_receipt(name, item):
            while not (root/"processed"/name).exists():
                check()
                if (root/"failed"/name).exists():raise RuntimeError(command_failure_detail(root, name))
                time.sleep(.05)
            timeline.append({"command":item,"elapsed":time.monotonic()-started,"status":read_status()})
        def submit(item):
            nonlocal index
            index+=1; name=f"{index:06d}.txt"
            publish_command(root,name,item)
            wait_receipt(name,item)
        completed=False
        failure=None
        try:
            # A booted flag precedes LoadState completion. Require a restored frame ID.
            while True:
                check()
                try: status=dict(line.split("=",1) for line in (root/"status.txt").read_text().splitlines() if "=" in line)
                except OSError:status={}
                if int(status.get("frame_count","0"))>=int(route["restored_frame_min"]):break
                time.sleep(.1)
            if args.profile_dispatch:
                (root/"profile.enabled").write_text("restored combat state\n")
            if args.jit_block_profile:
                submit({"command":"jit_profile_reset"})
                submit({"command":"read_timing","path":"jit-profile-start.txt"})
            if args.queue_route:
                pending = publish_route(root, execution_commands, index)
                index += len(pending)
                for name, item in pending: wait_receipt(name, item)
            else:
                for item in execution_commands: submit(item)
            if args.jit_block_profile:
                submit({"command":"read_timing","path":"jit-profile-end.txt"})
                submit({"command":"jit_profile_dump","path":"jit-blocks.tsv"})
            submit({"command":"pad_frames","port":0,"frames":2})
            submit({"command":"stop"})
            child.wait(timeout=30)
            if child.returncode:raise RuntimeError(f"runtime exited {child.returncode}")
            validate_command_receipts(root,index)
            validate_timed_inputs(root, route['commands'], args.guest_sequence_start)
            validate_runtime_settings((root/"runtime.log").read_text(), cpu, args.logical_processor)
            validate_formatter((root/"runtime.log").read_text(), args.simple_format)
            if args.profile_audio:
                validate_audio_profile(root, (root/"runtime.log").read_text())
            if args.capture_audio: validate_audio_capture(root)
            if args.profile_runtime:
                spans = (root / "runtime-spans.csv").read_text()
                if "# dropped_samples=0\n" not in spans or "throttle," not in spans:
                    raise RuntimeError("runtime span trace missing throttle events or dropped samples")
            completed=True
        except Exception as exc:
            failure = f"{type(exc).__name__}: {exc}"
            raise
        finally:
            if child.poll() is None:
                try:publish_command(root,"999999-stop.txt",{"command":"stop"})
                except OSError as exc:print(f"Unable to publish cleanup stop: {exc}",flush=True)
                try:child.wait(timeout=20)
                except subprocess.TimeoutExpired:child.terminate();child.wait(timeout=10)
            (root/"result.json").write_text(json.dumps({"route_completed":completed,"exit_code":child.returncode,"error":failure,"elapsed":time.monotonic()-started,"timeline":timeline},indent=2))
    print(root,flush=True)

if __name__=="__main__":main()
