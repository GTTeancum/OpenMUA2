"""Build with installed Roslyn/.NET Framework. No downloads or package restore."""
import json, os, re, subprocess, sys, zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
src=root/'tools/alchemy_batch'
addon=root/'.local/luke-pc-20261018/igb-blender-reference'
out=root/'.local/AlchemyBatchConverter'
out.mkdir(parents=True,exist_ok=True)
build=out/'build';build.mkdir(exist_ok=True)
env=dict(os.environ, TEMP=str(build),TMP=str(build),DOTNET_CLI_HOME=str(build),DOTNET_SKIP_FIRST_TIME_EXPERIENCE='1',DOTNET_NOLOGO='1',PYTHONDONTWRITEBYTECODE='1')
head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=addon,text=True).strip()
if not head.startswith('4672c8d'): raise SystemExit('Unexpected igb-blender revision: '+head)
files=subprocess.check_output(['git','ls-files','-z'],cwd=addon).decode().split('\0')
with zipfile.ZipFile(build/'addon.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in files:
  if f and (addon/f).is_file(): z.write(addon/f,f)
profiles={'auto':'Auto-detect (review detected profile)'}
profiles.update(re.findall(r'game_id="([^"]+)",\s*game_name="([^"]+)"',(addon/'game_profiles.py').read_text()))
(build/'profiles.json').write_text(json.dumps(profiles),encoding='utf-8')
ref=Path(r'C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.8.1')
csc=Path(r'C:\Program Files\dotnet\sdk\9.0.314\Roslyn\bincore\csc.dll')
refs=['mscorlib','System','System.Core','System.Xaml','System.Web.Extensions','System.Windows.Forms','System.Drawing','WindowsBase','PresentationCore','PresentationFramework','System.IO.Compression','System.IO.Compression.FileSystem']
args=['dotnet',str(csc),'/nologo','/target:winexe','/platform:x64','/optimize+','/langversion:latest','/nostdlib+',f'/out:{out / "AlchemyBatchConverter.exe"}']
args += [f'/reference:{ref / (n+".dll")}' for n in refs]
args += [f'/resource:{p},{name}' for p,name in [(src/'Main.xaml','Main.xaml'),(src/'worker.py','worker.py'),(build/'profiles.json','profiles.json'),(build/'addon.zip','addon.zip')]]
args += [str(src/'App.cs')]
subprocess.run(args,cwd=root,env=env,check=True)
(out/'BUILD.json').write_text(json.dumps({'addon_commit':head,'profiles':len(profiles)-1,'compiler':str(csc),'runtime':'.NET Framework 4.8.1','blender':'4.5'},indent=2))
print(out/'AlchemyBatchConverter.exe')
