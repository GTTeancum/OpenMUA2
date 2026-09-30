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


def validate_runtime_settings(log, cpu):
    expected_cpu = "CPU backend: " + ("JIT" if cpu == "jit" else "StaticRecomp")
    if expected_cpu not in log.splitlines():
        raise RuntimeError("runtime did not confirm the requested CPU backend")
    video = [line for line in log.splitlines() if line.startswith("Effective video: ")]
    if not video or any(not line.startswith("Effective video: dual_core=0 ") for line in video):
        raise RuntimeError("runtime did not confirm required single-core emulation")


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ("runner","game","user","state","route","output"):
        p.add_argument("--"+name,required=True,type=Path)
    p.add_argument("--module",type=Path,help="Required only for static recompilation.")
    p.add_argument("--cpu",choices=("jit","staticrecomp"),help="Defaults to jit; selective native diagnostics imply staticrecomp.")
    p.add_argument("--windowed",action="store_true",help="Create the game window; inputs remain process-local.")
    p.add_argument("--audio",default="No Audio Output",help="Runtime backend name, e.g. Cubeb; requires --windowed for sound.")
    p.add_argument("--resolution",default="1920x1080")
    p.add_argument("--timeout",type=float,default=600)
    p.add_argument("--no-trace",action="store_true")
    p.add_argument("--trace-presentation",action="store_true",help="Diagnostic copy/before/after events; copy counts are not FPS.")
    p.add_argument("--present-queue",action="store_true",help="Experimental half-refresh FIFO presentation; adds latency, logs shortages/overflow.")
    p.add_argument("--no-screenshots",action="store_true")
    p.add_argument("--jit-diagnostic",action="store_true")
    p.add_argument("--profile-dispatch",action="store_true")
    p.add_argument("--jit-block-profile",action="store_true",help="Intrusive full-JIT resident-block counters; diagnostic only, never release-FPS evidence.")
    p.add_argument("--jit-profile-callers",help="With --jit-block-profile: up to 32 comma-separated aligned hexadecimal block addresses; private bounded caller/argument samples.")
    p.add_argument("--jit-ranges",help="Diagnostic: comma-separated hexadecimal start-end ranges use JIT within the native core.")
    args=p.parse_args()
    if args.jit_ranges and args.jit_diagnostic:
        p.error("--jit-ranges and --jit-diagnostic are mutually exclusive")
    cpu = args.cpu or ("staticrecomp" if args.jit_ranges or args.profile_dispatch else "jit")
    if args.jit_diagnostic and cpu != "jit":
        p.error("--jit-diagnostic requires the jit CPU backend")
    if args.jit_block_profile and cpu != "jit":
        p.error("--jit-block-profile requires the jit CPU backend")
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
    if not args.windowed and args.audio not in ("Null", "No Audio Output"):
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
    env=os.environ.copy()
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
    metadata["presentation_trace"] = args.trace_presentation
    metadata["presentation_queue"] = args.present_queue
    metadata["jit_block_profile"] = args.jit_block_profile
    metadata["jit_profile_callers"] = args.jit_profile_callers
    metadata["single_core_required"] = True
    metadata["jit_block_profile_scope"] = "resident blocks after restored frame threshold; intrusive, invalidated blocks excluded" if args.jit_block_profile else None
    (root/"run.json").write_text(json.dumps(metadata,indent=2))
    started=time.monotonic(); index=0; timeline=[]
    with (root/"runtime.log").open("w") as log:
        child=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT,env=env,creationflags=subprocess.CREATE_NO_WINDOW if os.name=="nt" else 0)
        (root/"pid.txt").write_text(str(child.pid))
        def check():
            if child.poll() is not None:raise RuntimeError(f"runtime exited {child.returncode}")
            if time.monotonic()-started>args.timeout:raise TimeoutError("benchmark deadline")
        def read_status():
            # Windows can temporarily deny sharing while the runtime replaces
            # this advisory file. Command acknowledgments are authoritative.
            try:return (root/"status.txt").read_text()
            except OSError:return ""
        def submit(item):
            nonlocal index
            index+=1; name=f"{index:06d}.txt"
            publish_command(root,name,item)
            while not (root/"processed"/name).exists():
                check()
                if (root/"failed"/name).exists():raise RuntimeError(command_failure_detail(root, name))
                time.sleep(.05)
            timeline.append({"command":item,"elapsed":time.monotonic()-started,"status":read_status()})
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
            for item in route["commands"]:submit(item)
            if args.jit_block_profile:
                submit({"command":"read_timing","path":"jit-profile-end.txt"})
                submit({"command":"jit_profile_dump","path":"jit-blocks.tsv"})
            submit({"command":"pad_frames","port":0,"frames":2})
            submit({"command":"stop"})
            child.wait(timeout=30)
            if child.returncode:raise RuntimeError(f"runtime exited {child.returncode}")
            validate_command_receipts(root,index)
            validate_runtime_settings((root/"runtime.log").read_text(), cpu)
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
