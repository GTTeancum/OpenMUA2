from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
MODULE_EXPORT = (
    ROOT
    / "project/lib/ModernGekko/vendor/dolphin/module-template/module_export.c"
)


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

        self.assertIn("return dolrecomp_call(ctx, address);", text[baseline:v3])
        self.assertIn(
            "return dolrecomp_call__x86_64_v3(ctx, address);",
            text[v3:export],
        )
        self.assertIn("s_use_x86_64_v3 = host_has_x86_64_v3();", get_module)
        self.assertIn("if (s_use_x86_64_v3)", get_module)
        self.assertIn("return &s_desc_x86_64_v3;", get_module)
        self.assertIn("return &s_desc_baseline;", get_module)
        self.assertNotIn("selected_dispatch(ctx, address)", text[baseline:export])


if __name__ == "__main__":
    unittest.main()
