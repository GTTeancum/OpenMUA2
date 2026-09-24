from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
MODULE_EXPORT = (
    ROOT
    / "project/lib/ModernGekko/vendor/dolphin/module-template/module_export.c"
)
DOLRECOMP_DISPATCH = ROOT / "project/lib/DolRecomp/src/backend/dispatch.c"
DOLRECOMP_VARIANTS = ROOT / "project/lib/DolRecomp/src/backend/variant_output.c"


class ModuleExportDispatchPerfTests(unittest.TestCase):
    def test_x86_64_v3_feature_probe_is_cached_before_compiler_paths(self) -> None:
        text = MODULE_EXPORT.read_text(encoding="utf-8")
        start = text.index("static int host_has_x86_64_v3(void)")
        end = text.index("static int selected_dispatch", start)
        body = text[start:end]

        cache = body.index("static int supported = -1;")
        cached_return = body.index("if (supported >= 0)", cache)
        msvc = body.index("#if defined(_M_X64) && defined(_MSC_VER)")
        gcc = body.index(
            "#elif defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))"
        )

        self.assertLess(cache, msvc)
        self.assertLess(cached_return, msvc)
        self.assertLess(msvc, gcc)
        self.assertIn("supported = 0;", body[msvc:gcc])
        self.assertIn("supported = ((unsigned int)leaf[2]", body[msvc:gcc])

    def test_indirect_dispatch_reuses_module_load_variant(self) -> None:
        text = MODULE_EXPORT.read_text(encoding="utf-8")
        start = text.index("static int selected_dispatch")
        end = text.index("void dolrecomp_indirect_dispatch", start)
        body = text[start:end]
        self.assertIn("if (s_use_x86_64_v3)", body)
        self.assertNotIn("host_has_x86_64_v3()", body)
        self.assertIn("return dolrecomp_call__x86_64_v3(ctx, address);", body)

    def test_chassis_dispatch_variant_is_bound_at_module_load(self) -> None:
        text = MODULE_EXPORT.read_text(encoding="utf-8")
        baseline = text.index("static int chassis_dispatch_baseline")
        v3 = text.index("static int chassis_dispatch_x86_64_v3")
        export = text.index(
            "RECOMP_MODULE_EXPORT const StaticRecompModuleDesc* staticrecomp_get_module"
        )
        get_module = text[export:]

        self.assertIn(
            "return dolrecomp_call_chassis(ctx, address);",
            text[baseline:v3],
        )
        self.assertIn(
            "return dolrecomp_call_chassis__x86_64_v3(ctx, address);",
            text[v3:export],
        )
        self.assertNotIn("return dolrecomp_call(ctx, address);", text[baseline:v3])
        self.assertNotIn(
            "return dolrecomp_call__x86_64_v3(ctx, address);",
            text[v3:export],
        )
        self.assertIn("s_use_x86_64_v3 = host_has_x86_64_v3();", get_module)
        self.assertIn("if (s_use_x86_64_v3)", get_module)
        self.assertIn("return &s_desc_x86_64_v3;", get_module)
        self.assertIn("return &s_desc_baseline;", get_module)
        self.assertNotIn("selected_dispatch(ctx, address)", text[baseline:export])

    def test_chassis_helpers_preserve_non_host_dispatch_behavior(self) -> None:
        dispatch = DOLRECOMP_DISPATCH.read_text(encoding="utf-8")
        variants = DOLRECOMP_VARIANTS.read_text(encoding="utf-8")

        chassis = dispatch.index(
            'static inline int dolrecomp_call_chassis(CPUState* ctx, u32 address)'
        )
        public = dispatch.index(
            'static inline int dolrecomp_call(CPUState* ctx, u32 address)',
            chassis,
        )
        chassis_body = dispatch[chassis:public]
        self.assertIn("dolrecomp_dispatch_replacement(ctx, address)", chassis_body)
        self.assertIn("dolrecomp_call_original(ctx, address)", chassis_body)
        self.assertIn("dolrecomp_physical_pc_alias(ctx, address, &alias)", chassis_body)
        self.assertNotIn("ppc_host_call", chassis_body)

        self.assertIn(
            '"\\nstatic inline int dolrecomp_call_chassis%s(CPUState* ctx, u32 address) {\\n"',
            variants,
        )
        self.assertIn(
            '"        fn = dolrecomp_find_original%s(alias);\\n"',
            variants,
        )
        self.assertIn(
            '"    if (ctx->host_call && ppc_host_call(ctx, address)) return 1;\\n"',
            variants,
        )


if __name__ == "__main__":
    unittest.main()
