#!/usr/bin/env python3
"""Static scan of the exact original MUA2 DOL; not a gameplay verification."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('dol',type=Path)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args(); data=a.dol.read_bytes()
expected='0857973ed7646eaf1294981295673243546c935cdbd1fd07345b4d1093c62741'
if hashlib.sha256(data).hexdigest()!=expected: p.error('Not the recorded original RMSE52 DOL')
ranges=[]
for i in range(7):
    off=struct.unpack_from('>I',data,4*i)[0]
    addr=struct.unpack_from('>I',data,0x48+4*i)[0]
    size=struct.unpack_from('>I',data,0x90+4*i)[0]
    if off+size>len(data): p.error('Invalid DOL section range')
    if size: ranges.append((addr,off,size))
def word_at(addr):
    for start,offset,size in ranges:
        if start<=addr and addr+4<=start+size:
            return struct.unpack_from('>I',data,offset+addr-start)[0]
    raise ValueError(f'Address outside original text: {addr:08X}')
found=[]
for addr in range(0x80388908,0x80388b00,4):
    word=word_at(addr)
    xo=(word>>1)&1023
    if word>>26==63 and xo in (14,15):
        found.append({'pc':f'{addr:08X}','instruction_word':f'{word:08X}',
                      'opcode':'fctiwz' if xo==15 else 'fctiw',
                      'destination_fpr':(word>>21)&31,'input_fpr':(word>>11)&31})
report={'source':'Original RMSE52 DOL; static inspection only, not MG02 runtime replay',
        'dol_sha256':expected,'dol_size':len(data),'historical_mg02_entry':'0x80388908',
        'historical_mg02_end':'0x80388B00','conversion_instructions_in_range':found,
        'interpretation':'This historical mismatch range contains fctiwz instructions. This does not identify the MG02 executed path or operands or prove the live mismatch is fixed.',
        'native_gameplay_retested':False}
a.output.write_text(json.dumps(report,indent=2)+'\n')
print(f'{len(found)} conversion instructions; static audit only.')
