"""Compare reachable menu triangles through igb-blender's independent decoders."""
import sys,types,importlib,collections,json,struct,argparse
from pathlib import Path
repo=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--writer-root',type=Path,required=True)
parser.add_argument('--source',type=Path,required=True)
parser.add_argument('--converted',type=Path,required=True)
parser.add_argument('--report',type=Path,required=True)
args=parser.parse_args()
addon,source_root,converted,report=(p.resolve() for p in (args.writer_root,args.source,args.converted,args.report))
if any(not p.is_relative_to(repo/'.local') for p in (addon,source_root,converted,report)):
 raise ValueError('All private verification inputs and outputs must stay in repository/.local')
sys.dont_write_bytecode=True
pkg=types.ModuleType('menu_igb_addon');pkg.__path__=[str(addon)];sys.modules[pkg.__name__]=pkg
Reader=importlib.import_module('menu_igb_addon.igb_format.igb_reader').IGBReader
extract=importlib.import_module('menu_igb_addon.scene_graph.sg_geometry').extract_geometry
profile=importlib.import_module('menu_igb_addon.game_profiles').get_profile
sys.path.insert(0,str(repo/'tools'))
from convert_pc_menu_assets import red
def fingerprint(path,source):
 r=Reader(str(path));r.read();out=collections.Counter();meshes=0
 reachable=set()
 def walk(index):
  if index<0 or index in reachable:return
  reachable.add(index);obj=r.objects[index]
  if not hasattr(obj,'_raw_fields'):return
  for slot,value,definition in obj._raw_fields:
   if definition.short_name==b'ObjectRef':walk(value)
   elif definition.short_name==b'MemoryRef' and slot==4 and obj.is_type(b'igObjectList') and value>=0:
    for ref, in struct.iter_unpack('<i',r.objects[value].data):walk(ref)
 walk(r.info_list_index)
 tint=source and path.stem.startswith(('m_options_','m_pda_','m_share_','m_team_','x2m_'))
 for index,obj in enumerate(r.objects):
  if index not in reachable:continue
  if not hasattr(obj,'is_type') or not (obj.is_type(b'igGeometryAttr1_5') or obj.is_type(b'igGeometryAttr2')):continue
  g=extract(r,obj,profile('xml2_pc' if source else 'mua_pc'))
  if g is None:raise ValueError('Geometry extraction failed')
  meshes+=1
  def vertex(i):
   values=[g.positions[i]]
   values += [g.normals[i] if g.normals else (),g.uvs[i] if g.uvs else ()]
   color=g.colors[i] if g.colors else ()
   if color and tint:
    color=tuple(v/255 for v in red(tuple(round(v*255) for v in color[:3])))+(color[3],)
   values.append(color)
   return tuple(tuple(round(v,5) for v in row) for row in values)
  indices=g.triangulate()
  for k in range(0,len(indices),3):
   triangle=tuple(vertex(i) for i in indices[k:k+3])
   if len({v[0] for v in triangle})<3:continue
   triangle=min(triangle,triangle[1:]+triangle[:1],triangle[2:]+triangle[:2])
   out[triangle]+=1
 return out,meshes
records=[]
for source in sorted(source_root.rglob('*.igb')):
 target=converted/source.relative_to(source_root)
 before,n=fingerprint(source,True);after,m=fingerprint(target,False)
 if before!=after or n!=m:
  print('MISMATCH',source.name,n,m,sum(before.values()),sum(after.values()))
  print('missing',list((before-after).items())[:1]);print('extra',list((after-before).items())[:1]);raise SystemExit(1)
 records.append(dict(file=source.relative_to(source_root).as_posix(),meshes=n,triangles=sum(after.values())))
report.parent.mkdir(parents=True,exist_ok=True)
report.write_text(json.dumps(records,indent=2)+'\n')
print('Verified',sum(x['meshes'] for x in records),'meshes and',sum(x['triangles'] for x in records),'triangles, including winding, positions, normals, UVs and intended recolored vertex colors')
