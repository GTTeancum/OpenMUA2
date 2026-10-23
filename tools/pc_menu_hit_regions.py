"""Author invisible native mouse regions from each control's selection model.

Text placeholders are not hit boxes: their bounds do not grow when native
gamevars replace the string. Keep text/model rendering untouched and add a
separate scene anchor, transformed exactly like the existing focus panel.
"""
import copy
import struct
import xml.etree.ElementTree as ET


def add_hit_regions(scene, menu, assets, read, graph):
    field, put, memory, refs = graph.field, graph.put, graph.memory, graph.refs

    def kind(w, index):
        return w.meta_objects[w.objects[index].type_index].name

    def clone(index):
        result = len(scene.objects)
        scene.objects.append(copy.deepcopy(scene.objects[index]))
        scene.ref_info.append(copy.deepcopy(scene.ref_info[index]))
        scene.index_map.append(scene.index_map[index])
        return result

    transforms = {field(scene, i, 2): i for i, o in enumerate(scene.objects)
                  if hasattr(o, 'raw_fields') and kind(scene, i) == b'igTransform'}
    root = next(i for i, o in enumerate(scene.objects) if hasattr(o, 'raw_fields')
                and kind(scene, i) in (b'igTransform', b'igGroup')
                and field(scene, i, 2) == 'Scene Root')
    root_list = field(scene, root, 7)
    root_memory = field(scene, root_list, 4)
    children = refs(scene, root_memory)
    regions = []
    models = {}
    for item in list(menu.findall('item')):
        if item.get('neverfocus') == 'true' or not any(item.get(k) for k in ('usecmd', 'leftcmd', 'rightcmd')):
            continue
        focus_name, model_name = item.get('focusitemname'), item.get('focusmodel')
        if not focus_name or not model_name:
            continue
        anchor = transforms[focus_name]
        if anchor not in children:
            raise ValueError('Focus anchor requires an ancestor transform: ' + focus_name)
        if model_name not in models:
            model = read(assets / (model_name.lower() + '.igb'))
            scene_info = next(i for i, o in enumerate(model.objects)
                              if hasattr(o, 'raw_fields') and kind(model, i) == b'igSceneInfo')
            model_root = field(model, scene_info, 5)
            box = field(model, model_root, 3)
            if kind(model, box) != b'igAABox':
                raise ValueError('Selection model has no authored bounding box')
            models[model_name] = (field(model, box, 2), field(model, box, 3))
        # Clone the anchor and its user-info chain, not any visible model.
        infos = refs(scene, field(scene, field(scene, anchor, 7), 4))
        if len(infos) != 1 or kind(scene, infos[0]) != b'igHashedUserInfo':
            raise ValueError('Unexpected focus anchor structure: ' + focus_name)
        node, nodes, data = clone(anchor), clone(field(scene, anchor, 7)), clone(field(scene, field(scene, anchor, 7), 4))
        info = clone(infos[0])
        box = clone(field(scene, info, 3))
        properties = clone(field(scene, info, 8))
        property_data = clone(field(scene, properties, 4))
        property_refs = refs(scene, property_data)
        names = [p for p in property_refs if field(scene, field(scene, p, 2), 2) == 'ItemName']
        if len(names) != 1:
            raise ValueError('Expected one ItemName property on the focus anchor')
        prop = clone(names[0])
        value = clone(field(scene, prop, 3))
        name = 'pc_hit_' + item.get('name')
        put(scene, node, 2, name); put(scene, node, 7, nodes)
        put(scene, nodes, 4, data); memory(scene, data, struct.pack('<i', info))
        put(scene, info, 2, name); put(scene, info, 3, box); put(scene, info, 8, properties)
        put(scene, box, 2, models[model_name][0]); put(scene, box, 3, models[model_name][1])
        put(scene, properties, 2, 1); put(scene, properties, 3, 1)
        put(scene, properties, 4, property_data); memory(scene, property_data, struct.pack('<i', prop))
        put(scene, prop, 3, value); put(scene, value, 2, name)
        children.append(node)
        ET.SubElement(menu, 'item', name=name, text='', style='STYLE_SEL_SMALL_WHITE', neverfocus='true')
        regions.append({'item': item.get('name'), 'anchor': name, 'source': focus_name, 'model': model_name})
    if len(menu.findall('item')) > 320:
        raise ValueError('Mouse regions exceed the native expanded menu capacity')
    memory(scene, root_memory, struct.pack('<' + 'i' * len(children), *children))
    put(scene, root_list, 2, len(children)); put(scene, root_list, 3, len(children))
    return regions
