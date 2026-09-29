"""Replay process-local combat commands in an isolated OpenMUA2 profile."""
import argparse
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


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ("runner","module","game","user","state","route","output"):
        p.add_argument("--"+name,required=True,type=Path)
    p.add_argument("--windowed",action="store_true",help="Create the game window; inputs remain process-local.")
    p.add_argument("--audio",default="No Audio Output",help="Runtime backend name, e.g. Cubeb; requires --windowed for sound.")
    p.add_argument("--resolution",default="1920x1080")
    p.add_argument("--timeout",type=float,default=600)
    p.add_argument("--no-trace",action="store_true")
    p.add_argument("--no-screenshots",action="store_true")
    p.add_argument("--jit-diagnostic",action="store_true")
    p.add_argument("--profile-dispatch",action="store_true")
    p.add_argument("--jit-ranges",help="Diagnostic: comma-separated hexadecimal start-end ranges use JIT within the native core.")
    args=p.parse_args()
    if args.jit_ranges and args.jit_diagnostic:
        p.error("--jit-ranges and --jit-diagnostic are mutually exclusive")
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
    cfg=root/"user/config.ini"
    text=cfg.read_text(); text="\n".join("resolution="+args.resolution if line.startswith("resolution=") else line for line in text.splitlines())+"\n"
    cfg.write_text(text)
    route=json.loads(args.route.read_text())
    if args.no_screenshots:
        route["commands"]=[x for x in route["commands"] if x.get("command")!="screenshot"]
    env=os.environ.copy()
    for key in ("STATICRECOMP_DISPATCH_SAMPLES","STATICRECOMP_FALLBACK_SAMPLES","STATICRECOMP_PROFILE_DISPATCH","STATICRECOMP_PROFILE_GATE_FILE","STATICRECOMP_TRACE_FILE","MODERNGEKKO_FRAME_TIMES","MODERNGEKKO_STATICRECOMP","STATICRECOMP_FALLBACK_RANGES","STATICRECOMP_FALLBACK_USE_JIT"):
        env.pop(key,None)
    if not args.no_trace:env["MODERNGEKKO_FRAME_TIMES"]=str(root/"frames.csv")
    if args.jit_diagnostic:env["MODERNGEKKO_STATICRECOMP"]="0"
    if args.jit_ranges:
        env["STATICRECOMP_FALLBACK_RANGES"]=args.jit_ranges
        env["STATICRECOMP_FALLBACK_USE_JIT"]="1"
    if args.profile_dispatch:
        env["STATICRECOMP_PROFILE_DISPATCH"]="1"
        env["STATICRECOMP_PROFILE_GATE_FILE"]=str(root/"profile.enabled")
    cmd=[str(args.runner.resolve()),"--game",str(args.game.resolve()),"--module",str(args.module.resolve()),"--user-dir",str(root/"user"),"--automation-dir",str(root),"--graphics","Vulkan","--audio",args.audio,"--load-state",str(args.state.resolve())]
    if not args.windowed:cmd.append("--headless")
    metadata={"command":cmd,"runner_sha256":sha(args.runner),"module_sha256":sha(args.module),"state_sha256":sha(args.state),"route":route,"resolution":args.resolution,"trace":not args.no_trace,"jit_diagnostic":args.jit_diagnostic,"jit_ranges":args.jit_ranges,"profile_dispatch":args.profile_dispatch,"profile_scope":"after restored frame threshold" if args.profile_dispatch else None,"screenshots":not args.no_screenshots,"headless":not args.windowed,"requested_audio_backend":args.audio}
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
                if (root/"failed"/name).exists():raise RuntimeError(read_status() or "automation command failed")
                time.sleep(.05)
            timeline.append({"command":item,"elapsed":time.monotonic()-started,"status":read_status()})
        completed=False
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
            for item in route["commands"]:submit(item)
            submit({"command":"pad_frames","port":0,"frames":2})
            submit({"command":"stop"})
            child.wait(timeout=30)
            if child.returncode:raise RuntimeError(f"runtime exited {child.returncode}")
            validate_command_receipts(root,index)
            completed=True
        finally:
            if child.poll() is None:
                try:publish_command(root,"999999-stop.txt",{"command":"stop"})
                except OSError as exc:print(f"Unable to publish cleanup stop: {exc}",flush=True)
                try:child.wait(timeout=20)
                except subprocess.TimeoutExpired:child.terminate();child.wait(timeout=10)
            (root/"result.json").write_text(json.dumps({"route_completed":completed,"exit_code":child.returncode,"elapsed":time.monotonic()-started,"timeline":timeline},indent=2))
    print(root,flush=True)

if __name__=="__main__":main()
