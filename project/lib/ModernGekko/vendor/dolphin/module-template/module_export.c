// RecompCore per-game native module export glue (game id set at build time).
//
// Wraps the DolRecomp-generated constant-time chunk dispatcher behind the
// StaticRecomp module ABI. All environment access goes through the CPUState
// hook pointers the chassis installs; this dylib has no host dependencies.

#include "generated.h"

#include "StaticRecompABI.h"

#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3) && defined(_MSC_VER)
#include <intrin.h>
#elif defined(DOLRECOMP_MODULE_HAVE_X86_64_V3) && defined(__x86_64__) && \
    (defined(__GNUC__) || defined(__clang__))
#include <cpuid.h>
#endif

#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
static int host_has_x86_64_v3(void)
{
    // Host capabilities do not change during the process. Probe once and reuse
    // the result for both module-load dispatch selection and indirect dispatch.
    static int supported = -1;
    if (supported >= 0)
        return supported;
#if defined(_M_X64) && defined(_MSC_VER)
    int leaf[4];
    __cpuid(leaf, 1);
    const unsigned int leaf1_ecx = (unsigned int)leaf[2];
    const unsigned int leaf1_required = (1u << 12) | (1u << 22) | (1u << 27) |
        (1u << 28) | (1u << 29);
    if ((leaf1_ecx & leaf1_required) != leaf1_required ||
        (_xgetbv(0) & 6u) != 6u)
    {
        supported = 0;
        return supported;
    }
    __cpuidex(leaf, 7, 0);
    if (((unsigned int)leaf[1] & ((1u << 3) | (1u << 5) | (1u << 8))) !=
        ((1u << 3) | (1u << 5) | (1u << 8)))
    {
        supported = 0;
        return supported;
    }
    __cpuid(leaf, (int)0x80000001u);
    supported = ((unsigned int)leaf[2] & (1u << 5)) != 0;
    return supported;
#elif defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
    unsigned int eax, ebx, ecx, edx;
    supported = __get_cpuid(1, &eax, &ebx, &ecx, &edx) != 0;
    const unsigned int leaf1_required = (1u << 12) | (1u << 22) | (1u << 27) |
        (1u << 28) | (1u << 29);
    supported = supported && (ecx & leaf1_required) == leaf1_required;
    if (supported)
    {
        unsigned int xcr0_low;
        unsigned int xcr0_high;
        __asm__ volatile("xgetbv" : "=a"(xcr0_low), "=d"(xcr0_high) : "c"(0));
        supported = (xcr0_low & 6u) == 6u;
    }
    supported = supported && __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) != 0 &&
        (ebx & ((1u << 3) | (1u << 5) | (1u << 8))) ==
            ((1u << 3) | (1u << 5) | (1u << 8));
    supported = supported && __get_cpuid(0x80000001u, &eax, &ebx, &ecx, &edx) != 0 &&
        (ecx & (1u << 5)) != 0;
    return supported;
#else
    supported = 0;
    return supported;
#endif
}
#endif
#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
static int s_use_x86_64_v3 = 0;
#endif
static int selected_dispatch(CPUState* ctx, u32 address)
{
#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
    if (s_use_x86_64_v3)
        return dolrecomp_call__x86_64_v3(ctx, address);
#endif
    return dolrecomp_call(ctx, address);
}

void dolrecomp_indirect_dispatch(CPUState* ctx, u32 address)
{
    (void)selected_dispatch(ctx, address);
}

static int chassis_dispatch_baseline(CPUState* ctx, u32 address)
{
#if defined(DOLRECOMP_ENABLE_REPLACEMENTS)
    // Replacement-enabled modules need the public dispatcher policy.
    return dolrecomp_call(ctx, address);
#else
    // StaticRecomp only calls the module after native eligibility and host-call
    // routing have already accepted this effective address. Go straight to the
    // generated original-code lookup instead of repeating host-call and physical
    // alias policy in dolrecomp_call().
    DolRecompFunction fn = dolrecomp_find_original(address);
    if (!fn)
        return 0;
    ctx->pc = address;
    fn(ctx);
    return 1;
#endif
}

#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
static int chassis_dispatch_x86_64_v3(CPUState* ctx, u32 address)
{
#if defined(DOLRECOMP_ENABLE_REPLACEMENTS)
    return dolrecomp_call__x86_64_v3(ctx, address);
#else
    DolRecompFunction fn = dolrecomp_find_original__x86_64_v3(address);
    if (!fn)
        return 0;
    ctx->pc = address;
    fn(ctx);
    return 1;
#endif
}
#endif
static void chassis_on_state_loaded(CPUState* ctx)
{
    // Re-arm host FP rounding/flush state from the freshly loaded guest FPSCR.
    ppc_fpscr_updated(ctx);
}

#include "module_tables.inc"

#if MODULE_REL_MODULE_COUNT
#define MODULE_REL_MODULES s_rel_modules
#else
#define MODULE_REL_MODULES 0
#endif

#define MODULE_DESC_INIT(dispatch_fn) { \
    STATICRECOMP_ABI_VERSION, \
    GXRUNTIME_CPU_ABI_VERSION, \
    (u32)sizeof(CPUState), \
    MODULE_GAME_ID, \
    DOLRECOMP_ENTRY_POINT, \
    dispatch_fn, \
    chassis_on_state_loaded, \
    s_code_ranges, \
    MODULE_CODE_RANGE_COUNT, \
    s_smc_ranges, \
    MODULE_SMC_RANGE_COUNT, \
    s_chunk_ranges, \
    MODULE_CHUNK_RANGE_COUNT, \
    s_chunk_hashes, \
    MODULE_REL_MODULES, \
    MODULE_REL_MODULE_COUNT, \
}

static const StaticRecompModuleDesc s_desc_baseline =
    MODULE_DESC_INIT(chassis_dispatch_baseline);
#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
static const StaticRecompModuleDesc s_desc_x86_64_v3 =
    MODULE_DESC_INIT(chassis_dispatch_x86_64_v3);
#endif

#undef MODULE_DESC_INIT
#undef MODULE_REL_MODULES
#if defined(_WIN32)
#define RECOMP_MODULE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define RECOMP_MODULE_EXPORT __attribute__((visibility("default")))
#else
#define RECOMP_MODULE_EXPORT
#endif

RECOMP_MODULE_EXPORT const StaticRecompModuleDesc* staticrecomp_get_module(void)
{
#if defined(DOLRECOMP_MODULE_HAVE_X86_64_V3)
    // Bind the chassis to one dispatcher at module load. Generated indirect
    // dispatch also reuses this decision instead of re-entering feature probing.
    s_use_x86_64_v3 = host_has_x86_64_v3();
    if (s_use_x86_64_v3)
        return &s_desc_x86_64_v3;
#endif
    return &s_desc_baseline;
}
