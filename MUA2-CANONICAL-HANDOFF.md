# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow rules

- GitHub `main` is authoritative. Inspect current `main` before every change.
- **Multiplatform performance is the active priority.** Correctness expansion comes later unless a concrete failure blocks performance work.
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use **moderately short turns** because resume-stream failures occur: one focused implementation/merge plus validation and a small next-step investigation is appropriate; do not batch unrelated optimizations.
- At the end of **every turn**, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never fabricate build, game, controller, save, audio, or performance results.
- Never commit RMSE52 proprietary game data, extracted files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.

## Important locations

- DolRecomp: `project/lib/DolRecomp/`
- ModernGekko: `project/lib/ModernGekko/`
- GXRuntime: `project/lib/ModernGekko/vendor/dolphin/GXRuntime/`
- Static recomp runtime: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/`
- Static recomp config: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/Config/StaticRecompSettings.cpp`
- Tooling: `tools/`
- Tests: `tests/`
- CI: `.github/workflows/`
- Current status: `docs/CURRENT-STATUS.md`
- Work log: `docs/WORKLOG-2026-09-23.md`
- Recovery plan: `docs/RECOVERY-PLAN.md`
- Original Windows workspace target: `D:\\Programming\\GitHub\\OpenMUA2\\`
- Local proprietary/generated data: `.local/` only; never commit it.

## Accepted runtime/performance state

Native DOL and native REL execution are present. SMC/chunk-hash protection remains enabled.

Key accepted performance/correctness commits:

- `67d75dc63455b47e34159fefae4ed39fca5e5efc` — absolute linked REL section-table support.
- `770db1089526cae9d0fc1b48fbd65c760e75efe6` — cache-control ops stay in native bursts.
- `7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — scalar FMA semantics repair.
- `26334ad651567bf098401f781d1189e5f75c296c` — MEM2 lockstep journaling/restoration.
- `46091e1a2394e052f116838727471cbed974aa2d` — merged DOL+REL dispatch eligibility guards.
- `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — `O2 + indexed` default.
- `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc` — page-index combined DOL+REL dispatch.
- `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6` — empty/single-chunk merged-dispatch fast paths.
- `405b81de4136a7532e966218185a190f6eb9230d` — page-local ordinary indexed dispatch.
- `917a5d893087736a74566dbaf71de5f3989a6d90` — cached x86-64-v3 feature detection.
- `4e677ac8491bc3b5256998ba3688893f61d07695` — module-load baseline/v3 dispatch selection.
- `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1` — reuse module-load v3 choice for indirect dispatch.
- `2776fa0a3e80136495a32552b9d909e16dcfcd5e` — gate exact host-call probes by cached chunk coverage.
- `af7938bdb63d4530ea43a6ba445800fc4171a153` — reuse runtime→linked PC resolution across eligibility and dispatch.
- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — fast-path same-section linked→runtime REL return translation.
- `7a9cdc0165257ec19301972785150e4961f7f65e` — fast-path same-section runtime→linked REL continuation resolution.

## Historical RMSE52 boundary

Observed retained facts:

- module ID `1`
- REL version `3`
- header around `0x80E4A080`
- linked text around `0x80E4A164`
- accepted native REL text coverage `3,255,744` bytes
- historical accepted run advanced real game frames

Historical performance, predating recent optimizations:

- reported FPS ~`10.319`
- guest-frame FPS ~`2.062`
- speed ~`0.1404`
- native dispatches `174,476,293`
- native exceptions `5,959`
- hook fallback `1,299,055`
- hook fast cache `1,295,291`
- hook slow `3,764`
- JIT fallback runs `11,023`

Do not claim current FPS improvement until the actual RMSE52 route is rerun.

## Latest accepted work — PR #18

PR #18 `Reuse REL section for native continuation lookup` is **MERGED**.

Merge commit:

- `7a9cdc0165257ec19301972785150e4961f7f65e`

Behavior:

- Runtime→linked native address resolution now accepts an optional previous REL-section hint.
- Burst continuation snapshots the previous active section before the output section index is overwritten.
- Same-section continuations check that section's runtime range first and map directly to linked PC.
- Hint misses preserve the prior full active-section scan.
- Direct DOL lookup remains after the REL scan.
- `RefreshRelSections()` remains the final allowed fallback.
- Initial dispatch and callers without a hint retain old behavior via sentinel `0xffffffffu`.
- PR #17's linked→runtime same-section fast path remains intact.

Validation:

OpenMUA2 tooling run `36030032034`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: **PASS**

ModernGekko run `36030032243`:

- Standalone Ubuntu: **PASS**
- Standalone Windows: **PASS**
- Full build/test Ubuntu: **PASS**
- Full build/test Windows: **PASS**

The Windows full job uses the MSVC/Ninja integration path and compiled the modified StaticRecomp core.

`docs/CURRENT-STATUS.md` updated in:

- `41ac96c8fe9b6feee8daecd73d0ec2342cce4f2f` — `Record merged REL continuation hint`

## Current blocker

Fresh game-side performance measurement is still blocked because this environment does not have the proprietary RMSE52 workspace/image.

No fresh claim is made for:

- current FPS/speed
- complete boot/title/menu
- opening gameplay/combat
- complete level
- long-session stability
- multiplayer
- audio/controller correctness
- save/reload across restart
- full Windows game execution
- fresh large lockstep total
- crash-free/glitch-free completion

## Next performance target — investigated, not yet changed

The next likely hot-path cost is the forced-fallback eligibility check.

Verified source facts:

- `StaticRecompSettings.cpp` defines `MAIN_STATICRECOMP_FALLBACK_RANGES` with default value `""`.
- `StaticRecompCore::Init()` parses that string (or `STATICRECOMP_FALLBACK_RANGES`) into `m_forced_fallback_ranges`.
- Therefore the normal/default configuration has **no forced fallback ranges**.
- REL-backed native continuation still routes through `FastDispatchableAt()` because `m_has_rel_modules` is true.
- `FastDispatchableAt()` currently calls `IsForcedFallbackAddress(address)` unconditionally.
- `DispatchableAt()` does the same on burst entry.
- `IsForcedFallbackAddress()` is an out-of-line function that loops `m_forced_fallback_ranges`; with the default empty vector this still imposes a function call plus empty-loop check on every native eligibility test.

Recommended next focused change:

- in `FastDispatchableAt()` and `DispatchableAt()`, only call `IsForcedFallbackAddress(address)` when `!m_forced_fallback_ranges.empty()`;
- preserve the exact existing range behavior when any forced fallback range is configured;
- add a source regression pinning the empty-vector short-circuit and configured-range path;
- keep the change portable C++ and validate Windows + Ubuntu.

This is a lower-risk per-block chassis optimization than changing cross-section behavior further.

## Next exact moderately-short turn

1. Inspect current `main`.
2. Implement only the empty-forced-fallback short-circuit described above.
3. Add/update targeted regression coverage.
4. Open a focused PR.
5. Observe tooling and initial ModernGekko CI state.
6. Update/attach this handoff and stop.

## Last turn update — 2026-09-24

Latest `main` at turn start:

- `464182734220a32ee3b2b57ca680d559b2e5dc4f` — `Refresh MUA2 handoff while PR18 Windows build runs`

What happened this turn:

- PR #18 full Windows integration completed **PASS**.
- Confirmed all PR #18 tooling, standalone, and full integration jobs passed on Windows and Ubuntu.
- Merged PR #18 as `7a9cdc0165257ec19301972785150e4961f7f65e`.
- Updated `docs/CURRENT-STATUS.md` as `41ac96c8fe9b6feee8daecd73d0ec2342cce4f2f`.
- Investigated the next per-block chassis cost and verified that forced fallback ranges default to empty while `FastDispatchableAt()` / `DispatchableAt()` still call the range-scan helper unconditionally.
- No additional source optimization was stacked after merging PR #18.
- No RMSE52 game-side run occurred.
