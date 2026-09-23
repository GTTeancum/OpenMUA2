#!/usr/bin/env python3
from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    p = Path(path)
    data = p.read_bytes()
    newline = b"\r\n" if b"\r\n" in data else b"\n"
    text_newline = "\r\n" if newline == b"\r\n" else "\n"
    old_bytes = old.replace("\n", text_newline).encode("utf-8")
    new_bytes = new.replace("\n", text_newline).encode("utf-8")
    count = data.count(old_bytes)
    if count != 1:
        raise RuntimeError(f"{path}: expected one match, found {count}")
    p.write_bytes(data.replace(old_bytes, new_bytes, 1))


cpu_h = "project/lib/ModernGekko/vendor/dolphin/GXRuntime/include/core/cpu.h"
replace_once(
    cpu_h,
    """#define GC_MAIN_RAM_SIZE    (24 * 1024 * 1024)
#define GC_RAM_BASE         0x80000000u
#define GC_RAM_UNCACHED     0xC0000000u
""",
    """#define GC_MAIN_RAM_SIZE    (24 * 1024 * 1024)
#define GC_RAM_BASE         0x80000000u
#define GC_RAM_UNCACHED     0xC0000000u

/* Debug journal keys use physical RAM offsets: MEM1 starts at 0, MEM2 at 0x10000000. */
#define PPC_MEM_JOURNAL_EXRAM_BASE 0x10000000u
""",
)
replace_once(
    cpu_h,
    """        if (offset <= cpu->exram_size - size) {
            if (out_offset) *out_offset = (u32)-1;
            return cpu->exram + offset;
        }
""",
    """        if (offset <= cpu->exram_size - size) {
            if (out_offset) *out_offset = PPC_MEM_JOURNAL_EXRAM_BASE + offset;
            return cpu->exram + offset;
        }
""",
)

cpu_c = "project/lib/ModernGekko/vendor/dolphin/GXRuntime/src/core/cpu.c"
replace_once(
    cpu_c,
    """ * NULL and zero-cost unless installed; the chassis resolves the setter by name
 * via dlsym, so its absence simply disables lockstep. `offset` is the RAM byte
 * offset (into cpu->ram) about to be written, `size` the width in bytes. */
""",
    """ * NULL and zero-cost unless installed; the chassis resolves the setter by name
 * via dlsym, so its absence simply disables lockstep. `offset` is a physical
 * RAM journal key: MEM1 uses its byte offset from 0, while MEM2 uses
 * PPC_MEM_JOURNAL_EXRAM_BASE plus its EXRAM byte offset. `size` is the width. */
""",
)

tramp = "project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompLockstep_Trampolines.cpp"
replace_once(
    tramp,
    """void StaticRecompLockstepVerifier::LsJournalTrampoline(u32 offset, u32 size, void* user)
{
  auto* verifier = static_cast<StaticRecompLockstepVerifier*>(user);
  const u8* ram = verifier->m_core.m_guest.ram;
  const u32 ram_size = verifier->m_core.m_guest.ram_size;
  for (u32 i = 0; i < size; ++i)
  {
    const u32 off = offset + i;
    if (off >= ram_size)
      break;
    verifier->m_journal.ram_pre.emplace(off, ram[off]);
  }
}
""",
    """void StaticRecompLockstepVerifier::LsJournalTrampoline(u32 offset, u32 size, void* user)
{
  auto* verifier = static_cast<StaticRecompLockstepVerifier*>(user);
  if (offset >= PPC_MEM_JOURNAL_EXRAM_BASE)
  {
    auto& memory = verifier->m_core.m_system.GetMemory();
    const u8* exram = memory.GetEXRAM();
    const u32 exram_size = memory.GetExRamSizeReal();
    if (!exram)
      return;
    const u32 base = offset - PPC_MEM_JOURNAL_EXRAM_BASE;
    for (u32 i = 0; i < size; ++i)
    {
      const u32 off = base + i;
      if (off >= exram_size)
        break;
      verifier->m_journal.ram_pre.emplace(offset + i, exram[off]);
    }
    return;
  }

  const u8* ram = verifier->m_core.m_guest.ram;
  const u32 ram_size = verifier->m_core.m_guest.ram_size;
  for (u32 i = 0; i < size; ++i)
  {
    const u32 off = offset + i;
    if (off >= ram_size)
      break;
    verifier->m_journal.ram_pre.emplace(off, ram[off]);
  }
}
""",
)
replace_once(
    tramp,
    """void StaticRecompLockstepVerifier::LsShadowJournalTrampoline(u32 offset, u32 size, void* user)
{
  auto* verifier = static_cast<StaticRecompLockstepVerifier*>(user);
  const u8* ram = verifier->m_core.m_guest.ram;
  const u32 ram_size = verifier->m_core.m_guest.ram_size;
  for (u32 i = 0; i < size; ++i)
  {
    const u32 off = offset + i;
    if (off >= ram_size)
      break;
    verifier->m_journal.ram_shadow_pre.emplace(off, ram[off]);
  }
}
""",
    """void StaticRecompLockstepVerifier::LsShadowJournalTrampoline(u32 offset, u32 size, void* user)
{
  auto* verifier = static_cast<StaticRecompLockstepVerifier*>(user);
  if (offset >= PPC_MEM_JOURNAL_EXRAM_BASE)
  {
    auto& memory = verifier->m_core.m_system.GetMemory();
    const u8* exram = memory.GetEXRAM();
    const u32 exram_size = memory.GetExRamSizeReal();
    if (!exram)
      return;
    const u32 base = offset - PPC_MEM_JOURNAL_EXRAM_BASE;
    for (u32 i = 0; i < size; ++i)
    {
      const u32 off = base + i;
      if (off >= exram_size)
        break;
      verifier->m_journal.ram_shadow_pre.emplace(offset + i, exram[off]);
    }
    return;
  }

  const u8* ram = verifier->m_core.m_guest.ram;
  const u32 ram_size = verifier->m_core.m_guest.ram_size;
  for (u32 i = 0; i < size; ++i)
  {
    const u32 off = offset + i;
    if (off >= ram_size)
      break;
    verifier->m_journal.ram_shadow_pre.emplace(off, ram[off]);
  }
}
""",
)

mmu = "project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/MMU_Tables.cpp"
replace_once(
    mmu,
    """  if (m_memory.GetEXRAM() && (em_address >> 28) == 0x1 &&
      (em_address & 0x0FFFFFFF) < m_memory.GetExRamSizeReal())
  {
    em_address &= 0x0FFFFFFF;

    if (m_ppc_state.m_enable_dcache && !wi)
""",
    """  if (m_memory.GetEXRAM() && (em_address >> 28) == 0x1 &&
      (em_address & 0x0FFFFFFF) < m_memory.GetExRamSizeReal())
  {
    const u32 exram_offset = em_address & 0x0FFFFFFF;

    // Lockstep uses the same RAM journal for MEM1 and MEM2. MEM2 keys live in
    // their physical 0x10000000-based range so they cannot alias MEM1 offsets.
    if constexpr (flag == XCheckTLBFlag::Write)
    {
      if (StaticRecompLockstep::g_ram_write_journal)
        StaticRecompLockstep::g_ram_write_journal(
            PPC_MEM_JOURNAL_EXRAM_BASE + exram_offset, size,
            StaticRecompLockstep::g_ram_write_journal_user);
    }

    em_address = exram_offset;

    if (m_ppc_state.m_enable_dcache && !wi)
""",
)

check = "project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompLockstep_Check.cpp"
replace_once(
    check,
    """  u8* ram = m_core.m_guest.ram;
  const u32 ram_size = m_core.m_guest.ram_size;
  u8* const l1 = memory.GetL1Cache();
  const u32 l1_size = memory.GetL1CacheSize();
  u8* const vmem = memory.GetFakeVMEM();
  const u32 vmem_size = memory.GetFakeVMemSize();
""",
    """  u8* ram = m_core.m_guest.ram;
  const u32 ram_size = m_core.m_guest.ram_size;
  u8* const exram = memory.GetEXRAM();
  const u32 exram_size = memory.GetExRamSizeReal();
  u8* const l1 = memory.GetL1Cache();
  const u32 l1_size = memory.GetL1CacheSize();
  u8* const vmem = memory.GetFakeVMEM();
  const u32 vmem_size = memory.GetFakeVMemSize();
  const auto read_ram_journal_byte = [&](u32 key, u8 fallback) {
    if (key >= PPC_MEM_JOURNAL_EXRAM_BASE)
    {
      const u32 off = key - PPC_MEM_JOURNAL_EXRAM_BASE;
      return exram && off < exram_size ? exram[off] : fallback;
    }
    return ram && key < ram_size ? ram[key] : fallback;
  };
  const auto write_ram_journal_byte = [&](u32 key, u8 value) {
    if (key >= PPC_MEM_JOURNAL_EXRAM_BASE)
    {
      const u32 off = key - PPC_MEM_JOURNAL_EXRAM_BASE;
      if (exram && off < exram_size)
        exram[off] = value;
      return;
    }
    if (ram && key < ram_size)
      ram[key] = value;
  };
""",
)
replace_once(
    check,
    """  m_journal.ram_post.clear();
  for (const auto& [off, pre] : m_journal.ram_pre)
    m_journal.ram_post.emplace(off, off < ram_size ? ram[off] : pre);
  for (const auto& [off, pre] : m_journal.ram_pre)
    if (off < ram_size)
      ram[off] = pre;
""",
    """  m_journal.ram_post.clear();
  for (const auto& [off, pre] : m_journal.ram_pre)
    m_journal.ram_post.emplace(off, read_ram_journal_byte(off, pre));
  for (const auto& [off, pre] : m_journal.ram_pre)
    write_ram_journal_byte(off, pre);
""",
)
replace_once(
    check,
    """    for (const auto& [off, post] : m_journal.ram_post)
    {
      const u8 iv = (off < ram_size) ? ram[off] : post;
      if (iv != post)
        diff += fmt::format(" mem[{:#010x}]:N={:#04x},I={:#04x}", 0x80000000u + off, post, iv);
    }
""",
    """    for (const auto& [off, post] : m_journal.ram_post)
    {
      const u8 iv = read_ram_journal_byte(off, post);
      if (iv != post)
        diff += fmt::format(" mem[{:#010x}]:N={:#04x},I={:#04x}", 0x80000000u + off, post, iv);
    }
""",
)
replace_once(
    check,
    """  for (const auto& [off, pre] : m_journal.ram_shadow_pre)
    if (off < ram_size)
      ram[off] = pre;
  for (const auto& [off, post] : m_journal.ram_post)
    if (off < ram_size)
      ram[off] = post;
""",
    """  for (const auto& [off, pre] : m_journal.ram_shadow_pre)
    write_ram_journal_byte(off, pre);
  for (const auto& [off, post] : m_journal.ram_post)
    write_ram_journal_byte(off, post);
""",
)

tests = "project/lib/ModernGekko/vendor/dolphin/GXRuntime/tests/runtime_tests.c"
replace_once(
    tests,
    """static u8 g_last_cache_operation;
""",
    """static u8 g_last_cache_operation;
static u32 g_mem_journal_offsets[4];
static u32 g_mem_journal_sizes[4];
static unsigned g_mem_journal_count;

static void test_mem_journal(u32 offset, u32 size, void* user) {
    assert(user == &g_mem_journal_count);
    assert(g_mem_journal_count < 4u);
    g_mem_journal_offsets[g_mem_journal_count] = offset;
    g_mem_journal_sizes[g_mem_journal_count] = size;
    g_mem_journal_count++;
}
""",
)
replace_once(
    tests,
    """static void test_guest_memory(void) {
""",
    """static void test_mem_write_journal_regions(void) {
    CPUState cpu = {0};
    u8 ram[32] = {0};
    u8 exram[32] = {0};
    cpu.ram = ram;
    cpu.ram_size = sizeof(ram);
    cpu.exram = exram;
    cpu.exram_size = sizeof(exram);

    g_mem_journal_count = 0;
    g_mem_write_journal = test_mem_journal;
    g_mem_write_journal_user = &g_mem_journal_count;
    mem_write32(&cpu, GC_RAM_BASE + 4u, 0x11223344u);
    mem_write16(&cpu, 0x90000000u + 8u, 0xA1B2u);
    g_mem_write_journal = NULL;
    g_mem_write_journal_user = NULL;

    assert(g_mem_journal_count == 2u);
    assert(g_mem_journal_offsets[0] == 4u);
    assert(g_mem_journal_sizes[0] == 4u);
    assert(g_mem_journal_offsets[1] == PPC_MEM_JOURNAL_EXRAM_BASE + 8u);
    assert(g_mem_journal_sizes[1] == 2u);
    assert(mem_read32(&cpu, GC_RAM_BASE + 4u) == 0x11223344u);
    assert(mem_read16(&cpu, 0x90000000u + 8u) == 0xA1B2u);
}

static void test_guest_memory(void) {
""",
)
replace_once(
    tests,
    """int main(void) {
    test_guest_memory();
    test_store_reservation();
""",
    """int main(void) {
    test_guest_memory();
    test_mem_write_journal_regions();
    test_store_reservation();
""",
)
