#pragma once
#include "mua2_action_bindings.hpp"
#include <bit>
#include <cmath>
#include <tuple>

namespace moderngekko::controls {
// Reads only. The caller must also verify executable/evaluator identity.
// Reject unknown objects, stale handles, ambiguous ownership and invalid ranges.
template <typename Reader>
bool HasCoopInteraction(Reader read, unsigned port) {
  if (port >= 4) return false;
  const auto ram = [&](std::uint32_t address, std::size_t size) {
    const auto end=std::uint64_t(address)+size;
    if ((address>=0x80000000 && end<=0x81800000) ||
        (address>=0x90000000 && end<=0x94000000)) return read(address,size);
    return std::span<const std::uint8_t>{};
  };
  auto global=ram(0x80817368,4);
  if (global.size()!=4) return false;
  const auto manager_address=ReadBE(global,0);
  if (manager_address&3) return false;
  auto manager=ram(manager_address,0x1400);
  if (manager.size()!=0x1400 || ReadBE(manager,0)!=0x81177d08 ||
      ReadBE(manager,0x13fc)!=0x1ff) return false;
  const auto live=[&](unsigned i) {
    return (ReadBE(manager,0xd30+4*(i/32)) & (1u<<(i%32)))!=0;
  };
  const auto resolve=[&](std::uint32_t handle) -> std::uint32_t {
    const unsigned index=handle&0x1ff;
    if (!handle || index>=420 || !live(index) ||
        ReadBE(manager,0xd6c+4*index)!=handle) return 0;
    const auto address=ReadBE(manager,4+4*index);
    return (address&3) ? 0 : address;
  };
  unsigned owners=0;
  bool eligible=false;
  for (unsigned i=0;i<420;++i) {
    if (!live(i)) continue;
    const auto handle=ReadBE(manager,0xd6c+4*i);
    if ((handle&0x1ff)!=i) return false;
    const auto address=resolve(handle);
    if (!address || std::uint64_t(address)+0xa38>0xffffffffu) return false;
    auto type=ram(address+0x9c,4);
    if (type.size()!=4) return false;
    if (ReadBE(type,0)!=0x8052a748) continue; // CActor
    auto actor=ram(address,0xa38);
    if (actor.size()!=0xa38 || ReadBE(actor,0x48)!=handle) return false;
    if ((actor[0x4d0]&0x80) || ReadBE(actor,0x45c)!=port) continue;
    if (++owners!=1) return false;
    const float health=std::bit_cast<float>(ReadBE(actor,0x2a0));
    if (!std::isfinite(health) || health<=0) continue;
    const auto node_address=ReadBE(actor,0x3a8);
    if (node_address&3) continue;
    auto node=ram(node_address,0x10);
    if (node.size()!=0x10 || ReadBE(node,0)!=0x811b0620) continue; // CCombatNode
    auto strings=ram(0x805f8828,0x4008);
    if (strings.size()!=0x4008) continue;
    const auto generation=ReadBE(strings,0), tag=ReadBE(node,0xc);
    const auto index=tag&0xffffff;
    if (!generation || generation>255 || (tag>>24)!=generation || index>=4096) continue;
    const auto offset=ReadBE(strings,4+index*4);
    const auto name_address=std::uint64_t(0x805f8828)+0x4008+offset;
    if (name_address>0xffffffffu) continue;
    constexpr std::string_view name="generic_sequence";
    auto text=ram(static_cast<std::uint32_t>(name_address),name.size()+1);
    if (text.size()!=name.size()+1 || text.back()!=0 ||
        !std::equal(name.begin(),name.end(),text.begin())) continue;
    const auto target_handle=ReadBE(actor,0x6e8);
    const auto target_address=resolve(target_handle);
    if (!target_address || target_address==address) continue;
    auto target=ram(target_address,0xa0);
    if (target.size()!=0xa0 || ReadBE(target,0x48)!=target_handle ||
        ReadBE(target,0x9c)!=0x811769e0) continue; // CCoopEntity
    eligible=true;
  }
  return owners==1 && eligible;
}

// Convert existing light/heavy actions only within a verified interaction.
// Repeated physical presses remain repeated presses; no automatic progress.
inline bool ApplyInteractionButtons(bool context, std::span<const std::uint8_t> table,
                                   std::span<std::uint8_t> active,
                                   std::span<std::uint8_t> values) {
  if (!context || table.size()!=DescriptorTableSize || active.size()!=20 ||
      values.size()!=124*4) return false;
  for (const auto [id,name,selector] : {
      std::tuple{56u,std::string_view("ShakeGesture"),256u},
      std::tuple{58u,std::string_view("LiftGesture"),259u}}) {
    const auto d=table.subspan(id*DescriptorSize,DescriptorSize);
    if (ReadBE(d,0)!=id || !std::equal(name.begin(),name.end(),d.begin()+4) ||
        d[4+name.size()]!=0 || ReadBE(d,0x44)!=1 || ReadBE(d,0x48)!=selector ||
        ReadBE(d,0x4c)!=0 || ReadBE(d,0x50)!=0 || ReadBE(d,0x54)!=0x3e19999a)
      return false;
  }
  auto low=ReadBE(active,0), high=ReadBE(active,4);
  const bool light=(low&(1u<<9))!=0, heavy=(low&(1u<<10))!=0;
  if ((light && heavy) || (light && ReadBE(values,9*4)!=0x3f800000) ||
      (heavy && ReadBE(values,10*4)!=0x3f800000)) return false;
  const auto put=[](std::span<std::uint8_t> b,std::size_t p,std::uint32_t v) {
    for (unsigned i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-8*i));
  };
  low&=~((1u<<9)|(1u<<10));
  high&=~((1u<<(56-32))|(1u<<(58-32)));
  if (light) high|=1u<<(56-32);
  if (heavy) high|=1u<<(58-32);
  put(active,0,low); put(active,4,high);
  put(values,9*4,0); put(values,10*4,0);
  put(values,56*4,light?0x3f800000:0); put(values,58*4,heavy?0x3f800000:0);
  return true;
}
}
