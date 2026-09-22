#!/usr/bin/env python3
"""Enable the pinned runtime's Linux EGL context and scope its one feature macro.

The pinned RecompCore uses EGL, not GLX, to create its Linux OpenGL context.
Only Common/GL/GLContext.cpp consumes HAVE_EGL in this source snapshot.
"""
import argparse
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('moderngekko',type=Path)
a=p.parse_args()
changes=[
(a.moderngekko/'CMakeLists.txt',
 '    set(ENABLE_EGL OFF CACHE BOOL "" FORCE)',
 '''    # RecompCore's Linux OpenGL context factory requires EGL.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(ENABLE_EGL ON CACHE BOOL "" FORCE)
    else()
        set(ENABLE_EGL OFF CACHE BOOL "" FORCE)
    endif()'''),
(a.moderngekko/'vendor/dolphin/CMakeLists.txt',
 '    add_definitions(-DHAVE_EGL=1)',
 '    # HAVE_EGL is scoped to its consumer, Common/GL/GLContext.cpp.'),
(a.moderngekko/'vendor/dolphin/Source/Core/Common/CMakeLists.txt',
 'if(ENABLE_EGL AND EGL_FOUND)\n  target_sources(common PRIVATE',
 '''if(ENABLE_EGL AND EGL_FOUND)
  # Avoid changing every target's compiler command for a context-factory flag.
  set_property(SOURCE GL/GLContext.cpp APPEND PROPERTY COMPILE_DEFINITIONS HAVE_EGL=1)
  target_sources(common PRIVATE''')]
# Validate all files before writing any of them.
pending=[]
for path,old,new in changes:
 s=path.read_text()
 if new in s: continue
 if s.count(old)!=1: raise SystemExit(f'Unexpected source; refusing patch: {path}')
 pending.append((path,s.replace(old,new)))
for path,s in pending:
 path.write_bytes(s.replace('\n','\r\n').encode() if b'\r\n' in path.read_bytes() else s.encode())
print('Linux EGL build fix applied.' if pending else 'Linux EGL build fix already present.')
