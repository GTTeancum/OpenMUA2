# MUA2 FPC01 — floating-point compiler repair

**September 22, 2026 · Pre-integration source/test package. Not MG03 and not a new playable build.**

## Result

A real defect was reproduced and corrected in the retained, pinned DolRecomp `ppc_fctiw` helper, shared by `fctiw` and `fctiwz`. The native helper returned the expected integer but updated the wrong floating-point status bits. It could leave stale FI/FR flags and overwrite part of the floating-point result classification field instead.

Only three constants change:

| Operation | Original mask | Corrected mask |
|---|---|---|
| Clear FI and FR | `0x00006000` | `0x00060000` |
| Set FI | `0x00004000` | `0x00020000` |
| Set FR | `0x00002000` | `0x00040000` |

The correction agrees with the FI/FR definitions already present elsewhere in the same production file and the retained reference implementation. No game memory patch, clock override, comparison mask, whitelist, fallback removal or disabled code-hash check was introduced.

## Measured checks

The standalone fixture calls the unchanged pinned RecompCore `ConvertToInteger` implementation and the native DolRecomp helper. It links the actual production PowerPC, cache, condition-register and configuration support sources; it does not use a reimplemented reference or test-only replacements for their behavior. The fixture does not instantiate an Interpreter or run the whole emulator.

| Check | Actual result |
|---|---|
| Original helper, GCC and Clang | **1,217,268 failures out of 1,298,576 inputs** in each replay |
| Corrected helper, GCC 14.2.0, O1 | **1,298,576 inputs, zero failures** |
| Corrected helper, Clang 17.0.0, O1 | **1,298,576 inputs, zero failures** |
| Sanitizers for both final configurations | AddressSanitizer and UndefinedBehaviorSanitizer, with vptr excluded; no sanitizer failure reported |
| Guarded patch utility | **16 tests passed** |
| Original C-backend DolRecomp CTests | **19 passed** |
| Corrected C-backend DolRecomp CTests | **19 passed** |

The two compiler runs repeat the same matrix, not 2.6 million distinct inputs. The matrix contains 1,048,576 structured cases plus 250,000 seeded random IEEE-754 bit patterns. It covers all four guest rounding modes, both instructions, all 32 FPRF patterns, pre-existing FI/FR combinations, exception enables/sticky bits, signed zero, ties, subnormals, integer limits, infinities and quiet/signaling NaNs.

Full FPSCR words, boxed integer result bits and suppressed destination writes are compared. Independent conversion/result/flag invariants are also checked. Of the total inputs, 351,888 exercise invalid conversions, 32,881 contain NaNs and 175,820 suppress invalid-result writes. The original failures are expected negative controls, not passing tests. Their actual result bits and write decisions agree; the reproduced defect is in FPSCR state.

This is a **helper-level** test. Guest FE0/FE1 exception delivery, record-form full instruction dispatch, host FTZ/DAZ control integration, complete runtime behavior and gameplay are outside its coverage. Host FTZ/DAZ are off and MSR.FE0/FE1 are disabled. The vptr-only sanitizer is excluded because the fixture includes a production interpreter translation unit without instantiating its runtime object. No other UBSan category is deliberately excluded.

Final machine-readable results and command receipts are in `RESULTS.json` and `evidence/`. Early missing-include/link attempts and interrupted O2 build attempts are documented in `evidence/ATTEMPT_HISTORY.json`; they are not counted as passes. The completed sanitizer matrices use O1.

## Relationship to MG02

The retained MG02 report records three unresolved differential differences. Static inspection of the original DOL finds 15 `fctiwz` instructions in the first reported range, `0x80388908–0x80388B00`. That makes this repair relevant to investigate, but **does not prove that it resolves that live mismatch**. The original operands, exact executed path and MG02 native module must be replayed.

The second floating-point difference and the inlined-loop register/MEM2 difference remain unresolved. Other legacy helpers also contain suspicious old FI/FR masks; this package deliberately does not change those untested paths. No historical MG02 mismatch is marked closed.

**The actual `MUA2_MG02_Checkpoint.zip` is unavailable in the current scratch session and was not returned by conversation-file or Library recovery.** Its report, results, screenshot and archive validation receipt are retained, but those do not contain its 28-file source overlay, generated module and diagnostic states. The original game, dependency volumes and MG01 checkpoint remain available. Work here was performed in a separate recovery/validation tree; the restored original project was not modified.

The retained validation receipt identifies the needed archive as:

```
MUA2_MG02_Checkpoint.zip
Size:   384019790 bytes
SHA256: b458b6e0f1b1eddd40793be35fffff14b5856b50971dda3b10d3257961127779
```

Reattaching that archive is the required handoff for cumulative integration and further gameplay work. There is no need to collect or upload the game/dependencies again. This package is **not** a substitute for MG02 and must not replace its checkpoint directory. No new game binary, gameplay session, audible-audio test, save/reload test, Windows build or Windows execution is supplied or claimed.

## Safe application and reproduction

The patcher checks the exact function body, not a whole-file hash, so unrelated source changes can be retained. It accepts only the original body or this exact corrected body, validates both destinations before writing, preserves line endings and is idempotent. Other edits within this function cause refusal rather than overwrite. Each file replacement is atomic; a multi-file filesystem failure is not a transaction, so rerun the check after any error.

For a dependency workspace with `project/` underneath it, the default is read-only:

```bash
python3 tools/patch_fctiw.py --workspace /path/to/scratch
```

After recovering MG02 and confirming this function still needs the correction, an explicit application targets both the main and nested DolRecomp copies:

```bash
python3 tools/patch_fctiw.py --workspace /path/to/scratch --apply
```

That command **does not** regenerate or rebuild a native game module. Its effects must be rebuilt into the modules/runtime helpers that use this code. `patches/fctiw-fpscr.patch` is included for inspection, but the guarded helper is preferable to an unguarded whole-file replacement.

To reproduce the matrices, use the pristine pinned dependency sources, an empty output destination, and Linux x86-64 with a C++23-capable compiler. The supplied reference-file hashes intentionally reject a different reference revision. No SDK download, installation or internet access occurs:

```bash
python3 tools/run_regression.py --workspace /path/to/pristine-scratch \
  --output /path/to/new-gcc-results --cc gcc --cxx g++ --optimization 1 --sanitize

python3 tools/run_regression.py --workspace /path/to/pristine-scratch \
  --output /path/to/new-clang-results --cc clang --cxx clang++ --optimization 1 --sanitize

python3 -m unittest discover -s tests -p 'test_*.py' -v

python3 tools/run_upstream_regression.py \
  --source /path/to/pristine-scratch/project/lib/DolRecomp \
  --output /path/to/new-upstream-results
```

The commands select the completed O1 sanitizer configurations. The runner default is O2; interrupted exploratory O2 attempts are not claimed as completed. The upstream wrapper copies the source into its output before applying the change and building before/after; use the pinned source tree, not a symlinked custom project layout.

`tools/audit_fctiw_sites.py ORIGINAL_MAIN.DOL --output sites.json` reproduces the static instruction inventory and rejects a different original DOL hash.

## Provenance and license

`SOURCE_PINS.json` records exact revisions and reference-file hashes. DolRecomp is pinned at `1bec3554ecc4817cf78319ca3d8a0669477f29fa`; RecompCore at `10f7ac4fe48f74485fbc161ebf4d67242e11e671`. The dependency collector's retained source archives supply the full upstream trees. The regression does not fetch current branches or silently update those revisions.

The DolRecomp patch and original-function fixture are covered by its GPL-3.0 license, retained as `LICENSE`. RecompCore/Dolphin remains under its original per-file licenses; its `COPYING` notice is retained under `licenses/`. Full upstream code, game data, fonts, compiled test executables and build directories are not duplicated in this small package. Supporting scripts/tests written for this package may be used under GPL-3.0-or-later. No license to proprietary game assets is granted.
