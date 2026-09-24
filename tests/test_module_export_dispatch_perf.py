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

    def test_selected_dispatch_uses_cached_host_probe(self) -> None:
        text = MODULE_EXPORT.read_text(encoding="utf-8")
        start = text.index("static int selected_dispatch")
        end = text.index("void dolrecomp_indirect_dispatch", start)
        body = text[start:end]
        self.assertIn("if (host_has_x86_64_v3())", body)
        self.assertIn("return dolrecomp_call__x86_64_v3(ctx, address);", body)


if __name__ == "__main__":
    unittest.main()
