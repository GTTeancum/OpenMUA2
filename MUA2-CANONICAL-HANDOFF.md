# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow rules

- GitHub `main` is authoritative. Inspect current `main` before every change.
- **Multiplatform performance is the active priority.** Correctness expansion comes later unless a concrete failure blocks performance work.
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use **moderately short turns** because resume-stream failures occur: one focused implementation/merge plus validation and a small next-step investigation is acceptable; do not batch several independent changes.
- At the end of **every turn**, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never fabricate build, game, controller, save, audio, or performance results.
- Never commit RMSE52 proprietary game data, extracted game files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.

## Important locations

- DolRecomp: `project/lib/DolRecomp/`
- ModernGekko: `project/lib/ModernGekko/`
- GXRuntime: `project/lib/ModernGekko/vendor/dolphin/GXRuntime/`
- Static recomp runtime: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/`
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

Key accepted commits:

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
- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — fast-path same-section linked→runtime REL return translation using the active section index; cross-section/DOL returns retain the full scan.

Observed historical RMSE52 REL facts:

- module ID `1`
- REL version `3`
- header around `0x80E4A080`
- linked text around `0x80E4A164`
- accepted native REL text coverage `3,255,744` bytes
- one live REL module discovered
- historical accepted run advanced real game frames

Do not treat historical gameplay as fresh current-main validation.

## Performance boundary

The retained native-REL benchmark predates the recent optimizations:

- reported FPS: ~`10.319`
- guest-frame FPS: ~`2.062`
- speed: ~`0.1404`
- native dispatches: `174,476,293`
- native exceptions: `5,959`
- hook fallback: `1,299,055`
- hook fast cache: `1,295,291`
- hook slow: `3,764`
- JIT fallback runs: `11,023`

This proves native REL progression, **not acceptable current performance**. No FPS improvement may be claimed until RMSE52 is rerun.

## Latest accepted performance work — PR #17

PR #17 `Reuse REL section for return translation` is **MERGED**.

Merge commit:

- `7b7e412f671039e1d29e2cdff7b2e51509bc046e`

Behavior:

- Dispatchability carries the active REL section index alongside the already-carried linked PC.
- Native-burst entry and continuation carry that section index into the dispatch iteration.
- `TranslateRelAddress(linked_address, rel_section_hint)` checks the hinted section first.
- Same-section returns translate with a bounds check + arithmetic instead of scanning all active REL sections.
- Cross-section and DOL returns fall back to the existing full `ResolveRuntimeAddress()` scan.
- Non-REL path uses sentinel `0xffffffffu`.
- REL refresh, chunk verification, forced fallback, host-call gating, and prior runtime→linked reuse remain intact.

PR #17 validation:

OpenMUA2 tooling run `36026447896`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: **PASS**

ModernGekko run `36026447968`:

- Standalone Ubuntu: **PASS**
- Standalone Windows: **PASS**
- Full build/test Ubuntu: **PASS**
- Full build/test Windows: **PASS**

The Windows full job is the MSVC/Ninja integration path and compiles the changed StaticRecomp runtime.

Current status document updated in:

- `bccc38eb90bd822beca965773e3d849d79e8eb18` — `Record merged same-section REL return fast path`

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

The next hot path is the **continuation-side runtime→linked REL lookup**.

Current behavior after PR #17:

1. A native block returns a linked PC.
2. Same-section linked→runtime translation is now fast-pathed.
3. Before the next native block, `FastDispatchableAt()` → `ChunkIndexOf()` → `ResolveNativeAddress()` still linearly scans `m_active_rel_sections` to convert that runtime PC back to linked form.

Recommended next change:

- pass the previously active REL section index into `ResolveNativeAddress()` as a hint;
- check that section's **runtime** range first;
- if the runtime PC is still in that section, compute linked PC directly;
- on a hint miss, preserve the existing full active-section scan, direct-DOL lookup, and `RefreshRelSections()` fallback;
- do not allow the hint to bypass REL refresh or cross-section/DOL behavior.

This is the reverse counterpart to merged PR #17 and should remove the remaining same-section linear section scan on burst continuation.

## Next exact moderately-short turn

1. Inspect current `main`.
2. Implement only the runtime→linked same-section hint described above.
3. Add/update targeted source regression coverage.
4. Open a focused PR.
5. Observe tooling plus initial ModernGekko CI state; do not wait indefinitely if full integration jobs are still compiling.
6. Update/attach this handoff and stop.

## Last turn update — 2026-09-24

Latest source/status commits produced or observed this turn:

- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — PR #17 merged: `Reuse REL section for return translation`
- `bccc38eb90bd822beca965773e3d849d79e8eb18` — `Record merged same-section REL return fast path`

What happened:

- Waited for the two full PR #17 integration builds rather than ending at the initial CI gate.
- Full Windows integration: PASS.
- Full Ubuntu integration: PASS.
- Merged PR #17.
- Updated `docs/CURRENT-STATUS.md`.
- Investigated the next hotspot and confirmed continuation-side `ResolveNativeAddress()` still scans all active REL sections; a safe same-section runtime-range hint can precede that scan without altering fallback/refresh semantics.
- No additional source optimization was stacked after PR #17.
- No RMSE52 game-side run occurred.
