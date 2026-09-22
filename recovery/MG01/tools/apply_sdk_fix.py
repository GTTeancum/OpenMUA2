#!/usr/bin/env python3
"""Apply the observed private-SDK hidapi include fix to the pinned ModernGekko tree."""
import argparse
from pathlib import Path
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('moderngekko', type=Path)
a = p.parse_args()
path = a.moderngekko/'CMakeLists.txt'
old = '''        target_include_directories(common SYSTEM PRIVATE "${_moderngekko_sdk_usr}/include")
        if(TARGET videovulkan)'''
new = '''        target_include_directories(common SYSTEM PRIVATE "${_moderngekko_sdk_usr}/include")
        # hidapi's Linux wrapper links udev by name and does not inherit its
        # pkg-config include path. Keep the private frontend SDK self-contained.
        if(TARGET hidapi)
            target_include_directories(hidapi SYSTEM PRIVATE "${_moderngekko_sdk_usr}/include")
        endif()
        if(TARGET videovulkan)'''
s = path.read_text()
if new in s:
    print('SDK include fix already present.')
elif s.count(old) == 1:
    result=s.replace(old, new)
    path.write_bytes(result.replace('\n', '\r\n').encode() if b'\r\n' in path.read_bytes() else result.encode())
    print('Applied private-SDK hidapi include fix.')
else:
    raise SystemExit('Unexpected CMake source; refusing an unverified patch.')
