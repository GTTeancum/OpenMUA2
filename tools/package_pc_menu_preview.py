"""Build a private native Options preview retaining MUA2's background branches.

Uses OpenXML1's complete IGB graph-transfer helper. Does not install a WAD or
claim functional PC-options command hooks; this package is for native UI testing.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import xml.etree.ElementTree as ET
import zipfile


def decode_fb(data):
    at = 0
    while at < len(data):
        header = data[at:at+196]
        if len(header) != 196:
            raise ValueError('Truncated FB header')
        size, = struct.unpack_from('<I', header, 192)
        payload = data[at+196:at+196+size]
        if len(payload) != size:
            raise ValueError('Truncated FB payload')
        yield header[:128].split(b'\0')[0].decode(), header[128:192].split(b'\0')[0].decode(), payload
        at += 196 + size


def encode_xml(root):
    nodes = list(root.iter())
    offsets, at = {}, 8
    siblings = {}
    for node in nodes:
        offsets[id(node)] = at
        at += 16 + len(node.attrib)*8
        children = list(node)
        for left, right in zip(children, children[1:]):
            siblings[id(left)] = right
    pool, strings = bytearray(), {}
    def intern(value):
        if '\0' in value:
            raise ValueError('NUL in XML field')
        if value not in strings:
            strings[value] = at+len(pool)
            pool.extend(value.encode('cp1252')+b'\0')
        return strings[value]
    intern(root.tag)
    data = bytearray(struct.pack('<II',0x11b1,1))
    for node in nodes:
        children = list(node)
        sibling = siblings.get(id(node))
        data.extend(struct.pack('<IIII',intern(node.tag),
                                offsets[id(sibling)] if sibling is not None else 0xffffffff,
                                offsets[id(children[0])] if children else 0xffffffff,len(node.attrib)))
        for key,value in node.attrib.items():
            data.extend(struct.pack('<II',intern(key),intern(value)))
    return bytes(data+pool)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--assets',type=Path,required=True)
    p.add_argument('--retail-wad',type=Path,required=True)
    p.add_argument('--writer-root',type=Path,required=True)
    p.add_argument('--reference-scripts',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    repo=Path(__file__).resolve().parents[1]
    assets,wad,addon=(v.resolve(strict=True) for v in (a.assets,a.retail_wad,a.writer_root))
    output=a.output.resolve()
    if any(not v.is_relative_to(repo/'.local') for v in (assets,wad,addon,output)):
        raise ValueError('All game inputs and outputs must be private within this repository')
    owned={'retail-layout.igb','x2m_options.igb','options_rev.fb','preview-manifest.json'}
    if output.exists() and (not output.is_dir() or any(p.name not in owned or not p.is_file() for p in output.iterdir())):
        raise ValueError('Output contains files not owned by this authoring step')
    sys.dont_write_bytecode=True
    sys.path[:0]=[str(addon),str(a.reference_scripts.resolve(strict=True))]
    from igb_format.igb_reader import IGBReader
    from igb_format.igb_writer import from_reader
    import menu_igb_graph
    from menu_igb_graph import field,put,memory,refs
    from pc_menu_geometry import transfer_menu_graph
    from xml1_packages import read_pkgb
    from xbox_tutorials import read_nodes
    output.mkdir(parents=True,exist_ok=True)
    package='packages/generated/maps/package/menus/options_rev.fb'
    with zipfile.ZipFile(wad) as archive:
        original=archive.read(package)
    retail={name:(kind,data) for name,kind,data in decode_fb(original)}
    native_layout=output/'retail-layout.igb'
    native_layout.write_bytes(retail['ui/menus/cw_pausemenu.igb'][1])
    def read(path):
        reader=IGBReader(str(path));reader.read();return from_reader(reader)
    native=read(native_layout)
    panel=read(assets/'ui/menus/x2m_options.igb')
    if panel.version != native.version or panel.endian != native.endian:
        raise ValueError('Convert the PC scene to the native MUA2 IGB format before packaging')
    def named(writer,name):
        found=[i for i,o in enumerate(writer.objects) if hasattr(o,'raw_fields') and
               writer.meta_objects[o.type_index].name in (b'igTransform',b'igGroup') and
               any(k==2 and v==name for k,v,_ in o.raw_fields)]
        if len(found)!=1:raise ValueError(f'Expected one scene anchor named {name}')
        return found[0]
    branches=('background_red','background_white','Stars_And_Stripes','Star_Circle01','Star_Circle02')
    native_root=field(native,named(native,'Scene Root'),7)
    native_children=refs(native,field(native,native_root,4))
    panel_root=field(panel,named(panel,'Scene Root'),7)
    children=refs(panel,field(panel,panel_root,4))
    for name in branches:
        anchor=named(native,name)
        if anchor not in native_children:raise ValueError('Background anchor needs ancestor transform')
        if any(hasattr(o,'raw_fields') and any(k==2 and v==name for k,v,_ in o.raw_fields)
               for o in panel.objects):raise ValueError('Duplicate background anchor')
        children.append(transfer_menu_graph(panel,native,anchor,menu_igb_graph))
    memory(panel,field(panel,panel_root,4),struct.pack('<'+'i'*len(children),*children))
    put(panel,panel_root,2,len(children));put(panel,panel_root,3,len(children))
    layout=output/'x2m_options.igb';panel.write(str(layout))
    reread=read(layout)
    for name in branches:named(reread,name)

    menu=ET.parse(assets/'ui/menus/options.eng').getroot()
    menu.set('name','options_rev');menu.set('no_stack','true')
    styles={'STYLE_TITLE':'STYLE_TITLE_MED',
            'STYLE_MENU':'STYLE_SEL_SMALL_WHITE','STYLE_SMALL':'SMALL_WHITE'}
    for node in menu.iter():
        if node.get('style') in styles:
            node.set('style',styles[node.get('style')])
    items={item.get('name'):item for item in menu.findall('item')}
    for item in menu.findall('item'):
        if item.get('type')=='MENU_ITEM_SLIDER':
            # MUA2 uses an ordinary focusable label with increment/decrement
            # commands and a separate gamevar-driven model for its volume bar.
            variable=item.attrib.pop('gamevar')
            slider=items[item.attrib.pop('slidermodel')]
            item.attrib.pop('type')
            item.set('left',item.get('name'));item.set('right',item.get('name'))
            item.set('leftcmd','setdecrement '+variable)
            item.set('rightcmd','setincrement '+variable)
            slider.set('gamevar',variable)
            slider.set('model','ui/models/model_bar_sound')
            slider.set('neverfocus','true')
    native_nodes=read_nodes(retail['ui/menus/options_rev.engb'][1])
    background=[]
    for tag,attrs,_,_ in native_nodes:
        if tag=='item' and attrs.get('name') in branches:
            # Native anchor animation and model rotation are retained. The
            # original AT_bgelements text-opacity track is copied below.
            background.append(ET.Element(tag,attrs))
    for node in reversed(background):menu.insert(0,node)
    # Preserve the original background opacity timeline with its marks.
    native_bytes=retail['ui/menus/options_rev.engb'][1]
    def element(offset):
        name,sibling,child,count=struct.unpack_from('<4I',native_bytes,offset)
        def text(pos):return native_bytes[pos:native_bytes.index(0,pos)].decode('cp1252')
        node=ET.Element(text(name))
        for i in range(count):
            key,value=struct.unpack_from('<2I',native_bytes,offset+16+i*8)
            node.set(text(key),text(value))
        while child!=0xffffffff:
            node.append(element(child));child=struct.unpack_from('<I',native_bytes,child+4)[0]
        return node
    for node in element(8):
        if node.tag=='animtext' and node.get('name')=='AT_bgelements':menu.insert(0,node)
    compiled=encode_xml(menu);read_nodes(compiled)
    resources={}
    definition=assets/'packages/generated/maps/package/menus/options.pkgb'
    for row in read_pkgb(definition.read_bytes()):
        name=dict(row.attributes)['filename']
        if row.kind=='model':
            relative=name+'.igb';resources[relative]=('model',(assets/relative).read_bytes())
    resources['ui/menus/x2m_options.igb']=('model',layout.read_bytes())
    for node in background:
        name=node.get('model').lower()+'.igb'
        resources[name]=retail[name]
    for extension in ('engb','itab'):
        resources[f'ui/menus/options_rev.{extension}']=('xml',compiled)
    result=bytearray()
    for name,(kind,data) in resources.items():
        result.extend(name.encode().ljust(128,b'\0')+kind.encode().ljust(64,b'\0')+struct.pack('<I',len(data))+data)
    target=output/'options_rev.fb';target.write_bytes(result)
    if len(list(decode_fb(result)))!=len(resources):raise ValueError('FB verification failed')
    (output/'preview-manifest.json').write_text(json.dumps({'package':package,
        'source_sha256':hashlib.sha256(original).hexdigest(),'output_sha256':hashlib.sha256(result).hexdigest(),
        'native_backgrounds':list(branches),'resources':list(resources),'native_rendering_verified':False},indent=2)+'\n')
    print(f'Prepared native Options preview with {len(resources)} resources and five original background branches')


if __name__=='__main__':main()
