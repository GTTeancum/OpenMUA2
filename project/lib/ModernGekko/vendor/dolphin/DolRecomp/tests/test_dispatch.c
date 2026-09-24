#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/backend/dispatch.h"

#define BASE 0x80003000u

static int pass_count = 0;
static int fail_count = 0;

static void check(int condition, const char* name) {
    printf("DISPATCH,%s,%s\n", name, condition ? "PASS" : "FAIL");
    if (condition)
        pass_count++;
    else
        fail_count++;
}

// MSVC has no setenv, and _putenv_s is not portable back the other way.
static int set_lookup_mode(const char* value) {
#if defined(_WIN32)
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "DOLRECOMP_DISPATCH_LOOKUP=%s",
             value ? value : "");
    return _putenv(buffer) == 0;
#else
    if (!value)
        return unsetenv("DOLRECOMP_DISPATCH_LOOKUP") == 0;
    return setenv("DOLRECOMP_DISPATCH_LOOKUP", value, 1) == 0;
#endif
}

// The four ranges below cover the three shapes the lookup has to get right:
// a contiguous equal-stride run (0x3000..0x3080), a short chunk closing that
// run (0x3080..0x30A0), and an isolated chunk a page away (0x4000..0x4020).
static char* emit_dispatch_to_string(void) {
    FunctionList funcs = {0};
    FILE* f = NULL;
    char* buf = NULL;

    if (!function_list_add(&funcs, BASE, BASE + 0x40u) ||
        !function_list_add(&funcs, BASE + 0x40u, BASE + 0x80u) ||
        !function_list_add(&funcs, BASE + 0x80u, BASE + 0xA0u) ||
        !function_list_add(&funcs, BASE + 0x1000u, BASE + 0x1020u)) {
        function_list_free(&funcs);
        return NULL;
    }

    f = tmpfile();
    if (!f) {
        function_list_free(&funcs);
        return NULL;
    }

    emit_chunk_prototype(f, BASE);
    emit_chunk_prototype(f, BASE + 0x40u);
    emit_chunk_prototype(f, BASE + 0x80u);
    emit_chunk_prototype(f, BASE + 0x1000u);
    emit_dispatch_helpers(f, &funcs, BASE);
    emit_function_lookup(f, &funcs, "__x86_64_v3");
    function_list_free(&funcs);
    fflush(f);

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    buf = (char*)malloc((size_t)size + 1u);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)size, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

int main(void) {
    char* code = emit_dispatch_to_string();
    if (!code) {
        check(0, "emit dispatch helpers");
        printf("DISPATCH,total,%d passed %d failed\n", pass_count, fail_count);
        return 1;
    }

    check(strstr(code, "dolrecomp_find_original") != NULL,
          "emits original lookup helper");
    check(strstr(code, "dolrecomp_call_original") != NULL,
          "emits original call helper");
    check(strstr(code, "dolrecomp_page_first[DOLRECOMP_LOOKUP_PAGES]") != NULL &&
          strstr(code, "run = dolrecomp_page_first[page];") != NULL,
          "default lookup uses page-indexed dispatch");
    check(strstr(code, "dolrecomp_page_end[DOLRECOMP_LOOKUP_PAGES]") != NULL &&
          strstr(code, "end = dolrecomp_page_end[page];") != NULL,
          "indexed lookup bounds candidate runs to the current page");
    check(strstr(code, "if (end == run) return NULL;") != NULL &&
          strstr(code, "if (end == run + 1u)") != NULL,
          "indexed lookup fast-paths empty and single-run pages");
    check(strstr(code, "while (run < end && dolrecomp_run_end[run] <= address)") != NULL &&
          strstr(code, "if (run >= end || address < dolrecomp_run_start[run])") != NULL,
          "multi-run lookup cannot walk beyond the current page window");
    check(strstr(code, "#define DOLRECOMP_LOOKUP_RUNS 2u") != NULL &&
          strstr(code, "#define DOLRECOMP_LOOKUP_BASE 0x80003000u") != NULL &&
          strstr(code, "#define DOLRECOMP_LOOKUP_PAGES 2u") != NULL,
          "default indexed lookup covers exactly the emitted code");
    check(strstr(code, "func_80003000,") != NULL &&
          strstr(code, "func_80003040,") != NULL &&
          strstr(code, "func_80003080,") != NULL &&
          strstr(code, "func_80004000,") != NULL,
          "default indexed chunk table covers generated chunks");
    check(strstr(code, "if (address >= 0x80003000u && address < 0x80003040u") == NULL,
          "default indexed lookup removes the linear range chain");
    check(strstr(code, "dolrecomp_find_original__x86_64_v3") != NULL &&
          strstr(code, "func_80003000__x86_64_v3,") != NULL &&
          strstr(code, "func_80003040__x86_64_v3,") != NULL &&
          strstr(code, "return func_80004000__x86_64_v3;") != NULL,
          "target variants use indexed dispatch");
    check(strstr(code, "ctx->pc = address;") != NULL,
          "call helpers set the entry pc");
    check(strstr(code, "#if defined(DOLRECOMP_ENABLE_REPLACEMENTS)") != NULL &&
          strstr(code,
                 "if (dolrecomp_dispatch_replacement(ctx, address)) return 1;") != NULL,
          "public dispatcher supports module replacements");
    check(strstr(code,
                 "if (ctx->host_call && ppc_host_call(ctx, address)) return 1;") != NULL,
          "public dispatcher checks installed host replacements first");
    check(strstr(code, "dolrecomp_physical_pc_alias") != NULL &&
          strstr(code,
                 "if (ctx->host_call && ppc_host_call(ctx, alias)) return 1;") != NULL &&
          strstr(code, "if (dolrecomp_call_original(ctx, alias)) return 1;") != NULL,
          "public dispatcher retries physical MEM1 aliases");
    check(strstr(code, "if (dolrecomp_call_original(ctx, address)) return 1;") != NULL,
          "public dispatcher can fall back to original code");

    free(code);

    // Linear lookup remains available for controlled A/B performance tests and
    // as a fallback for unusual generated layouts.
    if (set_lookup_mode("linear")) {
        char* linear = emit_dispatch_to_string();
        if (!linear) {
            check(0, "linear: emit dispatch helpers");
        } else {
            check(strstr(linear, "dolrecomp_page_first[DOLRECOMP_LOOKUP_PAGES]") == NULL,
                  "linear: page index is disabled");
            check(strstr(linear,
                         "if (address >= 0x80004000u && address < 0x80004020u") != NULL,
                  "linear: isolated chunks use direct range tests");
            check(strstr(linear, "static const DolRecompFunction chunk_functions[]") != NULL &&
                  strstr(linear, "return chunk_functions[offset / 0x00000040u];") != NULL,
                  "linear: contiguous equal-stride chunks still use a compact table");
            check(strstr(linear, "ctx->pc = address;") != NULL &&
                  strstr(linear, "dolrecomp_physical_pc_alias") != NULL,
                  "linear: leaves the rest of the dispatcher alone");
            free(linear);
        }
        set_lookup_mode(NULL);
    } else {
        check(0, "linear: set DOLRECOMP_DISPATCH_LOOKUP");
    }

    printf("DISPATCH,total,%d passed %d failed\n", pass_count, fail_count);
    return fail_count == 0 ? 0 : 1;
}
