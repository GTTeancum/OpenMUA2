#!/usr/bin/env python3
"""Read-only MUA2 Wii-USA binary audit. No guessed REL runtime addresses.

Header layouts and relocation record interpretation follow the uploaded pinned
DolRecomp src/frontend/container/{dol,rel}.{c,h}. Does not patch the game.
"""
from __future__ import annotations
import argparse, collections, hashlib, json, struct
from pathlib import Path

EXPECTED = {
    'sys/main.dol': '0857973ed7646eaf1294981295673243546c935cdbd1fd07345b4d1093c62741',
    'files/Marvel-rev-fin-plf2.rel': '5b739b1046b6987897f078c27f54c214cfe7a1b0381bca0ee29754b57c8a6a7f',
}
RELOC_NAMES = {0:'NONE',1:'ADDR32',2:'ADDR24',3:'ADDR16',4:'ADDR16_LO',5:'ADDR16_HI',6:'ADDR16_HA',7:'ADDR14',8:'ADDR14_BRTAKEN',9:'ADDR14_BRNTAKEN',10:'REL24',11:'REL14',201:'DOLPHIN_NOP',202:'DOLPHIN_SECTION',203:'DOLPHIN_END'}

def be32(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValueError(f'Truncated 32-bit field at {offset:#x}')
    return struct.unpack_from('>I', data, offset)[0]

def checked(data: bytes, offset: int, size: int) -> bytes:
    if offset < 0 or size < 0 or offset > len(data) or size > len(data) - offset:
        raise ValueError(f'Out-of-file range {offset:#x}+{size:#x}')
    return data[offset:offset+size]

def dol_report(data: bytes) -> dict:
    checked(data, 0, 0x100)
    sections=[]
    for i in range(18):
        offset,address,size=(be32(data, base+i*4) for base in (0,0x48,0x90))
        if size:
            checked(data,offset,size)
            if address + size > 0x100000000: raise ValueError('DOL address overflow')
            sections.append(dict(kind='text' if i<7 else 'data',index=i if i<7 else i-7,file_offset=offset,address=address,size=size))
    text=[s for s in sections if s['kind']=='text']
    entry=be32(data,0xe0)
    if not any(s['address'] <= entry < s['address']+s['size'] for s in text):
        raise ValueError('DOL entry is outside executable sections')
    return dict(size=len(data),entry_point=entry,sections=sections,text_bytes=sum(s['size'] for s in text),bss_address=be32(data,0xd8),bss_size=be32(data,0xdc))

def rel_report(data: bytes) -> dict:
    checked(data,0,0x4c)
    module,version,count,table=be32(data,0),be32(data,0x1c),be32(data,0xc),be32(data,0x10)
    if not 1 <= count <= 4096: raise ValueError('Invalid section count')
    checked(data,table,count*8)
    sections=[]
    for i in range(count):
        flags,size=be32(data,table+i*8),be32(data,table+i*8+4)
        offset=flags & ~1
        if offset: checked(data,offset,size)
        sections.append(dict(index=i,file_offset=offset,size=size,executable=bool(flags&1),bss=bool(size and not offset)))
    imp,size=be32(data,0x28),be32(data,0x2c)
    if size%8: raise ValueError('Invalid import table size')
    checked(data,imp,size)
    imports=[]
    for p in range(imp,imp+size,8):
        target_module,stream=be32(data,p),be32(data,p+4)
        source_section=None; source_offset=0; counts=collections.Counter(); destinations=collections.Counter(); operations=0
        for position in range(stream,len(data)-7,8):
            delta,kind,target_section,addend=struct.unpack_from('>HBBI',data,position)
            counts[RELOC_NAMES.get(kind,f'UNKNOWN_{kind}')]+=1
            if kind==203: break
            if kind==202:
                if target_section>=count: raise ValueError('Bad source section switch')
                source_section=target_section; source_offset=0; continue
            source_offset+=delta
            if kind in (0,201): continue
            if kind not in RELOC_NAMES: raise ValueError(f'Unknown relocation type {kind}')
            if source_section is None: raise ValueError('Relocation before section marker')
            width=2 if kind in (3,4,5,6) else 4
            source=sections[source_section]
            if source_offset + width > source['size']: raise ValueError('Relocation write past section end')
            if target_module==module:
                if target_section>=count or addend>sections[target_section]['size']:
                    raise ValueError('Invalid self-relocation target')
            destinations[str(source_section)]+=1; operations+=1
        else: raise ValueError('Relocation stream has no END marker')
        imports.append(dict(module_id=target_module,stream_offset=stream,record_counts=dict(counts),relocations=operations,source_sections=dict(destinations)))
    entries={}
    for name,section_offset,entry_offset in [('prolog',0x30,0x34),('epilog',0x31,0x38),('unresolved',0x32,0x3c)]:
        index=data[section_offset]; offset=be32(data,entry_offset)
        if index and (index>=count or not sections[index]['executable'] or offset>=sections[index]['size'] or offset%4):
            raise ValueError(f'Invalid REL {name}')
        entries[name]=dict(section=index,offset=offset)
    return dict(size=len(data),module_id=module,version=version,section_count=count,section_table_offset=table,sections=sections,bss_size=be32(data,0x20),text_bytes=sum(s['size'] for s in sections if s['executable']),imports=imports,entries=entries,runtime_load_address=None,native_rel_integration_verified=False)

def audit(root: Path) -> dict:
    loaded={}
    for name,digest in EXPECTED.items():
        data=(root/name).read_bytes()
        if hashlib.sha256(data).hexdigest()!=digest: raise ValueError(f'Unsupported or changed executable: {name}')
        loaded[name]=data
    lcf=root/'files/eppcStatic.lcf'
    lines=lcf.read_text(errors='replace').splitlines() if lcf.exists() else []
    # Names are build/link hints only; this file supplies no symbol address map.
    body='\n'.join(lines).partition('FORCEACTIVE')[2]
    forced=[line.strip() for line in body.splitlines() if line.strip() and line.strip() not in ('{','}')]
    return dict(game='Marvel: Ultimate Alliance 2',platform='Wii USA',disc_id=(root/'sys/boot.bin').read_bytes()[:6].decode('ascii'),hashes=EXPECTED,dol=dol_report(loaded['sys/main.dol']),rel=rel_report(loaded['files/Marvel-rev-fin-plf2.rel']),linker_script=dict(path='files/eppcStatic.lcf',forceactive_names=len(forced),address_map=False),asset_files=sum(p.is_file() for p in (root/'files').rglob('*')))

def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game_root',type=Path); parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args(); result=audit(args.game_root)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(f"Verified {result['disc_id']}: {result['dol']['text_bytes']:,} DOL text bytes; {result['rel']['text_bytes']:,} REL text bytes; "+str(sum(i['relocations'] for i in result['rel']['imports']))+' REL relocation operations.')
if __name__=='__main__': main()
