"""Convert legacy menu meshes to native Wii typed arrays, retaining scene graphs.

Uses igb-blender graph objects and the reference graph copier. Native static UI
meshes supply the component schemas; no skeleton, weights or character data are
introduced. Preserve triangle-strip ordering, including degenerate connectors.
"""
import copy
import struct


def transfer_menu_graph(destination, source, index, graph):
    source = copy.deepcopy(source)
    # Directory names and memory-pool handles belong to source-local tables,
    # independently of the object/meta-field indices handled by the graph copier.
    for i, info in enumerate(source.ref_info):
        pool = info.get('mem_pool_handle', -1)
        new_pool = -1
        if pool >= 0:
            name = source.memory_pool_names[pool]
            if name not in destination.memory_pool_names:
                destination.memory_pool_names.append(name)
            new_pool = destination.memory_pool_names.index(name)
        entry = copy.deepcopy(source.entries[source.index_map[i]])
        entry.raw_bytes = None
        for n, definition in enumerate(source.meta_objects[entry.type_index].fields):
            if definition.short_name == b'String':
                old_name = entry.field_values[n]
                if not 0 <= old_name < len(source.name_pool):
                    raise ValueError('Source directory name is outside its name pool')
                entry.field_values[n] = destination._intern_name(source.name_pool[old_name])
            if definition.slot == (12 if info['is_object'] else 13):
                entry.field_values[n] = new_pool
        source.index_map[i] = len(source.entries)
        source.entries.append(entry)
        info['mem_pool_handle'] = new_pool
    return graph.transfer_graph(destination, source, index)


def convert_geometry(writer, native_root, graph, reader_type, from_reader):
    field, put, memory, refs = graph.field, graph.put, graph.memory, graph.refs

    def kind(w, i):
        obj = w.objects[i]
        return w.meta_objects[obj.type_index].name if hasattr(obj, 'raw_fields') else b''

    def find(w, name):
        return [i for i in range(len(w.objects)) if kind(w, i) == name]

    def load(name):
        reader = reader_type(str(native_root/name)); reader.read()
        return from_reader(reader)

    def transfer(destination, source, index):
        return transfer_menu_graph(destination, source, index, graph)

    def set_list(w, index, count, data):
        memory(w, field(w, index, 4), data)
        put(w, index, 2, count); put(w, index, 3, count)

    old_geometry = find(writer, b'igGeometryAttr1_5')
    if not old_geometry:
        return writer, 0
    textured = load('cw_bg_red.igb')
    colored = load('cw_options_select.igb')
    native_color = next(i for i in find(colored, b'igVertexData') if field(colored, i, 4) == 2)
    replacements = {}
    for old in old_geometry:
        if field(writer, old, 6) != 4:
            raise ValueError('Only verified menu triangle strips are supported')
        va = field(writer, old, 4)
        count, flags = field(writer, va, 3), field(writer, va, 6)
        if flags & ~0x10007:
            raise ValueError('Unexpected skinned or multi-UV menu geometry')
        slots = refs(writer, field(writer, va, 2))
        ia = field(writer, old, 5)
        if ia >= 0:
            if field(writer, ia, 4) != 0:
                raise ValueError('Unsupported menu index representation')
            raw = writer.objects[field(writer, ia, 2)].data
            order = list(struct.unpack('<'+'H'*field(writer, ia, 3), raw))
        else:
            order = list(range(count))
        if not order or min(order) < 0 or max(order) >= count:
            raise ValueError('Menu index exceeds source vertices')
        length_array = field(writer, old, 13)
        lengths = bytes(writer.objects[field(writer, length_array, 2)].data)
        if len(lengths) != field(writer, length_array, 3)*4:
            raise ValueError('Unexpected primitive-length representation')
        if sum(x[0] for x in struct.iter_unpack('<I', lengths)) != len(order):
            raise ValueError('Menu strip lengths do not cover all indices')
        donor = copy.deepcopy(textured)
        ga = find(donor, b'igGeometryAttr2')[0]
        native_va = field(donor, ga, 4)
        component_list = field(donor, native_va, 3)
        by_type = {field(donor, i, 4):i for i in find(donor, b'igVertexData')}
        components = []
        # old stream slot, format flag, native semantic, byte stride
        for slot, flag, semantic, stride in ((0,1,1,12),(1,2,3,12),(2,4,2,4),(11,0x10000,4,8)):
            if not flags & flag:
                continue
            if slot >= len(slots) or slots[slot] < 0:
                raise ValueError('Declared vertex component has no stream')
            raw = bytes(writer.objects[slots[slot]].data)
            if len(raw) != count*stride:
                raise ValueError('Vertex stream byte count differs from declared count')
            expanded = b''.join(raw[i*stride:(i+1)*stride] for i in order)
            if semantic == 2:
                component = transfer(donor, colored, native_color)
            elif semantic == 3:
                component = transfer(donor, donor, by_type[1])
                put(donor, component, 4, 3)
            else:
                component = by_type[semantic]
            set_list(donor, field(donor, component, 3), len(order), expanded)
            components.append(component)
        set_list(donor, component_list, len(components), struct.pack('<'+'i'*len(components), *components))
        native_lengths = field(donor, ga, 6)
        memory(donor, field(donor, native_lengths, 2), lengths)
        put(donor, native_lengths, 3, len(lengths)//4)
        put(donor, ga, 8, len(lengths)//4)
        replacements[old] = transfer(writer, donor, ga)

    def object_list(w, obj):
        index = obj.type_index
        while index >= 0:
            meta = w.meta_objects[index]
            if meta.name == b'igObjectList':
                return True
            index = meta.parent_index
        return False

    for i, obj in enumerate(writer.objects):
        if not hasattr(obj, 'raw_fields'):
            continue
        if object_list(writer, obj) and field(writer, i, 4) >= 0:
            block = field(writer, i, 4)
            old_refs = refs(writer, block)
            new_refs = [replacements.get(x, x) for x in old_refs]
            if new_refs != old_refs:
                memory(writer, block, struct.pack('<'+'i'*len(new_refs), *new_refs))
        for slot, value, definition in list(obj.raw_fields):
            if definition.short_name == b'ObjectRef' and value in replacements:
                put(writer, i, slot, replacements[value])
    # Alchemy loads every serialized object, including unreferenced legacy ones.
    # Copy the reachable graph so discarded vertex arrays cannot be instantiated.
    compact = copy.deepcopy(writer)
    compact.objects=[]; compact.entries=[]; compact.index_map=[]; compact.ref_info=[]
    compact.info_list_index=transfer(compact, writer, writer.info_list_index)
    if find(compact, b'igGeometryAttr1_5') or find(compact, b'igVertexArray1_1'):
        raise ValueError('Legacy geometry remains reachable')
    return compact, len(old_geometry)
