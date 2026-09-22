#!/usr/bin/env python3
"""Compare original REL relocations to a read-only guest RAM capture.

This is an analysis tool, not a replacement linker or a native REL module.
The game supplies the observed section addresses, including its BSS placement.
"""
from __future__ import annotations
import argparse, collections, hashlib, json, struct
from pathlib import Path
from audit_game import be32, checked, rel_report

class Memory:
    def __init__(self, folder: Path):
        self.chunks = []
        for p in sorted(folder.glob('memory-????????.bin')):
            self.chunks.append((int(p.stem.split('-')[1], 16), p.read_bytes()))
        if not self.chunks:
            raise ValueError('No memory-XXXXXXXX.bin captures found')
        for (a,b),(c,d) in zip(self.chunks,self.chunks[1:]):
            if a+len(b)>c: raise ValueError('Overlapping memory captures')
    def read(self, address: int, size: int) -> bytes:
        if address<0 or size<0 or address+size>0x100000000:
            raise ValueError('Invalid address range')
        out=bytearray()
        for base,data in self.chunks:
            if base <= address < base+len(data):
                count=min(size-len(out),base+len(data)-address)
                out.extend(data[address-base:address-base+count]); address+=count
                if len(out)==size: return bytes(out)
        if size==0: return b''
        raise ValueError(f'Uncaptured memory range ending at {address:#x}')

def patch(section: bytearray, offset: int, kind: int, target: int,
          patch_address: int, halfword_adjust: int=0) -> None:
    if kind in (3,4,5,6):
        start=offset+halfword_adjust; width=2
    else: start=offset; width=4
    checked(section,start,width)
    if kind==1: struct.pack_into('>I',section,start,target)
    elif kind in (3,4,5,6):
        value=target if kind in (3,4) else target>>16 if kind==5 else (target+0x8000)>>16
        struct.pack_into('>H',section,start,value&0xffff)
    elif kind==10:
        delta=target-patch_address
        if delta%4 or not -0x2000000<=delta<0x2000000:
            raise ValueError('REL24 target out of range')
        struct.pack_into('>I',section,start,(be32(section,start)&0xfc000003)|(delta&0x03fffffc))
    else: raise ValueError(f'Unsupported observed relocation type {kind}')

def audit(rel_data: bytes, memory: Memory, base: int) -> dict:
    original=rel_report(rel_data); header=memory.read(base,0x4c)
    for off,key in ((0,'module_id'),(0x1c,'version'),(0xc,'section_count')):
        if be32(header,off)!=original[key]: raise ValueError(f'Live header {key} mismatch')
    ptr=be32(header,0x10)
    # A linked OS module stores an absolute section-table pointer. The other
    # case permits an unlinked capture; neither is guessed from memory scans.
    table_addr=base+ptr if ptr==original['section_table_offset'] else ptr
    table=memory.read(table_addr,original['section_count']*8)
    layout=[]
    for s in original['sections']:
        i=s['index']; flags=be32(table,i*8); size=be32(table,i*8+4)
        if size!=s['size'] or bool(flags&1)!=s['executable']:
            raise ValueError(f'Live section {i} layout mismatch')
        addr=flags&~1
        if size: memory.read(addr,size)
        layout.append(dict(s,address=addr))
    variants={a:{s['index']:bytearray(checked(rel_data,s['file_offset'],s['size']))
                  for s in layout if s['file_offset'] and s['size']} for a in (0,2)}
    counts=collections.Counter(); halfword_alignment=collections.Counter()
    for imp in original['imports']:
        mod=imp['module_id']; section=None; offset=0
        for pos in range(imp['stream_offset'],len(rel_data)-7,8):
            delta,kind,target_section,addend=struct.unpack_from('>HBBI',rel_data,pos)
            if kind==203: break
            if kind==202: section=target_section; offset=0; continue
            offset+=delta
            if kind in (0,201): continue
            if section not in variants[0]: raise ValueError('Relocation writes to absent/BSS section')
            if mod==0: target=addend
            elif mod==original['module_id']:
                target=layout[target_section]['address']+addend
            else: raise ValueError(f'No observed mapping for external module {mod}')
            if target>0xffffffff: raise ValueError('Relocation address overflow')
            if kind in (3,4,5,6): halfword_alignment[str(offset%4)]+=1
            counts[str(kind)]+=1
            for adjust,buffers in variants.items():
                patch(buffers[section],offset,kind,target,layout[section]['address']+offset,adjust)
    comparisons=[]
    for adjust,buffers in variants.items():
        rows=[]
        for s in layout:
            if not s['executable'] or not s['size']: continue
            expected=bytes(buffers[s['index']]); observed=memory.read(s['address'],s['size'])
            mismatches=[i for i,(a,b) in enumerate(zip(expected,observed)) if a!=b]
            rows.append(dict(section=s['index'],address=s['address'],bytes=len(expected),
                expected_sha256=hashlib.sha256(expected).hexdigest(),observed_sha256=hashlib.sha256(observed).hexdigest(),
                mismatch_bytes=len(mismatches),first_mismatch_offsets=mismatches[:16]))
        comparisons.append(dict(halfword_write_adjust=adjust,text_sections=rows,
            mismatch_bytes=sum(r['mismatch_bytes'] for r in rows)))
    normal=next(c for c in comparisons if c['halfword_write_adjust']==0)
    return dict(status='LIVE_TEXT_MATCH' if normal['mismatch_bytes']==0 else 'LIVE_TEXT_MISMATCH',
        source_rel_sha256=hashlib.sha256(rel_data).hexdigest(),module_id=original['module_id'],
        observed_rel_base=base,live_section_table_field=ptr,section_table_address=table_addr,
        section_table_is_absolute=ptr==table_addr,live_import_table_size=be32(header,0x2c),
        live_fix_size_field=be32(header,0x48),original_fix_size=be32(rel_data,0x48),
        original_module_alignment=be32(rel_data,0x40),original_bss_alignment=be32(rel_data,0x44),
        layout=layout,relocations=sum(counts.values()),relocation_type_counts=dict(counts),
        halfword_patch_offsets_mod4=dict(halfword_alignment),comparisons=comparisons,
        native_rel_built=False,
        limitation='Read-only one-boot layout/text validation; no claim of a generic linker or native REL execution.')

def main() -> None:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rel',required=True,type=Path); p.add_argument('--memory',required=True,type=Path)
    p.add_argument('--base',required=True,type=lambda s:int(s,0)); p.add_argument('--output',required=True,type=Path)
    args=p.parse_args(); result=audit(args.rel.read_bytes(),Memory(args.memory),args.base)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(result['status'], 'relocations=',result['relocations'],
          'mismatch_bytes_by_halfword_adjust=',{c['halfword_write_adjust']:c['mismatch_bytes'] for c in result['comparisons']})
    if result['status']!='LIVE_TEXT_MATCH': raise SystemExit(1)
if __name__=='__main__': main()
