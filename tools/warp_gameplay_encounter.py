"""Diagnostic party warp for the pinned RMSE52 executable layout.

Reads authored XMLB coordinates from private game data and validates the live
entity before contained process-local writes. Requires an already running,
isolated automation session. Leaves it paused. Never use on actual user saves.
Does not enable encounters, change objectives or assert interaction success.
"""
import json,struct,zipfile,math
from pathlib import Path
import argparse,time
from run_combat_benchmark import publish_command

class Session:
 def __init__(self,root):
  self.root=Path(root).resolve()
  for d in ('commands','processed','failed'):
   if not (self.root/d).is_dir():raise ValueError('Missing existing automation session: '+d)
  self.index=max([0]+[int(p.stem) for d in ('commands','processed','failed') for p in (self.root/d).glob('*.txt') if p.stem.isdigit()])
 def command(self,item):
  self.index+=1;name=f'{self.index:06}.txt';publish_command(self.root,name,item)
  until=time.monotonic()+120
  while not (self.root/'processed'/name).exists():
   if (self.root/'failed'/name).exists():raise RuntimeError('Command rejected: '+name)
   if time.monotonic()>until:raise TimeoutError('Pending command; do not resubmit: '+name)
   time.sleep(.05)
 def read(self,address,size):
  if not any(lo<=address and address+size<=hi for lo,hi in ((0x80000000,0x81800000),(0x90000000,0x94000000))):raise ValueError('Unmapped memory range')
  name='encounter-inspection.bin'
  self.command(dict(command='read_memory',address=address,size=size,path=name))
  return (self.root/name).read_bytes()
 u=staticmethod(lambda b,o=0:struct.unpack_from('>I',b,o)[0])

n=None
WAD=None
def map_nodes(map_name):
 with zipfile.ZipFile(WAD) as z:d=z.read('packages/generated/maps/'+map_name+'.fb')
 name=('maps/'+map_name+'.engb').encode()+b'\0';p=d.index(name)
 b=d[p+196:p+196+struct.unpack_from('<I',d,p+192)[0]]
 def w(p):return struct.unpack_from('<I',b,p)[0]
 def s(p):return b[p:b.index(b'\0',p)].decode('cp1252')
 seen=set()
 def node(o):
  assert o not in seen;seen.add(o)
  a={s(w(o+16+i*8)):s(w(o+20+i*8)) for i in range(w(o+12))}
  yield s(w(o)),a
  c=w(o+8)
  while c!=0xffffffff:
   yield from node(c);c=w(c+4)
 return list(node(8))
def live_entities():
 """Read active entities in one bounded slab; never dereference absent handles."""
 u=n.u;mgr=u(n.read(0x80817368,4));table=n.read(mgr,0x1600)
 ptrs=[u(table,4+i*4) for i in range(512) if (u(table,0xd30+4*(i//32))>>(i%32))&1]
 ptrs=[p for p in ptrs if 0x90000000<=p<0x94000000]
 assert ptrs and max(ptrs)-min(ptrs)<0x800000,'Unexpected entity arena'
 lo=min(ptrs);slab=n.read(lo,max(ptrs)-lo+0x300);strings=n.read(0x805f8828,0x40000);rows=[]
 for p in ptrs:
  b=slab[p-lo:p-lo+0x300];v=u(b,0x44)
  if v>>24!=u(strings) or (v&0xffffff)>=4096:continue
  off=0x4008+u(strings,4+(v&0xffffff)*4)
  name=strings[off:strings.find(b'\0',off)].decode('cp1252')
  rows.append(dict(name=name,address=p,position=struct.unpack_from('>3f',b,0x4c),health=struct.unpack_from('>f',b,0x2a0)[0],flags=b[:16].hex()))
 return rows

def warp(map_name,entity,offset=(0,0,0),yaw=None):
 if yaw is not None and not math.isfinite(yaw):raise ValueError("Yaw must be finite")
 nodes=map_nodes(map_name)
 matches=[a for t,a in nodes if t=='inst' and a.get('name')==entity and 'pos' in a]
 assert len(matches)==1,(entity,len(matches))
 pos=[float(v) for v in matches[0]['pos'].split()]
 target=[pos[i]+offset[i] for i in range(3)];assert all(math.isfinite(v) for v in target)
 n.command({'command':'pause'})
 live=[r for r in live_entities() if r['name']==entity]
 assert len(live)==1 and sum((live[0]['position'][i]-pos[i])**2 for i in range(3))<25,('Target absent or moved; refusing warp',entity,live)
 team=n.read(0x80629490,0x740);mgr=n.u(n.read(0x80817368,4));rows=[]
 for i in range(min(n.u(team,0x73c),4)):
  h=n.u(team,0x72c+4*i);actor=n.u(n.read(mgr+4+4*(h&511),4));b=n.read(actor,0x220)
  old=struct.unpack_from('>3f',b,0x4c);dest=[target[0]+(i%2)*24,target[1]-(i//2)*24,target[2]]
  delta=[dest[j]-old[j] for j in range(3)]
  for off in (0x4c,0x208,0x64,0x70,0x7c,0x88):
   before=b[off:off+12];xyz=struct.unpack('>3f',before)
   after=struct.pack('>3f',*(dest if off in (0x4c,0x208) else [xyz[j]+delta[j] for j in range(3)]))
   n.command(dict(command='check_memory',address=actor+off,data=before.hex()))
   n.command(dict(command='write_memory',address=actor+off,data=after.hex()))
  if yaw is not None:
   before=n.read(actor+0x60,4);n.command(dict(command="check_memory",address=actor+0x60,data=before.hex()));n.command(dict(command="write_memory",address=actor+0x60,data=struct.pack(">f",yaw).hex()))
  rows.append(dict(slot=i,actor=hex(actor),before=old,after=dest,yaw=yaw))
 receipt=dict(map=map_name,entity=entity,authored=matches[0],offset=offset,actors=rows,scope='Diagnostic warp; does not prove normal campaign progression or encounter completion')
 path=n.root/'warp-receipts.json';data=json.loads(path.read_text()) if path.exists() else [];data.append(receipt);path.write_text(json.dumps(data,indent=2))
 print(json.dumps(receipt))

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--automation-dir',required=True,type=Path)
 p.add_argument('--wad',required=True,type=Path)
 p.add_argument('--map',required=True)
 p.add_argument('--entity',required=True)
 p.add_argument('--offset',nargs=3,type=float,default=(0,0,0))
 p.add_argument('--yaw',type=float)
 p.add_argument('--inspect',action='store_true',help='Pause and inspect target without moving actors')
 a=p.parse_args();n=Session(a.automation_dir);WAD=a.wad
 if a.inspect:
  n.command(dict(command='pause'))
  print(json.dumps(dict(authored=[v for t,v in map_nodes(a.map) if v.get('name')==a.entity],live=[r for r in live_entities() if r['name']==a.entity]),indent=2))
 else:warp(a.map,a.entity,a.offset,a.yaw)
