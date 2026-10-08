# Current status â€” GitHub main

## Controls and Powers-page completion - 2026-10-08

TODO #2 is complete and removed. Both native Options copies now use the shared
Xbox layout; the pause-menu cw_pda_rev package was previously missed. Version8
requires both packages in the paired manifest. No C++ runtime changes.

Private-profile native menu checks passed: LB refunded one point/rank, RB
restored it, Y cycled priority, A opened Assign, LT redistributed with Autospend
On, and D-pad left/right changed the selected hero. Eligibility used an isolated
level/points fixture, not installed saves or forced menu outcomes. Fresh boot,
normal Load Game, then Start/Options confirmed the corrected pause layout.

Windows optimized x64 launcher build passed without printed warnings/errors;
Options tests4/4 (both packages), font tests2/2, launcher checks67/67 passed.
Paired v8 staged at C:/Games/MUA2/OpenMUA2.exe with GameData; all55 saves unchanged.
Installed embedded-runtime and actual paired UI manifest verification passed.
No audio/FPS or physical-controller acceptance claim. Runtime stopped.
Costume cycling remains a separate TODO: X Skin, Y Details.
Evidence: evidence/windows-20261007/MENU-COMPLETION-20261008.json.


## Controls layout and rank-symbol preservation - 2026-10-08

Native Options scene now uses aligned glyph/action columns plus Powers/Fusion
rows. QTE reference removed from Options; contextual gameplay prompt retained.
Extra Xbox font cells moved into verified unused cells, preserving every native
power-rank star and priority dot. Fresh campaign capture confirms restored stars;
native Options capture confirms the final layout without runtime position edits.
Corrected the embedded Powers Assign label to A (A enters assignment; X did not),
and updated the raw fusion label for the new glyph slots. Final Assign-label
change is built but not recaptured.

MSVC Release passed. 68/68 built CTests passed under VsDevCmd; three unbuilt
vendor tools excluded and playTests disabled. Initial full invocation lacked
compiler headers; corrected toolchain rerun passed. Options 4/4, fonts 2/2,
launcher 58 checks. Paired v7 EXE/data staged at C:/Games/MUA2; 55 saves unchanged.
Root remains OpenMUA2.exe, GameData, saves. No gameplay auto-launch/warp.
Powers allocation/refund, redistribution and hero cycling remain open, so TODO
section 2 is not closed. No audio/FPS validation claimed.
Evidence: evidence/windows-20261007/MENU-LAYOUT-20261008.json.

## Section 1 closed: loading tip corrected - 2026-10-08

The last open section1 item was a loading-screen tip. It now describes the
verified RT + A/X/B/Y power controls instead of unsupported LB + D-pad quick
assignment. Three loose tables and two embedded copies validated; all other
archive members unchanged. Controller/prompt tests2/2 passed. User explicitly
ended further screenshot pursuit: this specific tip was not captured in-game.
Only the private development WAD was updated; installed package unchanged.
Runtime stopped. Sections2/3 remain deferred.
Evidence: evidence/windows-20261007/LOADING-TIP-FIX-20261008.json.

## PS2 held-power prompt fix - 2026-10-08

Corrected the shield HUD diagnosis: its D-pad symbol was a samepowerhold hint,
not a stick-steering instruction. PS2 executable mapper0x4b6c10 selects the
power face button. Shared guarded Xbox resolver now uses A/X/B/Y for power
slots1/2/3/4, preserving the native static suppression and unrelated prompts.
No input or proprietary data changes. RT+A plus opposite left-stick inputs
steered the live shield; reviewed captures show Xbox A during the throw and
Captain America portrait restored after release. Windows Release build and
focused prompt test passed. PS2 itself was not run; installed package unchanged.
Evidence: evidence/windows-20261007/SHIELD-POWER-PROMPT-FIX-20261008.json.

## Profile-ready Xbox Start glyph fixed - 2026-10-08

Legacy MenuExit token now resolves to the existing Xbox Start/Menu font cell
only with direct provider and established Start-accept context. No input mapping
changed. Windows Release build passed without printed warnings/errors; focused
prompt test1/1 passed. Fresh one-controller headless launch, native ready-screen
capture reviewed: Xbox Menu beside Start Game, B Back intact. Start reached the
normal overwrite confirmation; no overwrite accepted. Installed package unchanged.
Evidence: evidence/windows-20261007/PROFILE-START-GLYPH-20261008.json.

## Scope correction and turret exit code finding - 2026-10-08

User removed fresh maze-win replay and four-port tests. Gameplay acceptance and
edge-case testing belong to their beta testers. Do not restart those checks,
Nullifier research, broad inventories, packaging or installed-build regressions.

PS2 method0040D1D0 belongs to CCHBFGEnd (vtable007D9E40, RTTI00782EA0,
name00782E80). It resolves category31 and runs the ending transition; it does
not establish a manual-exit input. No speculative button mapping was added.
Automatic turret exit passed previously. Manual exit remains unproven, not
claimed absent. Evidence: TURRET-EXIT-DISAMBIGUATION.json. No runtime change.

## Single-controller scope and current hacking return - 2026-10-08

User excludes all four-port testing from this pass. Remaining work is limited to
one-controller hacking current-build success and PS2 turret manual-exit analysis.
Current runner69bfa1f6 loaded the prior native winning results state (180points,
1000EXP). Start returned to world; left stick moved Captain America from
[2937.368,1635.920,1.252] to [2939.275,1605.379,1.258]. Native result/world
captures reviewed. This verifies return/movement, NOT a fresh current-build win.
PS2 turret update40C650..40CEEC traced farther; no manual-exit binding proved.
Do not invent one or close that question based solely on the aiming handler.
Runtime stopped; no source/build/installed or proprietary-data changes.

## Current scope and four-player hacking checkpoint - 2026-10-08

User narrowed this pass to remaining hacking and turret checks only. Do not
pursue Nullifier research, a broader interaction inventory, packaging, installed
regressions or physical-controller acceptance in this pass.

Four-port hacking steering isolation and scheduler release passed. Native
signals map ports0..3 to green/red/yellow/blue. Four-player FAILED results and
port0 Start retry to the Xbox A/left-stick tutorial were visually reviewed.
Multiplayer success/return and four-player turret ownership remain pending.
Marker overlapping tutorial text remains a deferred overlay issue.
Harness correction: release=0 retains input across command gaps; release=1
zeros values at the scheduled time and passed this four-port release test.
Do not repeat the unsupported blanket claim that release=1 disconnects pads.
Runtime stopped, private WAD restored byte-exact, installed game unchanged.
Evidence: evidence/windows-20261007/FOUR-PORT-HACKING-20261008.json.

## Four-port lock-on encounter passed - 2026-10-08

Four Guest profiles selected/confirmed through native new-game UI before the
private Chemical Plant checkpoint route. All four actor owners0..3 remained
AI=false after loading. Synthetic pads must remain connected between commands:
release=0 neutral; releasing their override caused a real disconnect dialog.
No actor owners or join flags were assigned by the harness.

XMLB-validated tank placement, then native movement/use after the boss intro:
Player2 repeated X completed the tank. Each controller port0..3 independently
cleared the active A lock-on target; LB left it intact. Player4 cleared the
remaining B targets and native boss HP fell75600->54600. Native captures reviewed:
tilted X, gamepad instruction, readable B timing ring, and world combat after win.
Player3's hero was KO before lock-on; its joined controller still participated.
No HP, target assignments/timers or completion writes. Evidence:
FOUR-PORT-LOCKON-20261008.json. Runtime unchanged from prior69bfa1f6 build.

The retained chemical-four-joined.sav is during the boss introduction; wait for
it to end before use. chemical-four-lockon-instruction.sav and
chemical-four-lockon-active.sav preserve the actual four-player interaction.
Private WAD restored to6c3f21c07c4c386c2f61b2a11820a4b5b028138b8a1e997ac444ce043254feec;
runtime stopped, installed game/saves untouched. Earlier standalone zoneinfo
start-map experiment loaded Latveria and was restored; do not repeat it.
Quitting to main menu clears party; select four profiles before direct map route.

Next: four-player turret/hacking via the same native profile-first fixture;
then remaining gameplay inventory and reviewed installation/source integration.
Keep deferred menu overlays separate. Profile Start prompts on additional ports
still show Wii plus; note for UI pass. No physical-controller/audio/FPS claim.
Temporary dump cleanup remains blocked by the earlier automatic approval review;
no retry through another method. Goal remains active.

## Lock-on prompt readability fixed - 2026-10-08

Removed the colored star model covering lock-on face glyphs through the shared,
hash-guarded gamepad target transform. Native timing ring, target lifetime,
button assignment and completion remain intact. Native screenshots reviewed:
Y is readable; pressing Y returns to combat. Windows Release moderngekko-run
rebuilt successfully without printed compiler/linker warnings/errors. Four
focused control tests passed. Evidence: LOCKON-VISUAL-20261008.json.

Post-dispatcher-fix hacking regression: all four stick directions change the
native aim record with correct opposite signs; neutral is zero. Native screenshot
shows signal movement and subsequent collision/FAILED results with Xbox Start.
Full hacking success was previously tested but not repeated on this build.

Four-player encounter ownership remains unverified. Boss checkpoint has player
manager byte40=0x70: join handler80132E20..28 requires bit0x80, so exits before
reading Start. Do not force join flags or repeat Start in that checkpoint.
Next: establish native joins before the boss transition, then carry the joined
party into the XMLB-targeted encounter. No statue/fusion or campaign traversal.
Runtime stopped; installed game and real saves unchanged. Private WAD unchanged.

## Native lock-on reached and dispatcher fixed - 2026-10-08

Supersedes the tank-entry diagnosis below. Tank heaviness2 rejects Captain
America (strength0). Iron Man has strength2; selecting him, placing behind the
trigger and walking into use range starts generic_sequence. Existing repeated X
completes the tank. No input-binding or strength bypass was needed.

Live lock-on then exposed stale PPC state.pc in external hook callbacks under
JIT. HLE dispatcher now synchronizes it to the explicit instruction address for
external branches/observers. This shared fix activates the guarded gamepad path.
Native instruction now says Press the buttons on the target. A/B/X/Y each cleared
their assigned native targets across [X,A,A] and [B,X,Y] sequences. LB did not
clear X. Boss HP48000 ->43200 (tank) ->31200 (lock-on win); timeout left43200.
Movement returned after success. No HP, timers, target assignments or win flags
were edited. Y glyph is partly covered by a star effect and still needs visual
correction. Unjoined port1 did not affect targets; joined four-player validation
remains open, not replaced by that check. Shared aiming needs a post-fix regression.

Windows Release rebuild completed without printed compiler/linker warnings or
errors. 68 tests passed; three dependency binaries (fullbench/fuzzer/zstreamtest)
were unavailable and playTests disabled, so full CTest invocation returned nonzero.
Runner SHA256 11c69585d846c32340ecba0f22384663d92beb094c39c4dd6af4ba8e22ddec66,
16226304 bytes. Installed game/saves unchanged; private WAD remains restored.
Runtime stopped. Evidence: TARGETED-LOCKON-20261008.json.
Next: inspect the star covering Y from chemical-lockon-y.sav, then joined-player
ownership and shared aiming. Do not repeat statue/fusion or campaign traversal.

## Targeted Chemical Plant checkpoint - 2026-10-08

Boss lock-on is NOT passed. XMLB identifies the fuel-tank co-op interaction as
its prerequisite. The authored boss checkpoint startup path was used in the
private diagnostic route; native boss introduction, fuel tanks and Hank Pym HP
bar appeared. Direct placement at the earlier trigger had failed to activate it.
X Throw Fuel Tank displays correctly and X runs fade_safeguards, but both
Captain America and Iron Man remain in idle with no co-op target. Spreading
allies to remove collision overlap did not resolve entry. No QTE progress,
boss HP, target timers or completion callbacks were changed. Lock-on itself
has not yet been tested. Next: trace tank acttargets -> CCoopEntity activation
-> generic_sequence using chemical-tank-before.sav; do not revisit statue/fusion.

Runtime stopped. Private route restored to original SHA256
6c3f21c07c4c386c2f61b2a11820a4b5b028138b8a1e997ac444ce043254feec.
Installed game/saves unchanged; no new source build in this checkpoint.
Retained chemical-boss-checkpoint.sav and chemical-tank-entry-failure.sav as
reproduction points. Evidence: TARGETED-CHEMICAL-20261008.json.

## Targeted encounter continuation - 2026-10-08

Supersedes the incomplete turret status below. Port0 completed both native turret
objectives, the ending cutscene released the turret, and left-stick movement
worked afterward. No enemy HP or completion flags changed during turret combat.
The harness needed the authored projectile-origin offset when choosing its aim;
runtime aim code did not change. Four-player encounter ownership remains pending.

Hacking reached through the authored armory_terminal in
act2/rebelhideout/rebel_anti_2. Diagnostic fixture enabled only the terminal to
bypass the boss prerequisite; this does not prove campaign access. Native hacking
instructions show left-stick wording, A dismisses them, and steering moves the
signal. A wall collision produced the native FAILED results. Start and A could
not leave that screen: CW_Results_Hack (0x81194460) requires MENU_OK, which was
missing from the shared Start-continue context. Added the guarded context and a
regression. Windows Release rebuild passed (5/5 focused, 50/50 supported tests). Native
results now show Xbox Start; Start retries successfully. The maze was completed
using stick input and save-state retries: 180 points, +1000 EXP. Start returned to
gameplay and movement worked. No collision, speed, score or win flags modified.
Four-player ownership and physical-controller acceptance remain pending.
Native signal/marker overlaps the instruction panel; record as unresolved visual
layering alongside the deferred menu overlay issue.
See evidence/windows-20261007/TARGETED-HACKING-20261008.json.

Private new-game route has been restored byte-for-byte to WAD SHA256
6c3f21c07c4c386c2f61b2a11820a4b5b028138b8a1e997ac444ce043254feec.
Installed game and saves untouched. Diagnostic states hacking-tutorial.sav,
hacking-active.sav, hacking-failed.sav, hacking-result-final.sav and hacking-complete.sav are in the existing workspace
work/campaign-progress-20261008. No OS input, audio or physical-controller claim.


## XMLB-targeted turret validation - 2026-10-08

User explicitly authorized warps to authored map encounters, superseding campaign
traversal. Added tools/warp_gameplay_encounter.py. It reads private XMLB and
validates live target name/position, moves four party actors through process-local
checked writes, records receipts and leaves the runtime paused. No host input.

Storm Castle prerequisites used a diagnostic 1-HP-generator fixture, destroyed
through normal attacks. X mounted the enabled turret. Found and fixed remaining
Wii wording in both shared turret tutorial branches and the mounted HUD. Fresh
entry screenshots were inspected: left-stick aim and Xbox A/B fire are correct.
A/B firing and stick aim destroyed all six original-HP turrets; native objective
reported Covering Fire: Turrets COMPLETE. Doombot phase/final release remain
unverified after a diagnostic entity-arena failure. No manual exit binding
accepted. Do not claim full encounter or four-player acceptance.

Windows Release build passed, focused4/4 and supported50/50 passed; no printed
warnings. Runtime SHA25672f205f86322114ea3b9759139f58c2737a5215392b2740f411a0b2b1d1f001f
(16,226,304 bytes). Installed EXE and user saves unchanged; no repackaging.
PID32304 paused at TRIG_turretEvent. Workspace work/campaign-progress-20261008/
turret-before-use.sav preserves targeted entry; campaign-current.sav preserves
natural progress separately (updated hash in evidence). Finish turret release,
then target hacking/other gameplay interactions using XMLB; no statue/fusion
replay. See evidence/windows-20261007/TARGETED-TURRET-20261008.json.


## Normal campaign progression authorized - 2026-10-08

User authorized normal progression to the later encounters, superseding the
route-scope blocker below. Loaded the preserved natural campaign state on the
reset-fixed runtime. Repeated X presses completed the required statue objective;
native capture confirmed Hero Training: Might (COMPLETE). Continued through
adjacent streets using contained process-local input. No later map transition or
turret encounter reached yet; do not treat navigation as encounter acceptance.

PID13876 is paused. Current natural progress is saved at workspace-relative
work/campaign-progress-20261008/campaign-current.sav (not yet reload-verified).
Installed saves and EXE are unchanged. Turret, hacking, lock-on, packaging and
physical-controller acceptance remain open. Audio was Null and is unverified.
See evidence/windows-20261007/CAMPAIGN-PROGRESSION-20261008.json. Continue from
this checkpoint; do not repeat completed statue/fusion proof audits.

## Encounter validation blocked - 2026-10-07

Rechecked installed saves: Game1 is the 00:01 Latveria auto-save; Game2..10
are blank. No mounted/later normal encounter checkpoint is available. The same
route constraint has persisted through the recent audit/review turns. Runtime
PID21228 stopped with exit0 and its unique parked state is retained. User asked
to leave statue/fusion work parked; do not restart that route without resolving
scope. Need a later normal save or a route decision permitting ordinary campaign
progression to the required encounters. Static checks do not replace live proof.
Packaging/install validation, final reviewed push and physical acceptance remain
outstanding. See evidence/windows-20261007/ENCOUNTER-VALIDATION-BLOCKER.json.

## Lock-on runtime restart flags repaired - 2026-10-07

Runtime::Run now resets lock-on active/registration flags with other controls.
Core shutdown clears HLE hooks; stale flags could otherwise skip reinstall on a
second run in the same process. Initial compilation passed but link failed
LNK1104 because parked PID21228 held the runtime open. Saved that unique run to
work/native-campaign-validation/normal-campaign-parked.sav and stopped it (exit0).
No statue/fusion replay performed. Installed saves untouched.

Retry Windows Release build passed without printed warnings. Focused tests4/4,
supported tests50/50 passed. Runtime SHA256:
a7e889a52dc3144ac17e4c03ab288017a01fdd4c8502421b8d5c5fa7d729353d
(16,225,792 bytes). Existing private v6 launcher still embeds the prior runtime;
installed EXE unchanged. Two complete runs in one process and actual lock-on
encounter remain unverified. See LOCKON-RUN-RESET.json in evidence/windows-20261007.

## Packaging preflight and encounter checkpoint - 2026-10-07

Build-Launcher now validates icon/optional UI manifest before creating its large
payload archive. Missing-icon and missing-manifest checks both failed early,
with identical temporary-package directory inventories before/after. No runtime
or installed changes, and no rebuild needed for this preflight-only edit.
See evidence/windows-20261007/PACKAGING-PREFLIGHT.json.

Retained turret checkpoint inventory found only the known KeepTeam entry;
no mounted or normally progressed turret state. Asked whether a later normal
save exists. Live encounter acceptance remains open; do not repeat static
audits as a substitute or return to statue/fusion without a route decision.

## Turret analog and tutorial code audit - 2026-10-07

Verified PS2 CCHWeapon stick rotation constants/deadzone/limits against the
candidate helper and original binary. The exact-hash turret instruction hook
selects shipped analog-stick text at80563D38, skipping pointer text at80563DC4.
This verifies code selection only: no live turret tutorial, firing, exit or
four-player acceptance. Do not infer an exit button from unrelated handlers.
See evidence/windows-20261007/TURRET-ANALOG-PROMPT-AUDIT.json.

## Scripted turret identity and private package checkpoint - 2026-10-07

Read-only authored map audit distinguishes ordinary use-activated turrets from
mm_turret, whose actonuse=false and dropscript selects turretevent_dropturret.
Validate the actual scripted encounter; a generic turret test cannot establish
its entry/exit behavior. No new exit binding inferred or source patch applied.
Statue/fusion replay remains parked; turret/hacking/lock-on retain priority.
See TURRET-LIFECYCLE-AUDIT.json in evidence/windows-20261007.

Previously built private v6 candidate is retained and its SHA256/size rechecked:
4b566c4e669e820aaf7b5d414e6d37b341582ab5e522d43bcdd012e58fc4af4e,
10,636,800 bytes. Packaging checks previously verified 2,634 payload members
and asset verifier version6. This is not installed or gameplay acceptance.
Installed EXE/data/saves unchanged. See XBOX-UI-V6-CANDIDATE.json.
Automatic cleanup review rejected deletion of two package temporary directories
with only "blocked by policy"; retained, not retried through another mechanism.

## Gameplay priority correction and turret exit audit - 2026-10-07

Stop replaying statue/fusion checks; user reaffirmed turret/hacking/lock-on
priority. PID21228 remains paused near statue after Spider-Man double jump;
do not continue that detour. Turret exit remains unresolved. PS2 text saying
Block to Exit belongs to CCHTurnObjSequence, a different handler (vtable
007DA250), whose code consumes61/62 for turn and12 for exit. This is not
turret evidence. Actual usage/Wii parity of that family must be established
before implementation. No speculative binding change or installed update.
Evidence: evidence/windows-20261007/TURRET-EXIT-DISAMBIGUATION.json.


## Normal-campaign fusion completion verified - 2026-10-07

PID21228 reached plaza naturally. Native tutorial shows LT + A/B/X/Y.
Port0 LT+B input selected Captain America/Iron Man; actual completion banner
shows Beam Split, 1294 total damage and10 total KOs. No seeded positions,
resources, eligibility or progression writes. Input sequence includes retries;
exact initial latch timing not isolated. One pairing only; physical controller,
statue QTE and later gameplay interactions remain open. Installed unchanged.
PID21228 paused in plaza after completion, all heroes alive.
Evidence: evidence/windows-20261007/NATIVE-CAMPAIGN-FUSION.json.


## Normal-route powers tutorial inspected - 2026-10-07

Continued PID21228 through street combat using process-local input only.
Native powers tutorial visibly shows RT + A/X/B/Y for powers1/2/3/4,
matching provider action order. This verifies the prompt in context, not
execution of all four powers. All heroes alive; paused beyond tutorial at
Cap position approximately [786,126,1]. No forced state/progression writes.
Installed game/saves unchanged. Later gameplay interactions remain open.
Evidence: evidence/windows-20261007/NATIVE-POWER-TUTORIAL.json.


## Native player action activation verified - 2026-10-07

Unseeded campaign: B enters CCHHoldSmash, RT+A enters
CCHReturningProjectile; both return to CCHIdle with cleared input on release.
Native capture shows Captain America throwing his shield. A changes to a
distinct generic combat node; exact authored name not decoded. Damage
attribution, other powers/players and physical input remain unverified.
PID21228 paused in street combat. Installed game unchanged.
Evidence: evidence/windows-20261007/NATIVE-PLAYER-ACTION-PROOF.json.


## Normal campaign validation reset - 2026-10-07

Retained statue/fusion states reviewed have seeded positions/resources and
cannot prove normal campaign progression. Restored original New Game script
in existing private WAD, checked against authoritative extraction; all other
members unchanged. No extra WAD retained, installed data/saves untouched.
Cold-start PID21228 uses work/native-campaign-validation, no loaded state or
gameplay writes. Normal title/main/Guest/autospend flow reached original
Latveria intro and initial street with four heroes alive. Continued into
first Doombot combat: A/B attack tutorial visibly correct, mixed player/AI
damage and downed enemies observed; isolated attack/power proof pending. Native captures
inspected. PID21228 paused in initial street combat; continue rather than restart. Physical
controller, audio, combat and later-interaction acceptance remain pending.
Evidence: evidence/windows-20261007/NATIVE-CAMPAIGN-FIXTURE.json.


## Retained turret fixture progression confirmed - 2026-10-07

Native Hero Details confirms Captain America and Iron Man are level1 with
zero spare points and one available power each. This KeepTeam fixture skipped
prior campaign; do not repeat it as normally progressed combat acceptance.
Enemy scaling and defeat cause remain unproven. Installed save headers show
only an early Latveria autosave; others blank. No saves changed. Audit process
exited0. Evidence: evidence/windows-20261007/TURRET-PROGRESSION-AUDIT.json.


## Storm Castle contained route check - 2026-10-07

Normal generator objective display and Iron Man flight were visually checked
using native captures. Double-Y and Y/LB flight prompts observed; camera-relative
route changes prevented reaching the turret. No turret acceptance or v6 HUD
acceptance claimed from this older saved state. Follow-up combat again
failed to reach generators; PID35908 stopped normally, exit0. Audit native
progression/power availability before repeating this fixture. Timing receipts
confirm synthetic presses/releases, not combat or physical acceptance.
Nullifier usage corroborated by online PS2/Wii transcript; unique mechanics
remain unverified. Installed game and actual saves unchanged.
Evidence: evidence/windows-20261007/TURRET-ROUTE-R3.json.


## Turret handler identity confirmed - 2026-10-07

PS2 RTTI/vtables confirm CCHBFG and CCHBFGFire share update0040C650,
the reference used for direct rotation. Both use shared action dispatch.
Action21 reaches a guarded drop routine, but that is not sufficient proof
of fffgunexit; do not claim X exit verified. No speculative binding change.
Updated TURRET-LIFECYCLE-AUDIT.json with addresses and remaining trace.


## Turret lifecycle audit - 2026-10-07

Read-only authored-data audit confirms west then east generator destruction
enables the Storm Castle turret event. The activating hero owns the gunner
role; native scripts handle camera, collision, exit/re-entry and completion.
Fightstyle chains attack/smash to firing (current A/B); this is source evidence,
not gameplay acceptance. Exact exit input remains to trace. Preserve native
gating and lifecycle during validation. No installation or gameplay changes.
Evidence:evidence/windows-20261007/TURRET-LIFECYCLE-AUDIT.json.


## Paired gameplay package version prepared - 2026-10-07

Xbox UI v6 pairs shared HUD artwork with local aiming and lock-on launcher
switches after asset-hash verification. Older versions retain prior behavior.
Compiler rejects shared HUD without full Xbox UI and rapid-tap assets.
49 launcher checks and2 atlas checks passed; incomplete CLI rejected.
Packaging source only, not installed. Actual encounters remain unverified.
Evidence:evidence/windows-20261007/XBOX-UI-V6-PACKAGING.json.


## Lock-on prerequisite correction - 2026-10-07

Candidate activation now requires successful direct-QTE hooks, matching the
shared prompt renderer prerequisite. Previously partial QTE registration
failure could enable X target consumption without the static-X draw override.
Windows build exit0, no compiler/linker warnings/errors; focused4/4(2.03s),
full50/50(9.67s). Cold registration PID41452 confirms complete hook group
and exits0. Failure path source-reviewed, not fault-injected. Encounter
acceptance remains pending; installed game unchanged. Current runner SHA256
4A67B357F172688E2BC13E232A37478C8D933622D50B15DE829739AAD6123C33.
Evidence: evidence/windows-20261007/LOCKON-PREREQUISITE-FIX.json.


## Private load and flight validation - 2026-10-07

PID29748 cold-started and loaded the private Punish Pym autosave via normal
menus. Native captures verify Xbox A/B load prompts and Iron Man flight
with Y/LB tutorial glyphs. Continuous double-Y activated flight; Y raised
altitude and LB returned him to the walkway. Initial separated diagnostic
steps delayed the double tap; not a confirmed binding defect. Yard enemies
damaged Iron Man before safeguard; lock-on/Nullifier/turret remain unverified.
Process stopped cleanly (exit0). Installed game and actual saves unchanged.
Evidence: evidence/windows-20261007/CHEMICAL-FLIGHT-VALIDATION.json.


## Chemical Plant native entry tested - 2026-10-07

Candidate cold-start navigated title/main/Begin Story/Easy/Guest/Autospend
through process-local input; native captures inspected. Entered Chemical
Plant and traversed walkway/stairs; D-pad Right selected Iron Man. Yard
fire/enemies downed Iron Man before safeguard. Final image shows fire
exposure; no proven level-scaling defect or lock-on acceptance. Process
PID33784 exited0. Private chemical route + paired HUD remains selected;
receipt workspace work/rapid-tap-game/chemical-route-receipt.json is current.
Actual installed game/saves unchanged. No new savestate created. Profile
Ready still shows Wii + for Start Game; flagged in TODO, deferred.
Evidence: evidence/windows-20261007/CHEMICAL-NATIVE-ENTRY.json.

Read-only follow-up: Iron Man final XY lies inside authored HARM_ALL_FIRE_07
footprint at2296.63,-299.076. Private native autosave is Punish Pym GUEST(E),
act2/chemical_plant/chem_anti_2. Reload through Load Game and avoid that
hazard; do not infer hero-level mismatch or repeat the prior movement route.


## Lock-on gamepad candidate built - 2026-10-07

Opt-in OPENMUA2_GAMEPAD_LOCKON=1 restores PS2 target actions A/B/X/Y,
assignment/drawing/consumption and instruction4 gamepad wording as one
six-hook group. Shared block remains LB; lock-on grab uses static-X sprite99.
Native timing, callbacks and four-port input loop remain. Requires paired
Xbox HUD pack; not enabled in installed launcher. HLE address-map budget
audited and increased32->64; no opcode/array capacity change. Windows build
exit0, focused4/4(2.33s), full50/50(11.04s), no build warnings/errors.
Headless registration PID13740 activated all hooks with shared aiming,
loaded retained prison state and exited0. This verifies registration only;
actual lock-on encounter and in-game prompts remain unverified.
Evidence: evidence/windows-20261007/LOCKON-CANDIDATE.json.

Paired private HUD archive prepared and verified2026-10-07: only the11
shared button cells changed; all other member/package bytes preserved.
Private WAD now532c8f5654f41601947c54252fcf24a0b1169fcae707856ec8fd876009027260;
older prison-route WAD hash is historical. Prison route still selected.
Chemical Plant native win_boss_safeguard script activates lock-on after
the safeguard succeeds; use that progression, not a forced lock-on call.
Installed game unchanged. Evidence: evidence/windows-20261007/LOCKON-PAIRED-HUD-PREPARED.json.


## Lock-on button and prompt mismatch traced - 2026-10-07

PS2 target table is [9,10,11,19] (current Xbox A/B/X/Y); retained Wii
table is [9,10,12,19] (A/B/LB/Y). Three separate network-mask gates
control assignment, rendering and consumption. Enabling consumption alone
would not restore the PS2 path. Native draw IDs38/44/40/41 use HUD provider80FA770C and
atlas texture slot1, separate from the Xbox menu font. PS2 draws38/39/40/41.
Restore shared Xbox HUD artwork and routing together; changing the font
alone cannot fix this encounter. Native target timer and success/failure dispatch
identified and must remain intact. Audit evidence updated; no runtime or
installed build changed, no new gameplay acceptance.
Evidence: evidence/windows-20261007/LOCKON-GAMEPAD-PATH-AUDIT.json.

Shared HUD button compiler now has opt-in --xbox-hud-buttons for11 A/B/LB/Y
frames. Tests2/2 pass against stock coordinates and byte-preservation checks.
Decoded atlas inspected; shared scale-up rule fixes small LB artwork. This
is asset-only, not installed or in-game acceptance. Lock-on X routing and
full coherent gamepad activation remain pending.
Evidence: evidence/windows-20261007/SHARED-XBOX-HUD-BUTTONS.json.


## Lock-on gameplay path identified - 2026-10-07

CLockonEntity uses instruction4 and CHudTargetPoints. Authored lockonent
entities and win/fail scripts exist in both Chemical Plant boss routes and
Wakanda Man-Ape sequences. PS2 update004CE960 tests target-assigned buttons
for ports0..3. Native8024F408 contains the corresponding button consumer,
but entry8024F43C queries network mask2 before choosing it. Full lock-on
coverage is NOT established by shared aiming changes. Next audit button
generation/rendering, lifecycle and ownership before enabling local path;
never spoof network flags or force outcomes. No build/install changes.
Evidence: evidence/windows-20261007/LOCKON-GAMEPAD-PATH-AUDIT.json.


## Prison walkway combat evidence - 2026-10-07

Bounded actor probe now completes. Native capture identifies immediate foes
as Light Nanite Mutants (Endopsychic Tether), not established to be the three
guards gating HackPanel. Two heroes fell before reaching the interaction.
Nearby enemies took damage during mixed player/AI combat; this is not isolated
attack-binding proof. No native hero level measured; difficulty mismatch is
unproven. Installed save read-only header is Latveria S, act1/latveria/latveria1.
Asked about a later campaign save; avoid repeating this approach unchanged.
Process exited0; no installed changes, new states or duplicate data/builds.
Evidence: evidence/windows-20261007/PRISON-WALKWAY-COMBAT.json.


## Prison diagnostic failure - 2026-10-07

Actor-name read computed an invalid address; command channel then stopped
completing valid reads. Test process explicitly terminated (exit4294967295).
No gameplay acceptance. Workspace read helper now rejects requests outside
mapped RAM before publishing; four invalid-range checks pass. Next use bounded
numeric actor records, never unverified tags as string handles, from retained
prison entry. No source build/install changes or additional states this run.
Web playthrough/transcript also confirms the Nullifier beam is used against
Absorbing Man in MUA2's last-generation version.
Evidence: evidence/windows-20261007/PRISON-PROBE-FAILURE.json.


## Interaction-instruction aim reset restored - 2026-10-07

PS2's second reset predicate is CHudPointerError visibility: this object is
also the interaction-instruction panel used by turret and Nullifier callers.
Native getter80FB9030 and loaded object812D9EA0/vtable80564248 identify the
matching panel. Candidate now centers each sampled aim while it is visible,
preserving validity and projection data; normal accumulation resumes on close.
No panel activation/confirmation/completion is forced. Windows build exit0;
focused4/4 in2.11s, full50/50 in9.90s; no compiler/linker warnings/errors.
New branch still needs live encounter validation. Installed game unchanged.
Evidence: evidence/windows-20261007/GAMEPAD-INSTRUCTION-CENTERING.json.


## Gamepad projection aspect corrected - 2026-10-07

PS2 projection calls use selector0 (display aspect/scale); the candidate
incorrectly selected Wii mode1 fixed-aspect/alternate-scale branches. Changed
three guarded branches to native zero-selector instructions, preserving all
projection/collision math. All three60-byte hashes and target instructions
verified against linked code. Windows build exit0; focused4/4 in2.07s, full
50/50 in8.54s, no compiler/linker warnings/errors. Encounter targeting still
unverified; installed game unchanged. Validate this candidate before acceptance.
Evidence: evidence/windows-20261007/GAMEPAD-PROJECTION-ASPECT.json.


## Full-aiming prison approach - 2026-10-07

Restored the prison entry with full shared aiming and turret hooks installed,
extra profile tracing off. Process-local movement and attacks reached combat;
native captures inspected. Guard prerequisite remains uncleared; team damaged.
No hacking/Nullifier/turret operation acceptance. Process exited0. Existing
entry retained, no extra states/builds/GameData copies; installed game unchanged.
Continue normal route/combat from entry without forcing terminal eligibility.
Evidence: evidence/windows-20261007/PRISON-FULL-AIM-APPROACH.json.


## Gameplay priority and profile diagnosis correction - 2026-10-07

Gameplay progression (turrets, hacking, Nullifier, QTEs) precedes menu polish.
The prior profile-blocker diagnosis is superseded: native capture showed an
Autospend modal omitted by top-level menu metadata. A accepted it and loaded
the prison introduction. All three diagnostic hooks registered and native
parent input dispatch executed. No remapping fix was required.
Windows build exit0, focused4/4 in2.29s; previous full50/50 in11.90s predates
the registration-log-only change. This diagnostic run disables aiming for
hook capacity and is NOT interaction acceptance. Installed game unchanged.
Next retain a normal prison entry and validate with full aiming enabled.
Evidence: evidence/windows-20261007/PROFILE-AUTOSPEND-CORRECTION.json.


## Profile dispatcher diagnostic - 2026-10-07

Retained-state modal mode27452 and network flags1864 both read0; neither
explains the missing profile handler query. Added default-off bounded trace at
81000A90,80F94214,80FC8738, exact hashes, disabled alongside full aiming.
Windows Release exit0; focused4/4 in2.45s, supported50/50 in11.90s; no compiler/
linker warnings/errors. Three native diagnostic processes exited0.
No profile-dispatch entries emitted. Loaded code hashes match; registration
success not logged, so empty trace is INCONCLUSIVE. Verify registration next
before concluding these sites are unreachable. No mapping fix or acceptance.
Installed game unchanged; no new states/build trees/GameData copies.
Evidence: evidence/windows-20261007/PROFILE-DISPATCH-GATE-AUDIT.json.


## Profile button delivery narrowed - 2026-10-07

Scoped native trace confirms A89 and Start105 reach generic dispatch, and A89
reaches the profile GUI consumer. Port0 profile pane is page2; others page4.
Parent profile handler80F94214 should query Start105 at80F944B0, but that caller
is absent from captured queries. Next inspect parent input dispatch/gates.
Do not assume a missing button mapping or force profile readiness.
Two full-aiming trace configurations exhausted32 observer/branch slots and
rejected core input hooks; those runs are invalid acceptance evidence. Scoped
trace disabled aiming to free slots, had no rejected hooks and reproduced the
failure. Any fix must pass again with full aiming and no extra trace.
All four processes exited0; no source/build/installation change. Existing exact
profile state retained, no additional states. Goal incomplete.
Evidence: evidence/windows-20261007/PROFILE-ACCEPT-CONSUMER-TRACE.json.


## Prison encounter entry exposed profile blocker - 2026-10-07

Selected prison_break_4 for native hacking/Nullifier acceptance: three guard
deaths enable HackPanel; normal completion leads to Nullifier. Private KeepTeam
new-game destination changed only; no eligibility/completion state forced.
Single-core native run exited0 but stopped at four-slot profile screen (Guest).
Repeated A, held A and Start did not progress. Title/main/play/difficulty did
respond. Native title/profile captures inspected; no hacking encounter reached.
Retained one exact profile repro state paired with prison-route WAD receipt.
Do not load older Storm Castle state until restoring its matching WAD.
Next trace native profile accept/ready consumers; cause not established.
No source/build/installed change this checkpoint. Goal incomplete.
Evidence: evidence/windows-20261007/PRISON-ENTRY-PROFILE-BLOCKER.json.


## Hacking entry reset candidate - 2026-10-07

Implemented reset on the native transition into mazehack, using the game's
mode setter and identifier. Centers all four targets once per entry; preserves
validity/projection and original setter execution. No host history or forced
interaction state. Exact code hash guards the observer.
Windows Release exit0, focused4/4 in2.40s, supported50/50 in10.53s; no compiler/
linker warnings/errors. Regression covers entry/re-entry, sustained steering,
exit, invalid context and all four records. Native single-core run exit0;
all six aiming hooks registered and accumulated aim/release/isolation passed.
Actual hacking entry and completion NOT exercised; gameplay acceptance remains
open. Second PS2 reset predicate and projection parity still need tracing.
Not installed/staged/pushed. Evidence: evidence/windows-20261007/HACKING-ENTRY-RESET.json.


## Gameplay aiming lifecycle audit - 2026-10-07

Found a missing PS2 behavior: shared aiming centers on entry to mazehack.
Current accumulated-stick candidate omits that transition reset; hacking is
not ready for acceptance. Mode setters on both platforms only store mode,
so they do not supply the missing reset. A second PS2 reset condition and
projection-dimension parity require tracing before changes. Wii special-mode
getter identified from the actual DOL vtable; no state was forced.
No runtime edit, rebuild, gameplay run, installation or push this checkpoint.
Next implement equivalent lifecycle behavior and verify real interactions.
Evidence: evidence/windows-20261007/GAMEPAD-AIM-LIFECYCLE-AUDIT.json.
Goal remains incomplete; gameplay blockers retain priority.


## Hacking instruction wording - 2026-10-07

Four hardcoded hacking descriptions/tutorials now say left stick, using the
existing bounded, exact-hash text handler only while candidate gamepad aiming
is active. Removed pointer/Nunchuk centering requests; retained power-up tokens
and original pause-reset warning. No proprietary original file changed.
Windows Release exit0; focused4/4 in2.40s; supported50/50 in8.15s; no compiler/
linker warnings/errors. Two native single-core runs exited0: all4 replacements
verified in live memory when enabled; all4 original hashes verified disabled.
These are text-allocation checks, not visual or actual hacking acceptance.
Screen layout, interaction lifecycle, turret/Nullifier and four-player gameplay
remain pending. Not installed/staged/pushed. Goal incomplete.
Evidence: evidence/windows-20261007/HACKING-GAMEPAD-TEXT.json.


## Nullifier analog instruction; hacking audit - 2026-10-07

Private candidate now selects the shipped Nullifier analog-stick tutorial,
using the same guarded instruction callback as turret with separate native
call sites. No tutorial identity/owner or NetPlay state is changed.
Windows Release exit0; focused4/4 in2.27s; supported50/50 in8.02s; no compiler/
linker warnings/errors. Native run exit0, both tutorial hooks registered, and
accumulated-aim record probe passed again. Tutorial rendering and real encounters
remain unverified. Not installed/staged/pushed.
PS2 hacking code uses owner-indexed shared aiming and four participant records.
Correction:800E9588 is hack availability, not an input-mode selector; preserve
its network restriction. Wii-only hacking wording remains in hardcoded strings
(805390CE,805391A0,805638AF,80563A5C,80563B8C); shared aim does not replace it.
Next trace those instruction sites/lifecycle and validate actual gameplay.
Evidence: evidence/windows-20261007/NULLIFIER-PROMPT-HACKING-AUDIT.json.


## Shared aiming correction with native record proof - 2026-10-07

Fixed two candidate defects: enable guard must check byte5925 bit0x80, not0x40
(native rotate25/mask31); PS2 default mode3 accumulates stick/20 per native
update with +/-0.1 deadzone and [-1,1] limits, retaining target on release.
Previous absolute-axis candidate incorrectly recentered; prior enable analysis
was wrong. Native projection/collision and network flags remain unchanged.
Final Windows Release build exit0, focused4/4 in2.42s, supported50/50 in11.45s;
no compiler/linker warnings/errors. Native single-core process exited0.
Actual owner0 record: 0 ->0.30 ->0.30 neutral ->0.30 unjoined pad1 ->~0 opposite.
Other records stayed invalid. This proves shared input handling in the running
game, not an actual aiming encounter or four joined players. No screenshots,
audio/FPS or physical acceptance this pass. Not installed/staged/pushed.
Next real Nullifier entry/aim/use/exit and prompt; turret/hacking still pending.
Evidence: evidence/windows-20261007/ACCUMULATED-GAMEPAD-AIM.json. Goal incomplete.


## Nullifier PS2 consumer trace - 2026-10-07

PS2 executable code confirms Nullifier uses an owner-indexed shared aiming
record and collision ray, unlike the turret's direct pitch/yaw controls.
The getter and named activation registration support slots 0..3; this is code
evidence, not four-player gameplay acceptance. The PS2 instruction specifies
left analog stick. Next trace the exact record producer and activation lifetime
before accepting or changing the current shared gamepad aim candidate.
Native turret retry cleared the first group but failed in the corridor before
the generator. No turret acceptance. Map has no explicit world level; team/enemy
scaling remains unknown. Process exited 0. No runtime edits, rebuild, installation
or push. Gameplay interactions retain priority; goal incomplete.
Evidence: evidence/windows-20261007/NULLIFIER-PS2-PATH-AUDIT.json.


## Gameplay-first encounter validation - 2026-10-07

Gameplay interactions remain ahead of all menu polish. Continued the private
Storm Castle entry through stairs and corridor using contained Xbox input.
Native captures show real enemy combat; team defeat occurred before reaching
the generator. No eligibility, actor position or completion flags were forced.
This is a failed approach, not a turret pass or an established turret defect.
The reason for defeat (fixture progression versus combat approach) is unresolved.
Process exited 0. No rebuild, installation, proprietary-data publication or push.
Latest candidate remains the prior 50/50-tested build; gameplay acceptance open.
Next inspect native progression/encounter requirements and establish a viable
combat approach, then verify turret controls and instructions in context.
Nullifier, hacking, remaining QTEs and four-pad ownership remain required.
Evidence: evidence/windows-20261007/TURRET-ENCOUNTER-APPROACH.json.


## Direct gamepad turret candidate - 2026-10-07

PS2 CCHWeapon code confirms direct stick-driven pitch/yaw, not a pointer ray.
Added opt-in direct rotation with native game delta and weapon/reset limits,
then the original native rotation setter/cleanup. Selects the shipped analog
turret instruction without changing NetPlay state. Earlier shared-aim candidate
alone is NOT the complete PS2 turret path.
Windows Release rebuild exit0; focused4/4 in2.48s, full50/50 in12.16s, no
compiler/linker warnings/errors. First native run exposed an unsupported hook
site; corrected to the existing call. Second native run confirms both rotation
and tutorial registrations, restores level entry and exits0. No actual turret
operation or in-context tutorial acceptance yet. Not installed/staged/pushed.

The original new-game script reaches its cinematic; the AddTeam diagnostic route
fails. KeepTeam loads Storm Castle with four heroes visibly present. One50MB
entry state is retained. Private test WAD keeps ONLY the diagnostic new-game
script change (plus earlier font candidate) to match that state; original script
is retained for exact restoration. This route is not for the user-facing game.
No turret flags, generator prerequisites or outcomes were forced. Next reach
and test the generator/turret normally, then Nullifier/hacking/four-player paths.
Evidence: evidence/windows-20261007/DIRECT-TURRET-CANDIDATE.json. Goal incomplete.


## Turret encounter fixture attempt - 2026-10-07

Gameplay interactions remain the priority. A cold, single-core private run
reached normal player setup through contained Xbox input, with native captures
inspected. The diagnostic new-game route did not reach Storm Castle: after
autosave/AI autospend confirmation it returned to the ready screen. One retry
and a neutral wait did not advance. Script dispatch was not traced; do not infer
the cause or count this as an aiming failure/pass. No turret was exercised.
Process exited0. The private new-game script was restored and the entire WAD
hash exactly matches its pre-test hash. Installed game/user saves are unchanged.
Next verify map entry/script dispatch, then native turret entry/aim/fire/exit
with original encounter prerequisites intact. No forced gameplay flags or
completion. Nullifier, hacking and four-player acceptance remain outstanding.
Evidence: evidence/windows-20261007/TURRET-NATIVE-ENTRY.json. Goal incomplete.


## Shared gamepad aiming candidate - 2026-10-07

Private opt-in OPENMUA2_GAMEPAD_AIM=1 with the gamepad provider; not installed.
Adds shared stick-mode selection, per-player sampling and consistent native
projection selection without changing CNetPlayManager state. Menu/disabled aim
contexts retain original behavior. Native collision and outcomes remain owned
by the game. Windows Release build exit0, focused4/4 in3.41s, supported runtime/
codegen suite50/50 in10.46s. Initial unfiltered tests outside the MSVC environment
failed on missing vendor benchmark binaries/string.h; corrected invocation passed.

The native plaza probe installed hooks and exited0 but DID NOT exercise aiming:
its native aiming-enable bit was clear and all four records remained invalid.
No context flags were forced. This is not turret/hacking/Nullifier acceptance.
Use the genuine stormcastle_1 turret sequence (generator_destroyed enables
mm_turret/TRIG_turretEvent, turretevent_start enters, dropturret exits) for next
validation; prison_break_4 contains Nullifier and rebelhideout maps contain hack
observers. Source/data inspection is only a route lead, not interaction proof.
Matching gameplay prompts and all four players' live behavior remain pending.
Evidence: evidence/windows-20261007/GAMEPAD-AIM-CANDIDATE.json. No staging/push.


## Aiming dependency correction: network state, not platform - 2026-10-07

Code tracing identifies mask2 of802722DC as CNetPlayManager Game state.
80273AAC sets it in NetPlayStart Game;80273AD8 references the corresponding
RESEEDING log at8056B968. There are149 literal-mask2 callers in the current
combined code. A global override would affect session/network behavior and
ownership; do not use it to enable Xbox controls.

Shared aiming still contains real stick input, but three projection helpers
(80FFB394,80FFB524,80FFB750) independently gate stick scaling on network state.
Selecting manager mode1 alone therefore does not establish correct aiming.
Hacking entry800E3550 selects mode1 and exit800E5DF8 restores0 under the same
gate. The repair needs a shared local-gamepad input-mode policy across sampling,
projection and interaction lifecycle, leaving network state intact. No runtime
patch, build, game launch or installation change in this follow-up. Gameplay
acceptance remains pending. GAMEPLAY-AIM-PATH-AUDIT.json records the correction.


## Shared gameplay aiming path traced - 2026-10-07

Read-only code audit found an executable shared analog aiming branch, not just
alternate text: mode1 reads owning-player actions0/1; pointer mode reads4/5/6.
The native manager bounds ports to0..3 and has four72-byte aiming records.
Nullifier entry/exit selects stick/default mode only behind config mask2; HUD
selection uses the same mask. Do not globally override this flag without auditing
its other consumers. The exact aiming integration and turret behavior remain
unverified. A contained headless run loaded an existing diagnostic state solely
to resolve the live manager vtable, then stopped exit0. No game input, guest
writes, captures, installed changes or gameplay acceptance in this audit.
Evidence: GAMEPLAY-AIM-PATH-AUDIT.json under evidence/windows-20261007.
Next: trace shared mode selection and aiming consumers against PS2, implement
the direct gamepad lifecycle, then verify actual entry/use/exit and four players.


## Gameplay interactions take priority - 2026-10-07

User redirected work to gameplay blockers before menu polish. Next: turret and
Nullifier entry/aim/use/exit, hacking, then remaining motion/pointer QTE families.
Trace the native consumers against PS2 code and verify direct gamepad behavior,
player ownership, release/cancellation and actual outcomes. Matching gameplay
prompts are part of each interaction's acceptance. These paths remain unresolved.
The Hero Management overlay stays deferred. Menu prompt candidate source is
preserved; its Windows rebuild and 50/50 tests passed, but latest visual
verification was interrupted before menu acceptance. Diagnostic process stopped
with exit0. Installed game and user saves are unchanged. The current priority
checklist is in docs/PERFORMANCE-GOAL.md at the repository root.


## Native menu gamepad mapping candidate - 2026-10-07

Private, not installed/pushed. Shared native active-menu routing adds missing
point/profile/hero menu actions from the PS2 initializer groups and suppresses
combat/camera/fusion/QTE outputs while a menu is open. Menu A accepts/assigns,
B backs out, X details/cycle, Y priority, LB/RB remove/add points, LT previous
hero/redistribute, RT next hero. Native consumers retain eligibility/ownership.
Windows Release build exit0, focused3/3 in3.44s, rebuilt action-bindings test
and full50/50 in11.36s. No compiler/linker warnings/errors in build log.
Native saved-menu captures verify RB allocation help, A dismissal/assignment,
B return to powers and Y priority3->0. Zero spare points and auto-spend prevent
spend/refund acceptance; LT redistribution effect and RT hero change unverified.
Prompts still wrong (LB Assign, Wii +/- and1); do not stage this candidate.
Fresh startup/four-port/physical acceptance still outstanding. Overlay issue
remains deferred at user request. See MENU-GAMEPAD-MAP.json. No new forced
menu/state writes; existing diagnostic saved-state provenance still applies.


## Remaining binding consumers traced - 2026-10-07

Private WIP only, not packaged/installed/pushed. Bounded opt-in action-query
tracing now observes native held/scalar/edge consumers without changing guest
registers or input. Live trace identifies pointer actions 4/5/6 at 80ffbcac,
80ffbcdc,80ffbd0c and camera action3 at800f6368. Fixed missing vertical camera
mapping: RightUp/Down publishes action3 and enables action7. Native +/-0.75,
release-to-zero and three actual captures verify closer/back camera movement.
Windows MSVC Release build exit0, focused3/3 in2.06s, full50/50 in9.97s. No
compiler/linker warnings/errors in successful build log. No physical/audio/FPS claim.

Hero Management native captures verify View entry, A tutorial dismissal, LB
powers/details then assignment. Actual powers page still displays Wii +/- Allocate
and 1 Redistribute Points; live descriptors118/119/122 have no direct bindings.
Preserved native hero-powers state for repair. Short Back sequence remained in
hero browse. Longer saved-menu repeat cleared active menu but capture showed
stale menu background plus gameplay HUD with no world. A subsequent cold-start
repeat without loading a state reproduced the failure after normal View entry.
Formatter-off isolation also fails on View then Back without entering Powers.
Opening/closing Pause restores the world; Start -> Hero Details -> Back -> Back
returns correctly. These native captures were inspected. Direct View return
remains failed; code-level cause unresolved. See HERO-MENU-RETURN-ISOLATION.json.


Graphics isolation: native menu/stack flags clear identically in failed and good
returns. Four differing viewport drawing flags were temporarily cleared in the
private process and restored; captures still fail. No corrective source patch
was made. Investigation deferred at user request; see the open issue at the
top of docs/PERFORMANCE-GOAL.md. Earlier hooks/test setup remain suspects, not
established causes. Continue remaining bindings and prompts instead.
HERO-MENU-GRAPHICS-AUDIT.json retains evidence. Installation unchanged.

Wave-QTE registration still requires legacy managed_ports plus experimental flag.
Mask2 query802722dc selects analog/pointer tutorial branches but has many other
callers; no global flag was changed. Hacking, turret/Nullifier and other motion
families remain unresolved. Evidence: windows-20261007/REMAINING-BINDINGS-TRACE.json.
Goal not complete; installation/saves untouched. Compact source recovery refreshed.


## Fresh-data statue entry verified - 2026-10-07

The preceding unresolved entry was tested using Captain America. Stock hero data
assigns Might to Spider-Man, not Captain America. Resuming the pre-interaction
fresh-assets session, native D-pad Down selected Spider-Man. After diagnostic
placement at the same statue, X entered the real coop interaction. The in-game
capture shows the tilted X using the cold-loaded candidate HUD; no active-QTE
state import or resident texture refresh was used for this repeat. Native hero
eligibility was preserved; no entry workaround was added. Diagnostic placement
and the earlier prerequisite trigger enable mean this is not full natural-route
acceptance. Hero eligibility explains the prior difference; do not record it as
a proven lost-input defect.

Repeated-X validation passed again: wrong physical port counts zero, held X one,
twelve separate presses complete, target clears, coop entity is removed and
movement resumes (51.76 world units in the measured movement check). All four
simulated capability disconnect/reconnect transitions passed with empty Wii
bindings. Native entry/completion captures were visually inspected. Full existing
runtime/codegen suite passed 50/50 in 11.43s. No physical-input/audio/FPS claim.
Receipt: evidence/windows-20261007/FRESH-STATUE-GAMEPAD.json; private native proof
in workspace outputs/OpenMUA2-fresh-statue-entry.png and fresh-statue-completed.png.

Installed game remains unchanged. The remaining release work includes review of
the combined provider/fusion/prompt changes, remaining pointer/HUD prompts and
interaction paths, source checkpoint/verified routine backup/push and packaging.
The goal remains active; this correction does not establish full-game coverage.

## Rapid-tap statue prompt visual proof - 2026-10-07

Private opt-in candidate only; installed EXE/data/saves unchanged. The shared
motion HUD draw path now selects the supplied tilted X raised/pressed poses at
four cycles per game second while a validated direct button-QTE session is active.
Pauses stop its clock. This is presentation only; progress still requires twelve
separate X presses. Paired assets use --rapid-tap and the runtime requires
OPENMUA2_RAPID_TAP_GLYPHS=1. Do not enable either side alone.

Windows MSVC Release build passed with no compiler warning/error reported;
focused native tests passed 3/3 (2.41s), private rapid-art preservation test 1/1.
Native captured frames visibly show both tilted poses, the statue falling/removal,
and Hero Training: Might (COMPLETE). Process-local tests: wrong port = 0 presses,
held X = 1, twelve fresh presses complete and clear the actor interaction target.
This is a saved active-statue diagnostic fixture, with resident HUD pixels refreshed
byte-for-byte from the private candidate WAD. It is not cold-entry acceptance:
a separate fresh-loaded plaza/statue placement attempt did not enter the QTE and
remains unresolved. Preserve its state and logs. Other rapid-tap encounters and
physical-controller acceptance remain unverified; do not claim full-game coverage.

Proof receipt: workspace outputs/OpenMUA2-statue-QTE-proof.json. Native screenshots
were encoded at their game-clock timestamps (sampled capture, no FPS/audio claim).
Completion captured 68/71 requested frames; missing 10/14/16 remain timestamp gaps,
not fabricated frames. GIF shows the cue; MP4 shows completion. Private cold-start
profile captures also verify Connect gamepad to join in all three unused panels.
Source remains WIP, uncommitted/unpackaged; compact recovery saved, no release
checkpoint or push claimed. The full controls goal remains active.

## In-progress prompt validation - 2026-10-07

Experimental working tree remains uncommitted, unpackaged and unaccepted for release.
Native headless Vulkan captures show the fusion instruction panel for all four
verified initiators. An earlier missing-Iron-Man report was a visual inspection
error; the original and fresh-process repeat both show it. Do not change visibility
based on that rejected finding. The shield is also the selected Stars And Stripes
profile emblem; do not presume it should track the hero portrait.

The shared Xbox font builder trims alpha padding for every button and normalizes
button metrics to the face-button box, preserving UVs and unrelated characters.
The combined Xbox UI compiler includes both normalized font tables as well as the
textures. Private-data font regression tests pass 2/2 for normal and widescreen.
A private WAD changes exactly four font entries; installed data/saves are unchanged.
Matched cold-start repeat with empty Wii bindings passed title Start, main-menu
A/B, profile selection/confirmation and entered gameplay. Captures were inspected.
The first candidate attempt failed to advance main-menu A; cause remains unproven,
so preserve the failed evidence. Later matching repeat passed without a control fix.

Fresh-loaded fonts visibly enlarge LT in both the fusion banner and native fusion
help. The shared help now says to hold LT and choose A/B/X/Y, preserving resource,
revival text and its native formatting placeholder. Proof uses private diagnostic
plaza placement/resource setup, not a natural full-playthrough acceptance. A state
saved from that fresh session was used to repeat the help-text verification.
Remaining prompts include Connect Wii Remote to Join in disconnected player panels.
The old four-player fixture has a red pointer cursor; the fresh fusion capture does
not. Audit its lifecycle before suppressing a possibly unrelated HUD element.

Windows Release runner SHA256 c79392096ac97ce913ced21226aca0560534bbe150f4944b7d4a9d68c49231b5.
Build succeeded; focused3/3 passed in2.41s, with no compiler/linker warnings/errors
in the successful incremental log. No physical-controller, audio or FPS acceptance.
Installation unchanged. Evidence: task workspace outputs/OpenMUA2-prompt-investigation.json
and OpenMUA2-fusion-shared-LT-proof.png. Broader controls/prompt acceptance, source
review/checkpoint, routine backup, push and package remain outstanding.

## Paused experimental fusion work - 2026-10-07

Working tree only: not committed, pushed, packaged, or accepted for release.
PS2 fusion entry uses held action33; Wii queries an edge. The shared provider
candidate adds held FusionPower semantics, LT preparatory block sequencing,
and a common partner selector before either Wii pointer picker. Native roster,
pair eligibility, approach, resource consumption and animation lifecycle remain.

Headless raw process-local tests: all four LT owners entered the native chooser;
12 teammate choices selected the requested pair, four self choices rejected.
Native role ordering may swap the pair; the original initiator remained correct.
One ground-level human pair entered active fusion, spent resource1->0, and later
returned to mode0 with cleared pair pointers. The statue-elevated owner caused a
native unreachable-partner timeout without resource cost. No physical-controller,
rendered, audio or FPS acceptance. Windows Release build and focused3/3 passed;
runtime/codegen50/50 passed in18.58s. Diagnostic process stopped with exit0.

Earlier edge-pulse and held-level-only candidates failed native entry; retained
as failure evidence. The first partner matrix had stale held-port inputs and an
incorrect ordered-pair assertion; corrected neutral matrix passed16/16.

Paused at the user's requested higher-reasoning boundary. Next: remove obsolete
pointer-gate/conditional-HLE candidate and unused trace edits, review/simplify entry
sequencing, verify release/disconnect/simultaneous owners/empty resources and more
fusion pairs, then focused source checkpoint, verified Backup.cmd, push and package.
Preserve unrelated readiness/wave/StaticRecomp WIP. Existing installation unchanged.
Do not promote this working tree or claim the entire gamepad path is complete.
Evidence in the task workspace outputs/OpenMUA2-four-player-fusion-progress.json;
private native reads in work/gamepad-fusion-shared. Full routine backup deferred
with the unfinished checkpoint; compact WIP recovery archive saved in outputs.


## Shared gamepad capabilities and QTE completion - 2026-10-03

Found another shared Wii dependency: CInputManager vtable +124 reports device
connection and complete-controls capability independently of IsConnected/IsReady.
With empty Wii bindings and Extension=None, gameplay stopped updating while raw
input still polled. Merely changing the diagnostic profile to Nunchuk let the QTE
finish. The opt-in gamepad provider now replaces this shared physical-port query
with actual Xbox connection state for both outputs; disconnected pads stay false.
No extension identity, pointer, motion input or completion event is synthesized.

Moved QTE edge counting to input publication so animation callback cadence cannot
miss presses/releases. A recent native QTE clock is required, preventing progress
while gameplay is suspended. QTE and optional Xbox prompt hooks no longer require
managed Wii profile bindings when the shared provider owns input.

Final runtime test used empty Device/buttons and Extension=None on all four ports.
Wrong-port X counted zero; holding X counted once; twelve separate presses completed
the original stage/callback path, removed the interaction entity, cleared the actor
and restored movement. Native Start/B pause/resume passed: three X presses while
paused left progress at1, fresh X after resume made2. Each of four physical ports
independently reported connected/complete 1 -> 0 -> 1. Saved fixture and process-local
input only; no physical-controller or rendered acceptance claimed.

A prior-checkpoint five-minute main-menu idle test also passed A/B navigation and
reconnection, with timestamps past305 seconds. It is included as prior-build evidence,
not a new full regression of that flow. Final Windows Release runner build passed,
focused tests2/2 in1.94s; runtime/codegen suite50/50 in8.11s. Native run exited0.
No compiler warnings/errors in the targeted incremental build; existing unsupported
formatter diagnostics remain. Null audio; no FPS/audio claim.

Evidence: evidence/windows-20261003/GAMEPAD-CAPABILITIES-QTE.json. Installed game,
real saves and proprietary assets are unchanged. Provider remains opt-in. Next:
full per-player fusion (current selector excludes human partners), other QTE and
UI/targeting consumers, then whole-flow and physical/rendered acceptance. Preserve
unrelated readiness/wave/StaticRecomp experiments; do not resume prompt-only patches.


## Four joined gamepads and chooser ownership - 2026-10-03

The opt-in shared gamepad provider now maps horizontal D-pad/left-stick input to
HUDPreviousItem/HUDNextItem (63/64), consumed by the existing profile/hero join
chooser. Start opens joining; A confirms the profile and then an available hero.
The engine retains profile creation, occupied/KO checks and actor assignment.

A mapping-only candidate failed: unjoined chooser input also emitted gameplay
hero selection and changed player 0's hero. The corrected provider uses the
engine's joined-player flags and logical-to-physical mapping to restrict unjoined
pads to chooser/menu actions. Gameplay actions become available after native join.

A real headless plaza fixture now joined all four heroes through process-local
Start/A/direction input: actor slots 0..3 each have AI=false and owner ports 0..3.
Simultaneous movement actions were [0.25,0.375,0.5,0.625]; releasing port 2 changed
only its value to zero; all released values became zero. Selecting a hero already
owned by another player left all four owners unchanged. This is native state
verification, not physical-controller or rendered gameplay acceptance.

Cold boot reached the title and readiness-based Start/main/A/Story/B/main passed.
Earlier fixed-time assertions failed during the opening movie and menu transition;
those failures remain in diagnostics. No load-state was used for the cold check.
Windows Release runner build passed; focused tests 2/2 (1.97s), runtime suite plus
codegen_compile 50/50 (10.70s). Existing formatter diagnostics remain; Null audio,
no FPS/audio claims. Evidence: evidence/windows-20261003/FOUR-PLAYER-JOIN.json.

The installed game and real saves are unchanged. Full per-player fusion/QTE,
remaining UI/targeting consumers, cold campaign, idle/reconnect and visual/physical
acceptance still remain; provider is opt-in. Preserve unrelated local experiments.


## Shared gamepad status and tutorial acceptance - 2026-10-03

The opt-in OPENMUA2_GAMEPAD_PROVIDER path now replaces the two shared engine
connection/readiness methods with per-gamepad state, preserving logical-to-physical
port remapping and engine-owned input slots. Actual runtime logs exercised both
methods. Each of four ports independently reported ready -> disconnected -> ready.
This is process-local input evidence, not four joined actors or physical-pad acceptance.

PS2-style shared tutorial acceptance is implemented: the existing action-9 edge
schedules native close after 0.2 game-clock seconds instead of entering pointer
readiness. Original input-history cleanup and delayed hide/unpause/callback remain.
The live fusion tutorial fixture rejected port 1 A for owner 0, then owner 0 A closed
it with phase 0 retained. No ready-icon or completion flags were forced by the test.
All five tutorial kinds and four owners are covered by helper tests; only kind 1
was exercised in the runtime. This does not establish fusion combat completion.

Windows Release runner build passed; focused tests 2/2 (2.12s). The 49 runtime
regressions plus codegen_compile passed 50/50 (10.17s) in the documented MSVC
environment. An initial unfiltered CTest attempt exposed three absent dependency
benchmark/fuzzer executables and a missing string.h compiler environment; the latter
passed after msvc-env.cmd. These were not gamepad source failures. Native runs exited
0; existing unsupported-formatter diagnostics remain. Null audio, no FPS claims.

HLE now supports guarded function-entry replacement with captured stwu interpreter
fallback; the successful replacement path ran, fallback is not separately exercised.
The installed C:\Games\MUA2 build and real saves remain untouched. Provider stays
opt-in while full consumer/prompt, connection-warning, joining, four-actor gameplay,
QTE/targeting and cold-flow validation continue. See GAMEPAD-STATUS-TUTORIAL.json.
Prior unrelated readiness/wave/StaticRecomp experiments remain local WIP.


## Shared four-port gamepad provider - 2026-10-03

Development flag OPENMUA2_GAMEPAD_PROVIDER=1 now publishes raw Xbox actions at
CInputManager's shared update boundary, independently of KPad availability gates.
All four ports own connection, neutral-on-connect, held/released input and action
state. Existing game-disabled/ownership checks and action queue bookkeeping are
preserved. Default remains the supported compatibility path; no installation changed.

Fixed a release-build HLE dispatcher defect: its empty observer/branch marker
functions could share an address after linker folding. Dispatch now uses explicit
hook identity. The identical saved-state run then executed all four update callbacks.
The immediate-query replacement is implemented but has not been exercised by these
runs; do not claim full caller coverage from update counters alone.

Actual process-local tests with all 124 legacy binding counts disabled on all four
input objects passed simultaneous distinct axes, per-port release/disconnect,
held reconnect suppression, fresh input after neutral and held-state clearing on
load. Synthetic pads were not assigned to Wii profiles. Main-menu A/B navigation
passed; port 1 A did not operate port 0's menu. Native Story-menu capture was inspected.
This does not prove four joined actors, physical pads, connection warnings, combat,
fusion, QTEs or complete removal of pointer interactions. Full goal remains active.

Windows Release runner and two relevant test binaries rebuilt successfully;
focused tests 2/2 (2.18s), full existing regression suite 49/49 (7.28s). Initial full
Build.cmd completed before the manager changes (49/49, 14.41s); GUI/package was
not rebuilt or staged afterwards. No compiler/linker errors. Existing configure
warnings remain. A separate intrusive JIT-profiling run crashed before startup
(exit 3221225477); no profile was produced and its private config was restored.

PS2 executable reference is extracted only under ignored .local/ps2-reference.
Its shared tutorial accepts action 9 by scheduling normal close after 0.2 seconds;
Wii instead enters phase 1 pointer readiness. This is code evidence, not an asset
inference, and that semantic replacement is still pending. PS2 config initialization
loops through four slots using one method; retail multiplayer support is not inferred.

Next: finish shared connection/status and four-player ownership, then implement
PS2-style shared tutorial acceptance and audit remaining pointer/motion consumers.
Do not resume isolated prompt substitutions. Preserve real saves and single-core
execution. Prior readiness/wave/StaticRecomp experiments remain local and excluded
from this focused checkpoint. Evidence: SHARED-GAMEPAD-PROVIDER.json and
PS2-GAMEPAD-REFERENCE.json under evidence/windows-20261003.

## Full gamepad path audit â€” 2026-10-03

User requested full-path verification before more isolated Wii-interaction fixes.
See docs/GAMEPAD-PATH-AUDIT.md. No complete dormant alternate backend is verified.
The native factory/decoder is KPad-based. The direct model lacks several menu
actions, and an evaluator early exit bypasses its current hook at 0x810f8544.
That coverage hole is statically proven; the exact reason the modal run did not
activate deferred readiness hooks remains unverified.

New shared-readiness/HLE candidate remains uncommitted development work. Windows
build exit 0; 49/49 tests passed in 10.41s. Actual fusion-readiness acceptance FAILED:
plain A left ready slots zero and the renderer still displayed pointer instructions.
The first candidate stalled during asynchronous boot; revised run exited 0.
Do not stage either candidate or treat install logs/menu checks as full acceptance.
Installed EXE/assets/saves unchanged; no physical-controller or FPS/audio claims.
Audit source/docs checkpoint only; preserve the pending implementation and prior
wave-QTE/StaticRecomp changes. Goal remains active.

## Direct gamepad action layer development - 2026-10-03

An opt-in OPENMUA2_DIRECT_GAMEPAD=1 backend reads raw XInput/test-device samples
and writes logical action bits/values. It bypasses Wii binding outputs and passes
explicit hero/fusion slots without marker actions. It remains OFF by default.
Device assignment still uses the managed v5 profile and native polling lifecycle;
this is not complete Wiimote removal and is not installed in C:\Games\MUA2.

Final Windows Build.cmd --cpu jit --jobs 2: exit 0, 49/49 tests passed in 7.29s.
No compiler/linker errors; existing CMake warnings remain. Cold start/title/main
and A/B Story navigation worked. Main menu is horizontal: Down doing nothing was
misdiagnosed as failure until renderer inspection. Native trace established the
second directional action quartet; final Left/Right and Options navigation passed.
Internal widget names can misidentify visible entries: option_Profile_text was
Credits in the actual capture. Verify images, not field names alone.

With all 124 legacy descriptor binding counts zeroed in a private process,
Options Down/Up/Back and A/B navigation still worked. Wrong-port Down did not move
focus; all current action/scalar bytes cleared on release. Native captures were
inspected. Existing controls-menu Wii diagram/overlapping text remain deferred.
No physical-controller, gameplay, FPS/audio, or complete UI acceptance claimed.
All three diagnostic runs exited 0; single CPU2, Vulkan3x, Null audio, build-tail
CPU overlap. No installed EXE, WAD or real saves were changed.

Next: replace shared readiness/pointer and motion interaction families using the
direct gamepad state. Validate axes, gameplay, fusion, reconnect and idle before
staging. Preexisting wave-QTE and StaticRecomp changes remain local and excluded
from this focused commit; the tested worktree included them (wave default off).
Evidence: evidence/windows-20261003/DIRECT-GAMEPAD-DEVELOPMENT.json.
Full goal remains active.

## Gamepad architecture correction - 2026-10-03

The pointer prompt exposed a structural issue: managed Xbox v5 still feeds Wii
buttons/IR/tilt/Nunchuk shake. A complete dormant gamepad backend is NOT verified.
Code audit traced KPad device enumeration (0x80293cf4), sample decoder
(0x802930f0), and the 124-action CInput layer (0x810f7e80). The inspected device
factory accepts only types 0/1; no alternate gamepad branch was found there.
Reusable actions/button challenges do not establish a complete platform backend.

Follow docs/GAMEPAD-REPLACEMENT-PLAN.md: direct per-port Xbox input into the action
layer, then replace pointer/motion consumers by interaction family. Do not resume
screen-specific prompt patches as the architecture. Statue = repeated X;
fusion = LT plus face-button partner selection, with actual context validation.

Unvalidated readiness/pointer source is parked in workspace work/parked-tutorial-ready.
Preexisting wave-QTE changes were restored to the worktree; StaticRecomp work is
preserved. Local built runner is a stale FAILED readiness candidate: rebuild before
packaging. Installed EXE/GameData/saves unchanged. No new build/test/visual success
claimed. Evidence: evidence/windows-20261003/INPUT-ARCHITECTURE-AUDIT.json.
Full goal remains active.

## Fusion banner candidate rejected â€” 2026-10-03

The text-based top-banner candidate was WRONG: it replaced the statue co-op
QTE glyph when fusion tutorial text was forced into the HUD. The resulting
screenshot is not valid fusion acceptance. Candidate source has been withdrawn
from the worktree and retained only under workspace work/rejected-fusion-text-banner.
No candidate EXE or WAD was installed; supported C:\Games\MUA2 remains unchanged,
including all 55 save/profile files. Preexisting experimental work is preserved.

A fresh diagnostic run using installed assets completed the seeded statue with
12 X presses, then used LT to enter the actual fusion first-use flow. A advances
to a pointer/profile-icon readiness gate; A and LT+A do not dismiss that gate.
No forced tutorial text or fusion-state writes were used in this follow-up.
Native captures were inspected. Actor placement/mission-trigger fixtures mean
this is not normal-route acceptance. No FPS, audio or physical-controller claim.

Next: trace the real first-use readiness handler and partner-selection state.
Statue QTE remains repeated X; fusion uses its own verified context. Never use
manually injected tutorial text as proof of correct interaction selection.
Reproduction state: workspace work/fusion-real-interaction/fusion-ready-pointer-block.sav.
The local runtime binary is the rejected diagnostic build (feature disabled for
the final run); rebuild restored source before any packaging. 48/48 passing
tests were insufficient to validate the rejected context rule. Full goal remains
active. Evidence: evidence/windows-20261003/FUSION-CONTEXT-REJECTED.json.


## Installed alternate co-op QTE sequence - 2026-10-03

The direct repeated-X interaction now also accepts the exact electro_sequence
name. Five original interact_coopsequence assets define it and generic_sequence
with the same ch_coop_sequence handler and state transitions. The two Latveria
boss-pit objects use the alternate name. All owner/type/handle guards remain.
The regression suite checks both names and rejects near matches.

Windows Build.cmd --cpu jit --jobs 2 exited 0; 48/48 tests passed in 10.06s.
Package checksum verification passed, exit 0. A headless statue shared-handler
fixture substituted only the sequence name: wrong buttons/port made no progress,
holding X counted once, 12 fresh presses completed and cleared the target.
The string was restored and the game exited 0. This is NOT an actual Electro
boss encounter or multiuser acceptance. Chemical tank defaults remain unresolved.
No screenshots/host input or physical/visible/audio/FPS claims. Single CPU2,
Vulkan3x, normal clocks, Null diagnostic audio; build-tail overlap.

Installed EXE SHA256:
21379ce9be5c51e701f7a67425b54f8e077654903d6d9f2abf87d311d03ca911
Runner SHA256:
31ab4643395a4abdac0778185951521b19ed20dba9bfaa5962846f06bc20b66b
GameData and all 55 save/profile files unchanged. Previous EXE retained as rollback.
Xbox button glyphs/action labels remain installed; controller pictures excluded.
Full controls/UI goal remains active. Evidence:
evidence/windows-20261003/ELECTRO-SEQUENCE-QTE.json.


## Installed contextual Xbox menu prompts - 2026-10-03

Controller pictures/diagrams are excluded from further work at the user's
request. Continue focusing on Xbox button glyphs and button/action labels.

MENU_OK now shows Xbox Start only on the same verified title/profile menu types
where the managed Start adapter emits action 105. Other contexts remain
unresolved; do not relabel them A or Start without fixing/validating their action.
MENU_OTHER shares action 101/Details and shows LB; MENU_SUBTRACT (106) and
AUTOSPEND (109) use Y. The latter aliases have regression coverage only.

Windows Build.cmd --cpu jit --jobs 2 exited 0, 48/48 tests passed in 9.25 seconds.
Existing CMake warnings remain; no compiler/linker errors. The package checksum
verification exited 0 and the launcher accepts the existing matched v3 UI assets.
The headless run's first 240-second driver wait timed out while the game stayed
live. Continuing that same process found an attract movie; Start did not skip it,
but A did. All 4,080 title MENU_OK calls resolved 8->1 (Xbox Start). Start then
entered main, A/B opened/closed Story, and D-pad/A/B opened/closed Options.
The first continuation assertion incorrectly assumed title and is retained as a
failed observation. Stop was processed and the game exited; its exact exit code
was not retained after the original driver lost its handle. Navigation assertions
passed. No screenshots/host input or visible/physical acceptance; no FPS/audio
claim. CPU2/mask4, Vulkan3x, normal clocks, Null audio. Idle/reconnect not repeated
for this glyph-only change. Other QTE variants and menu actions remain open.

Installed EXE SHA256:
e58c99df57926933248da4a37201156066a625249fc5397ba02e70fca99137fb
Packaged runner SHA256:
f22b5769645759b7a0966ef570212a8fd7c27aa9b506266b5ac3391c4ad270a6
Unchanged WAD SHA256:
e90498f4aa3d9a91598ea7b135150dcf14ed34a9111ad4d9c2a58e802be81092
All 55 save/profile files unchanged; previous Options release preserved as
rollback. Automatic approval policy blocked duplicate-build cleanup. Do not
retry the exact targets listed in evidence/windows-20261003/XBOX-MENU-PROMPTS.json.


## Installed Xbox Options descriptions - 2026-10-03

The installed root executable and GameData now include ten corrected Options
control descriptions: A light attack, B heavy attack, RT+face powers, LT+face
fusion, Y jump, LB block, X grab/use, Start pause, D-pad hero selection and right
stick camera. The compiler rewrites only the ten selected descriptions in each
of three guarded string tables, reusing their allocations without changing size.
The launcher v3 manifest now checks ten archive members, including these tables.

The Windows C# launcher compiled successfully with no reported warnings/errors.
Four Options-table tests, five tutorial tests, nine launcher gate checks and nine
profile checks passed. The unchanged C++ runner was not rebuilt; its prior 48/48
result remains scoped to the preceding build. Actual package checksums passed;
the new launcher accepts its matching assets and rejects the previous UI set.

A cold headless run reached the native options_rev menu and its real descriptions
requested all ten expected Xbox glyph tokens, without memory text substitutions.
B returned to the main menu and the process exited 0. This proves native loading
and formatter behavior, not final appearance or physical-controller acceptance.
Single logical CPU 2, Vulkan 3x, normal clocks, Null audio; no FPS/audio claim.
The Wii controller diagram/static labels, gesture help, other QTE variants and
remaining menu actions are still open. MENU_OK still resolves to native icon 8
and needs a semantic audit. The full controls/UI goal remains active.

Installed EXE SHA256:
031934aa8187c7c4f87597b03f5d3e36005e8244961ce1aa25e357f6e58d02fd
Installed WAD SHA256:
e90498f4aa3d9a91598ea7b135150dcf14ed34a9111ad4d9c2a58e802be81092
All 55 real save/profile files are unchanged. The previous supported UI pair is
preserved as rollback; original extracted assets remain unchanged. Root contents
remain OpenMUA2.exe, GameData and saves. Evidence:
evidence/windows-20261003/XBOX-OPTIONS-HELP.json.

Duplicate candidate cleanup was rejected by automatic approval policy; no files
were deleted. Do not retry the targets recorded in the evidence receipt.


## Installed common and auxiliary Xbox UI - 2026-10-03

The root C:\Games\MUA2\OpenMUA2.exe now packages the matched Xbox UI resolver.
Both normal/widescreen English menu fonts contain Start, A/B/X/Y, LB, D-pad,
LT/RT, View and stick artwork. Twelve tutorial instructions use Xbox controls
and explicit glyph tokens. Native DPAD keeps ID 6; contextual X now uses ID 24,
fixing a collision in the earlier uninstalled candidate. The launcher embeds a
versioned seven-member hash manifest and enables glyph resolution only after
validating the matching GameData assets. No extra installation root files.

Windows Build.cmd --cpu jit --jobs 2 passed 48/48 tests in 11.45 seconds, exit 0.
Five tutorial tests, nine launcher UI-gate checks and nine profile checks passed.
Existing vendored CMake warnings remain; no compiler/linker errors. The packaged
runner cold-booted with candidate assets and native menu snapshots verified
Start -> main, D-pad focus movement, A -> Story, B -> main, A -> Story again.
A seeded process-local prompt fixture exercised LT/RT, sticks, View, D-pad and X
through the native formatter, then restored its strings and entered the QTE.
Earlier literal probes and too-short prompt windows failed their assertions;
the final three-second windows passed. No screenshots or host input were used.

Installed EXE SHA256:
36cfa89bf54d9b6f2666a8b3ff3497d9355834cbdfb63d75c04a745bbe1c256b
Installed runner SHA256:
4c0916c05d461cdbb9befbd504867d3ce388ec308f78a5f4e6d0da4480017f74
Installed WAD SHA256:
fc2710eb8df0a028b36e127ff93a049c302a21bb5f75aeea2a46201af923561b
All 55 save/profile files are unchanged; original extracted data is preserved.
The installation root remains OpenMUA2.exe, GameData and saves.

This is a PARTIAL Xbox UI conversion, not full visible/physical acceptance.
Other Wii-specific instructions/menu actions and QTE variants remain open.
Audio/FPS were not assessed (headless diagnostics used Null audio). Idle and
reconnection were not repeated for this UI-only change. Keep the full goal
active. Evidence: evidence/windows-20261003/XBOX-UI-INSTALLED.json.


## Xbox tutorial and combined UI candidate - 2026-10-03

The private --xbox-ui asset compiler now combines the seven QTE X cells, both
common Xbox menu fonts and twelve updated Rev tutorial instructions. Grab uses
contextual X; powers describe RT with A/X/B/Y; hero selection uses the D-pad;
fusion describes LT with a partner face button; horizontal camera control uses
the right stick. Trigger and face names in the new power/fusion text are literal
labels pending the remaining auxiliary glyph work.

Five private-data Python regressions pass. All three standalone tip tables and
both packaged copies retain their original sizes. The compiler separates a
shared Rev/PSP fusion string into reclaimed string padding without changing the
PSP entry. Initial shared-string and allocation-overflow checks failed safely;
the corrected conversion passes. Independent full archive comparison confirms
exactly seven changed members and 1,041 byte-identical members; archive CRC
validation passes. Original archive and installed EXE hashes remain unchanged.

This is NOT installed or native/rendered UI acceptance. No C++ changed, so the
previous 48/48 Windows build remains the latest runtime build. Unverified camera
reset, quick-assign and throw instructions remain open, as do the full UI/QTE and
physical-controller acceptance requirements. Evidence:
evidence/windows-20261003/XBOX-TUTORIAL-CANDIDATE.json.


## Native contextual Xbox X prompt correction - 2026-10-03

A seeded statue approach exposed the actual uppercase ACTION token (action 21).
The first opt-in prompt mapping missed that alias and still returned A. The
mapping and regression now include ACTION. A repeated headless native run exited 0
and traced ACTION:2->6 (Xbox X), MenuAccept:2->2 and MENU_BACK:3->3; X entered the
QTE. The mission trigger was enabled only in diagnostic memory. An earlier
attempt changing unrelated prompt literals did not exercise Action and remains
a failed fixture, not acceptance.

The corrected Windows build passed 48/48 tests in 17.46 seconds, exit 0. Existing
vendored CMake warnings remain; no compiler/linker errors. This remains opt-in
and uninstalled: fonts are separately inspected private candidates. No in-game
visual, physical-controller, audio or FPS acceptance. Full UI/QTE scope remains
open. Evidence: evidence/windows-20261003/XBOX-ACTION-PROMPT.json.

Removed the redundant private QTE WAD only after matching its SHA256 to the
installed copy; reclaimed 281,821,811 bytes. Original data and installed WAD/saves
are preserved. Previously rejected cleanup targets were not retried.


## Xbox menu prompt development checkpoint - 2026-10-03

The new opt-in OPENMUA2_XBOX_GLYPHS=1 observer resolves common Xbox prompts by
semantic action instead of the original Wii descriptor. Contextual Grab/Action
uses X; menu accept/back uses A/B; block uses LB, jump uses Y and pause uses Start.
It is restricted to managed controller ports, exact code hashes and the verified
controller type. Unknown tokens retain native behavior. This is not a complete
power/fusion/gesture UI conversion and is not enabled in the installed game.

The font compiler tools/build_xbox_menu_fonts.py produces private English normal
and widescreen menu font candidates from the supplied Xbox artwork. It guards
both texture and coordinate-table hashes, preserves all other bytes and refuses
existing output directories. Six glyphs per font were decoded and individually
inspected. The output is not installed and must accompany the runtime mapping.

Windows Build.cmd --cpu jit --jobs 2 passed 48/48 tests in 9.68 seconds. An initial
compile failed on a diagnostic state.lr reference; it was removed before the
successful build. Existing vendored CMake warnings remain. A headless native
repeat exited 0 and observed MenuAccept -> A and MENU_BACK -> B. Contextual X is
covered by the focused regression, but has not yet been observed as a displayed
prompt in the native flow. One prior automation command-file open failure was
retained; it occurred before parsing or input and the fresh full repeat passed.
No screenshots, host input, physical-controller, audio or FPS acceptance.

The installed EXE remains a3740b32dd3be5f8f1cac94c4fbb2fc14395dc6f6ae6dbea477cab5978fb1d3a.
Actual saves and installed assets were not touched. Existing unrelated static
recompiler edits remain preserved and included in this local build. Evidence:
evidence/windows-20261003/XBOX-PROMPT-DEVELOPMENT.json. Removed 272,241 bytes of
verified duplicate candidate fonts; previously rejected deletions were not retried.
Continue the full controls/QTE/UI goal; this checkpoint does not satisfy it.


## Installed repeated-X statue safeguard - 2026-10-03

C:\Games\MUA2\OpenMUA2.exe now includes direct repeated-X QTE progress for the
managed Xbox profile. Twelve separate presses complete the tested statue
safeguard; holding counts once. The legacy A/B-to-gesture adapter was removed.
The original completion callback still performs mission cleanup. Seven HUD
motion cells now use the supplied Xbox X artwork, and the safeguard instruction
says to repeatedly press X. Other Wii prompts remain to be converted.

The final Windows build (Build.cmd --cpu jit --jobs 2) exited 0; 47/47 tests
passed in 10.66 seconds. Existing vendored CMake warnings remain; no compiler or
linker errors. The packaged and installed EXE both verified every embedded file
and runner help, exit 0, CPU affinity mask 4. Exact binary hashes and limits are
in evidence/windows-20261003/DIRECT-QTE-INSTALLED.json.

Installed EXE SHA-256:
a3740b32dd3be5f8f1cac94c4fbb2fc14395dc6f6ae6dbea477cab5978fb1d3a
Installed runner SHA-256:
2c825b97cb5b863247bfffb13ec2fa5bb5aeab7597aad8d6b84f764debe7889b
Installed private assets archive SHA-256:
f4307ebb9ecc7f9d995c88bf4fc87b1ddcbfe5509866d617fb2b4f05bc28ec45
All 55 real save/profile file hashes are unchanged. The installation root still
contains only OpenMUA2.exe, GameData and saves. Original game data is preserved in
.local/game; no proprietary archive, supplied image or state is committed.

Headless process-local testing established twelve press edges, one completion,
removal of the original callback's blockers/spawners, cleared interaction target,
and subsequent player movement. Holding X counted once; A/B and the wrong port
did not advance progress. A menu-closing X press did not also advance the QTE.
Three consecutive state reloads reset progress and accepted the next fresh press.
The final run used the managed profile without the experimental QTE flag.

This used a seeded fixture: actor placement and the mission-gated statue trigger
were prepared solely in the diagnostic process. The trigger was disabled before
fusion tutorial completion, explaining the earlier failure to enter the QTE.
It is not normal-route, physical-controller, visual, audio or FPS acceptance.
No screenshots or host input. Artificial zero-health cancellation remains
unverified; multiplayer and other motion interactions remain outstanding.

An over-strict magnitude check was caught by the live repeat and removed: the
digital action bit owns the press edge. A separate diagnostic load-state race
was fixed: acknowledgment now follows CPU-thread restoration, preventing a
subsequent scheduled input from being erased. The repeat passed. One transient
read-only command-file open failure was also retained in private evidence.

Inventory found 29 cooperative map definitions: 25 generic_sequence, two
electro_sequence and two unspecified. Only the statue was exercised. Do not
claim every QTE or all Xbox UI converted. Continue the full controls goal.

Cleanup reclaimed 281,821,751 bytes from the superseded glyph-only archive.
Automatic approval review rejected deletion of the new temporary packaging
folders and superseded fixture (blocked by policy); these were not retried.

## Direct repeated-X runtime checkpoint - 2026-10-03

Repeated X presses directly own QTE progress and completion. Holding X does not
repeat; there is no separate B step. The development runtime now connects the
ButtonQte state to cooperative animation and stage completion, behind
OPENMUA2_DIRECT_QTE=1. Exact code hashes, live handles and ownership guard the
observers. The previous gesture adapter remains only in the default supported
path until this replacement is validated. Twelve presses is provisional tuning.

Windows Build.cmd --cpu jit --jobs 2 completed successfully: 47/47 tests passed
in 16.64 seconds. CMake emitted vendored policy, minimum-version, object-path and
unavailable Wayland warnings; no compiler/linker errors were reported. This build
includes the unrelated preserved StaticRecomp worktree changes. Exact binary
hashes and limits: evidence/windows-20261003/DIRECT-QTE-DEVELOPMENT.json.

The headless development runner installed its observers, but the retained plaza
fixture did not enter the cooperative QTE. The fixture includes diagnostic actor
placement and is not evidence of normal gameplay progression. Native completion,
cleanup, cancellation, load/reconnect and all motion-QTE variants remain unproven.
No screenshots, desktop input, physical-controller acceptance, audio or FPS claims.

The supplied Xbox X artwork can now be compiled into seven motion cells in the
HUD atlas with tools/build_qte_glyph_override.py. A private candidate WAD passed
archive round-trip/preservation checks (1047 unrelated members unchanged), and its
decoded static atlas was inspected. Only the main HUD motion cells are covered;
other Wii button prompts and gesture menus remain outstanding. Inputs are intact.

Installed C:\Games\MUA2\OpenMUA2.exe remains unchanged (SHA-256
3fa4ead4e7d7de3963077ea08cd6c94284450fa4f9592ff0bc43ffec8126c375).
The new local runner is a development build, not an accepted replacement. Do not
package it as ready based on unit tests. Keep the goal active and verify the
actual cooperative completion callback before enabling or installing this path.

## Direct mash-X QTE work in progress - 2026-10-02

The user reports smooth gameplay, high FPS, clear audio and working controls,
with motion QTEs still unusable. This is qualitative physical playtest feedback,
not a measured sustained combat benchmark or full reconnect acceptance.

Required behavior: repeated X presses directly advance and complete the QTE.
Holding X must not count repeatedly. No shake/lift translation and no separate
B step. The experimental X-to-shake change was reverted and never installed.
The current runtime still contains the previous A/B gesture adapter; replacing
that adapter is outstanding. Do not reintroduce it as the mash-X implementation.

A standalone ButtonQte gameplay state now counts press edges, guards ownership,
preserves progress across input suspension, resets for a new interaction, and
emits completion once. Windows MSVC focused build and CTest passed: 1/1,
0.04 seconds test time / 0.32 seconds total. It is NOT connected to the live game.
The test quota of twelve is a fixture value, not an installed gameplay setting.
No new game-side completion, animation, audio or FPS validation is claimed.

Next: connect direct progress to cooperative interaction animation and its
completion/cleanup callbacks, remove gesture routing for this interaction,
then validate release/hold, cancellation, load/reconnect and mission progression.
Local analysis identified the cooperative stage/animation/completion functions;
no extracted instructions or generated game code are included in this checkpoint.

Xbox glyph work is also incomplete. The read-only inspect_button_glyphs.py tool
parsed 15 local Wii/360/Xbox font tables and inventoried the ten supplied Xbox
PNG atlases. Coordinates and images remain private. No textures were replaced;
atlas placement and texture decoding still need verification.

Installed C:\Games\MUA2\OpenMUA2.exe remains unchanged, SHA-256
3fa4ead4e7d7de3963077ea08cd6c94284450fa4f9592ff0bc43ffec8126c375.
The rejected candidate EXE was deleted (10,596,864 bytes reclaimed), and the
packaging workspace runner was restored from the installed payload. The main
local build runner still contains the rejected experiment until rebuilt; do not
package it. No new screenshots, desktop input, save changes or live runs occurred
at this checkpoint. The controls goal remains active.

## Combat visual follow-up - 2026-10-01

The unchanged installed payload was tested further in the retained plaza state.
A emits native light-attack action 9; selecting Iron Man near a living Doombot
produced visible strikes. Teammate attacks prevent attributing damage amounts.
LB visibly holds a shield guard and returns to normal stance after release.
X emits grab/context-use actions 11/21, but two nearby-enemy attempts did not
conclusively establish a completed grab. That acceptance remains open.

All 31 captures across three follow-up runs were inspected individually; all six
sampled neutral windows cleared, and all three processes exited 0. These are
contained headless tests, not physical XInput acceptance. Physical reconnect,
full gameplay mappings, powers/fusion/revival, audio and FPS remain unverified.
No source or installed binary changed and no rebuild was necessary. Evidence:
evidence/windows-20261001/VISUAL-COMBAT-FOLLOWUP.json. Diagnostic profile reused;
no new save clone. Actual game saves remain separate. C/D free space at this
checkpoint exceeded 225/109 GiB; prior cleanup rejection remains unresolved.

## Installed cold-start and plaza visual verification - 2026-10-01

The final installed payload from source 857985af was cold-booted without a
savestate, idled in the main menu for 300 guest seconds, and progressed through
navigation/back, difficulty, complete profile creation, join/Ready and Start Game
to the opening Doomstadt cinematic. All 26 native captures were inspected in
order. All 476 native connection samples remained connected, and sampled action
bits cleared in all 26 neutral windows. No disconnect overlay appeared.

A separate retained plaza-state test inspected all 17 captures in order. D-pad
Right/Down/Left/Up selected Iron Man/Spider-Man/Wolverine/Captain America; Y showed
jump/landing, B a shield attack, and left stick movement. A's attack was not
identifiable in these selected frames and remains visually unconfirmed. All
eight sampled neutral windows cleared. Both installed-payload runs exited 0.

These are headless, process-local tests on logical CPU2, Vulkan 3x, normal clocks,
and Null audio. Physical XInput acceptance/reconnect, the full gameplay mapping,
audio and FPS remain unresolved. Latest physical poll returned 1167 for all four
slots. Actual saves were not opened; the second run reused the diagnostic profile.
No new build was needed; the installed package and 46/46 regression result remain
unchanged. Evidence: evidence/windows-20261001/INSTALLED-COLD-IDLE-VISUAL.json.

## Native screenshot controls checkpoint - 2026-10-01

The user's ImGui GetIO assertion was reproduced by the headless Vulkan diagnostic:
shader compilation progress attempted to access an absent OnScreenUI context.
The headless path now waits for compilation without drawing that progress UI.
Subsequent native screenshot runs exited 0 without that assertion.

Visual testing found two further Start-button errors: the profile-name keyboard
canceled the typed name, and the joined-player Ready screen backed out of Ready.
Start now emits continue/accept for those exact active menu types, preserving
pause/back elsewhere. Pointer bounds, manager type and game-code guards remain.
Regression cases cover both new contexts and the existing pause/back path.

Build.cmd --cpu jit --jobs 2 completed successfully: 46/46 Windows CTests,
7.81 seconds. No compiler/linker warnings or errors were emitted. Existing
CMake warnings remain: deprecated policies/minimum versions, missing Wayland,
object-path limits and LZ4 policy configuration. Output includes moderngekko-run.exe,
ModernGekko.exe, moderngekko-port.exe and diagnostic/test executables.

Native 2501x1410 captures were inspected individually for cold title/main-menu
navigation, A/B progression, profile naming/icon/autospend, player join/Ready,
Start Game and the opening Doomstadt cinematic. Fix-specific reruns loaded the
captured keyboard/Ready states; this is not a single uninterrupted cold-start
retest of the final binary. A final retained-plaza run visually verified
Start pause, Start resume, Start pause and B resume, with the scene and HUD
rendering after both returns. The initial failed screenshot run and both
pre-fix Start failures remain recorded as failures.

Tests used process-local input, one logical CPU2, normal clocks, Vulkan 3x and
Null audio. No desktop capture/control or host input was used. Selected captures
verify menu states and scene content, not every animation frame, audio, physical
XInput hardware or sustained combat FPS. The controls goal remains active.

Installed C:\Games\MUA2\OpenMUA2.exe SHA256:
3fa4ead4e7d7de3963077ea08cd6c94284450fa4f9592ff0bc43ffec8126c375
(10,596,864 bytes). Embedded payload verification passed before and after staging;
all 55 existing save files remained byte-identical. Root is EXE, GameData,
saves. No visible user game was launched. Prior installed EXE retained as rollback.
Source evidence: evidence/windows-20261001/VISUAL-MENU-CONTROLS.json. Native captures
and private game state remain outside Git. Disk check: repo 31.10 GiB, D free
109.06 GiB. Earlier automatic review blocked disposable-file deletion; no bypass.

## XInput failed-poll release checkpoint - 2026-10-01

XInput reads previously ignored their return code, allowing a disconnected pad's
last buttons, triggers or sticks to remain active. Failed reads now clear the
complete input packet; a later successful read on that slot restores input.
Battery queries now compare the Win32 result to ERROR_SUCCESS instead of using
HRESULT success semantics. This does not solve or claim verification of slot
changes or late wireless discovery.

Windows Build.cmd --cpu jit --jobs 1 exited 0: 46/46 tests in 12.38 seconds.
The new process-local polling regression covers held controls, an untouched output
buffer on disconnect, repeated failure, reconnection, partial failed packets and
release. The initial jobs-2 build stopped during linking with exit 1 and no linker
error in its log; it is not counted as a pass. No compiler/linker diagnostics in
the successful build; existing CMake warnings remain.

Headless single-CPU2 tests of the resulting runner passed cold title/main-menu
navigation through Play/Difficulty/profile selection and a retained plaza-state
Start/Start/Start/B pause sequence. All 15 sampled neutral windows cleared native
action bits. Menu native link stayed connected in all 132 samples. These tests
used Null graphics/audio and process-local input; they overlapped build linking
and establish no FPS, visible behavior, audible quality or physical acceptance.
The earlier native idle-timeout fix and its >600-second evidence remain unchanged.
Windows reported no connected XInput pad during this checkpoint.

Installed C:\Games\MUA2\OpenMUA2.exe: SHA256
3d04700ff311c71d9b04aed6716c07171f0dabe7f443a258acea6aef6df30307,
10,597,376 bytes. Embedded payload verification passed; all 55 existing save files
were unchanged. Root remains EXE, GameData, saves. No user game was launched.
Physical menu/reconnection and full gameplay-map acceptance remain outstanding;
the controls goal stays active. Evidence: evidence/windows-20261001/XINPUT-POLL-RELEASE.json.

The requested disk-hygiene rule is now in AGENTS.md. Backup temporary files use
the destination volume; its prior workspace regression run passed 46 tests with
one skip (47 discovered). This run omitted unnecessary new save-state copies.
Automatic review blocked deletion of one disposable 128 MiB test SD image; it
remains in the external diagnostic scratch folder, not source control.


## XInput connection and native idle-timeout repair - 2026-10-01

The previous short menu tests missed a real disconnect: the unmodified runner
lost its native Player 1 link at 242.64 guest seconds with the host SDL controller
still connected, 14 bindings resolved and the input gate enabled. Native logs
recorded HCI_CMD_DISCONNECT. The game configures a four-minute remote idle timeout.
The exact-executable/managed-profile adapter now passes zero to the verified
native timeout setter. The original routine performs the update; no direct SDA
write, instruction patch, clock change or synthetic keepalive is shipped.

An intermittent SDL binding loss was also observed while Windows XInput still
reported the controller. Its SDL root cause remains unproven. The launcher now
selects the connected Windows XInput gamepad for unmodified Xbox v5 profiles.
Only the player-one Device entry changes, with the old file backed up. Custom
profiles and other ports are preserved. Reproducible launcher source, packaging
instructions and nine passing profile regression checks are in
tools/windows-launcher; provide the user icon separately when packaging.

Final validation: 600.59 guest seconds with the real XInput backend, all 599
samples connected/bound, with physical button changes observed. No input was
injected in that run. The separate process-local menu test waited 270 seconds
neutral, read native timeout zero, then verified Start to Main, D-pad/stick
right-left selection changes and A/B through Play/Difficulty/profile selection.
All 310 connection samples stayed connected; that menu run exited 0.
Both completed runs used Null graphics/audio and one logical CPU2. A preliminary
Vulkan/Cubeb diagnostic was stopped because its visible simulated-input window
confused the user; their physical presses could not control that window. It is
not counted as physical on-screen or visual/audio validation. Further diagnostic
input runs must stay headless so they cannot be mistaken for the user game.

Supported build: Build.cmd --cpu jit --jobs 2, exit 0; 45/45 tests in 16.69 seconds.
No final compiler/linker warnings/errors; existing CMake warnings remain. The
installed C:\Games\MUA2\OpenMUA2.exe is 10,597,376 bytes. Of 54 existing files
under saves, 53 stayed byte-identical and only WiimoteNew.ini's Device entry
changed. Root layout remains OpenMUA2.exe, GameData and saves. Package payload
hash verification passed. Earlier candidate and preparation failures are retained
in the scoped evidence, not presented as successful runs.

No screenshots, host input, full physical control-map validation, gameplay/FPS,
visual-content or audio-quality acceptance is claimed. The performance goal stays
active. Evidence: evidence/windows-20261001/XINPUT-IDLE-DISCONNECT.json.

## Title-screen Start and game-window icon repair - 2026-10-01

Reproduced the title-screen blocker with the previous runner: after leaving the
attract movie, repeated A/Start presses left the active CMenuStart unchanged.
Xbox Start emitted Pause39/MenuBack90, but this screen requires MenuExit105.
The guarded managed-profile adapter now emits that continue action only when
the verified manager points to the active title-screen object. Other menus retain
pause/back semantics. Invalid, misaligned, truncated and differently typed menu
pointers fail closed. Existing executable and profile guards remain intact.

Final normal-boot Vulkan3x/1080p-preset probe exited 0 on one logical CPU2/mask4.
Start entered the main menu; D-pad and left-stick right/left changed selection
and returned; A/B traversed Main/Play/Difficulty to profile selection. The native
continue consumer observed 105, and all 11 release snapshots cleared tested menu
inputs. Separate private plaza-state tests confirmed Start opens/closes pause,
then Start opens it again and B closes it. Original horizontal navigation already
worked; a proposed direction remap was rejected and removed. Unconditional 105
also failed pause/resume and was rejected. No direction profile change shipped.

Correction to the preceding delay checkpoint: its 48/48 pulses measured native
input production during startup, not successful main-menu navigation. Unchanged
resource strings were also insufficient to infer a stuck screen. This checkpoint
checks the active menu pointer, runtime type, page, selection and action consumer.
Physical controller bindings resolve and the device is connected, but no physical
button test was completed. No screenshots, host input, visual-content validation,
audio-quality validation or combat-FPS acceptance is claimed.

The Windows window class previously requested resource 101 from the Windows system
module rather than the application. It now loads the packaged OpenMUA2.ico beside
the extracted runner, with module/application fallbacks. Native 16/32px loads passed;
taskbar appearance is unverified. The wrapper embeds the icon and verifies it in
its payload manifest, keeping only OpenMUA2.exe, GameData and saves at the game root.
The installed EXE is 10,593,792 bytes; 54 save/profile/cache files remained byte-identical.

Supported Windows build: Build.cmd --cpu jit --jobs 2, exit 0; 45/45 tests in 9.49s.
No compiler/linker warnings/errors in the final build; existing CMake deprecation,
object-path, policy and unavailable-Wayland warnings remain. Native cold-run logs
retain 51 WPAD_ERR_INVALID lines and 7 missing-file messages (drivers.ini, x_voice.zsm,
i109.bik paths). These did not block the tested transitions; they are not dismissed
as full-game/audio validation. Failed preparations and rejected prototypes remain
in the local diagnostics; complete scoped evidence and binary hashes:
evidence/windows-20261001/MENU-START-AND-WINDOW-ICON.json.

## Menu input delay repair - 2026-10-01

Found two defects in the staged user launch. A zero-byte controller profile was
mistaken for configured controls, suppressing discovery and leaving fallback
bindings. ControllerConfigExists now treats empty/whitespace-only files as
unconfigured while preserving nonempty/custom or unreadable files. The new
regression returned 26 before the fix; both controller-family tests now pass.
A real boot from an empty profile generated the connected Xbox v5 profile.

The title also retained simulated optical-disc seek timing while reading local
extracted files. RMSE52's shipped game settings now enable FastDiscSpeed, using
the existing buffered-transfer path and asynchronous completions. Guest/VI/CPU
clocks, single-CPU execution, resolution and graphics quality are unchanged.

Windowed, process-local 50 ms navigation pulses exposed the delay: 13/16 registered,
with five input-evaluation gaps over 100 ms and a 382.346 ms maximum during navigation.
With fast-disc loading, two explicit-setting repeats plus a fresh shipped-default
run registered 48/48. Per-run maximum evaluation gaps: 33.424/33.424/33.452 ms; action
acceptance 13.638-43.600 ms after scheduled input; every release cleared within 50 ms
of its scheduled release. No desktop input or screenshots were used. The runs
started normally in isolated copied save profiles, used CPU 2/mask 4 and Vulkan 3x
at the 1080p preset, and used silent audio. These measure game-side input handling,
not physical-controller-to-display latency or visual menu selection correctness.

The roughly 3.16 s startup transition gap remains and was retained in the traces;
its guest clock advances normally. It is distinct from the navigation read
stalls fixed here. A shader-worker-count experiment did not help and was rejected.
Earlier post-press snapshots could miss already-cleared actions; the new opt-in
OPENMUA2_INPUT_TIMING trace records chronological guarded native action evaluations
(guest ticks, host steady-clock nanoseconds, input-object address, five mask words).
It is disabled in normal launches. Initial invalid release_pad probe was retained
as failed; corrected probes use release=1 and release on the guest clock.

Supported Windows build: Build.cmd --cpu jit --jobs 2, exit 0, 45/45 tests in 9.32 s.
No compiler/linker warnings or errors; existing CMake warnings remain. Evidence,
runner hash and per-press results: evidence/windows-20261001/MENU-INPUT-DELAY.json.
The local user package uses OpenMUA2.exe with embedded runtime, alongside GameData
and saves; logs/cache remain under LocalAppData. Source and game data stay separate.
This checkpoint does not claim audio quality, combat FPS or full performance-goal
acceptance. The physical/visual acceptance gaps in the previous checkpoint remain.


## Three candidate repeats and acceptance audit - 2026-10-01

Three short repeats completed on the same Windows runner as the ten-minute
plaza diagnostic, with the formatter explicitly ON. All exited0 and measured
1422 new-frame intervals without dropped samples or warmup trimming. Averages:
29.97019 / 29.97028 / 29.97037 FPS. P99:36.184 / 35.774 / 36.102ms. Maximum:
51.702 / 53.618 / 55.441ms; each had one frame over50ms and minimum rolling
second29. Guest timestamp sequences matched exactly, all33.367ms intervals,
at normal game-clock speed. Averages alone do not establish solid30 acceptance.

Each repeat retained4 living heroes and5-7 positive-health opponents in16
guarded samples, with health and position changes between every adjacent pair.
Together with the completed600-second route, the numerical repetition/endurance
work is now recorded. These samples do not establish expected effects, animation
or visual behavior. No further identical repeats are needed without a new
question or change.

Full JIT, single logical CPU2/mask4, Vulkan3x/1080p preset, Cubeb70%, identity
cache OFF; no screenshots or host input. Same verified45/45-test runner, no new
source change/build. The formatter stays OFF by default: these are opt-in
candidate results, not normal-launch FPS. Current visual/audible acceptance and
broader animation/interrupt correctness remain unverified, so the goal is not
complete. The latest acceptance ledger and complete metrics/hashes are in
evidence/windows-20260930/V5-CANDIDATE-REPEATS.json.


## Ten-minute v5 diagnostic and sequence-loader fix - 2026-10-01

Sequence loading now normalizes each input path once and uses lexical-path hash
sets for collision checks. The previous nested loop repeated filesystem work
for every output. Existing limits and rejection behavior remain. New tests cover
2048 commands, normalized duplicate/output-to-input aliases, and2049 rejection.
Supported Windows build exited0:45/45 tests in7.22s (protocol1.65s), no compiler/
linker warnings/errors; existing CMake warnings remain. Binary hashes and sizes
are in evidence/windows-20260930/V5-ENDURANCE-SEQUENCE-LOADER.json.

Four failed preparations are retained: too many commands, over600 seconds, and
two missed absolute start deadlines (including one after the loader fix). The
successful retry uses the existing start-now option and1573 commands over exactly
600 scheduled seconds. It is not an identical-state A/B comparison. No limit was
relaxed. The earlier unchanged-source build is not validation of the fix.

The completed route exited0:17981 new-frame intervals across599.9656 host seconds,
29.97005 FPS, normal100.00007% game-clock speed, P99 34.602ms, maximum50.975ms,
two frames over50ms, minimum rolling second29,1%low28.073. Every guest interval
was33.367ms. No warmup trimming; all ten minute averages were29.9696-29.9701.
This is stronger endurance evidence, not completion of solid30 acceptance.

All48 guarded entity samples retained at least3 living heroes and2-7 positive-
health opponents. All36 positive-duration sample pairs changed both health and
positions; the11 unchanged pairs were zero-time cycle-boundary duplicates.
Latched gameplay time advanced599.999418s. These are sampled combat observations,
not continuous visual proof. All599 fully interior audio buckets had zero main
channel empty reads or trims. Full counters retain startup/terminal events.
Private pre-volume PCM retains the final60s, no clipped samples; audible quality
is unverified.

Full JIT, one logical CPU2/mask4, Vulkan3x/1080p preset, normal clocks, Cubeb70%,
formatter explicitly ON and identity cache OFF. The formatter remains default-
off. No screenshots or host input. Three qualifying repeats, broader timing/
gameplay correctness and visual/audio acceptance remain open. User edits and
proprietary data are preserved. Evidence: V5-ENDURANCE-SEQUENCE-LOADER.json.


## Gameplay-clock comparison - 2026-10-01

Read-only samples now check the native clock object identity before reading its
latched gameplay nanoseconds. Static accessor/vtable/return-path analysis supports
the field interpretation. Sixteen samples per mode span 51.16 scheduled seconds,
including a 3.7-second neutral tail after the action measurement marker. Earlier
FPS windows stop at measurement-end.txt, before that tail.

Original formatter: latched gameplay clock advanced 51.159066s (99.9982% of
scheduled elapsed time). Replacement: 51.151058s (99.9825%). Cumulative differences
from scheduled time were -0.934ms and -8.942ms. Largest adjacent-sample differences
were 28.592ms and 29.167ms. The clock is frame-latched and probes are asynchronous;
these sub-frame differences are not proof of zero drift or identical scheduling.
No sampled clock offset changes, pause activation or frame-step activation.

This supports normal overall gameplay-clock progression with the rewrite. It
does not establish individual animation/charge/AI behavior or interrupt equivalence.
The rewrite stays default-off and all performance/correctness gates remain.
Both routes exited 0: full JIT, single CPU2/mask4, normal clocks, Vulkan3x/1080p
preset and Cubeb volume70. No screenshots, host input or audible/visual acceptance.
An initial off-mode command was rejected before launch for an unsupported flag;
omitting that flag corrected it. No source change/rebuild; existing 45/45-test
Windows runner reused. Evidence: evidence/windows-20260930/XBOX-V5-GAME-CLOCK.json.


## V5 formatter comparison remains experimental - 2026-10-01

A contemporaneous opt-in formatter run on the same v5 plaza route measured
29.970 new FPS versus the three default-off runs at 26.659. All 1422 measured
guest intervals were 33.367ms; normal game-clock speed remained 99.9997%.
Host P99 was 36.292ms, maximum 61.831ms, two intervals exceeded 50ms and the
minimum rolling second was 29 FPS. This is not a solid-30 acceptance pass.

Shadow mode executed the original code and completed 1,858,169 comparisons with
zero output/FPSCR/preserved-ABI mismatches. One call was pending at shutdown
and is unverified. Its measured guest timestamp sequence exactly matched the
original baseline; average was 26.659 FPS. Replacement combat outcomes differed:
minimum four living heroes versus three in baseline/shadow. Equal input and
normal clock speed do not prove world/animation/interrupt equivalence.

This repeats the known benefit on corrected controls with enabled audio, not a
new source optimization. Keep the rewrite default-off. It reduces game-library
work but does not retain original loop latency or interrupt opportunities.
Further validation must address those gameplay/timing effects, not just more
output comparisons. No clock changes, reduced effects or relaxed acceptance.

Both routes exited 0 using the existing verified Windows v5 runner (45/45 tests),
full JIT, one logical CPU2/mask4, Vulkan3x/1920x1080 preset and Cubeb volume70.
No rebuild, screenshots, host input, listening/visual acceptance or ten-minute
session. Evidence: evidence/windows-20260930/XBOX-V5-FORMATTER-COMPARISON.json.


## V5 plaza: three failed frame-pacing repeats - 2026-10-01

Three audio-enabled runs of the corrected v5 route reproduced 26.659 newly
rendered FPS over the exact commanded interval (47.414s, 1264 frame intervals;
no warmup trimming). Each minimum rolling one-second rate was 23 FPS. P99 frame
times were 52.766 / 52.263 / 53.232ms; maxima 67.228 / 67.316 / 67.141ms.
All routes exited 0 with zero dropped frame-trace samples, but all FAIL the
sustained nominal 30 FPS requirement. These are not three passing acceptance runs.

Guest frame timestamps matched exactly across all three repeats: 333 intervals
at 50.050ms and five at 66.733ms in each run. Emulated clock speed remained
99.9997-100.0006%. This corroborates the earlier ORIGINAL-FORMATTER-CADENCE
finding with corrected controls and enabled audio; it is not a new host-hitch
fix. Investigate known original formatter/game-side work and frame production
without changing clocks, dropping effects or weakening correctness gates.

Each run has 16 guarded entity samples, at least three living heroes and 3-7
positive-health opponents; every adjacent sample shows position and health
changes. This supports sampled combat activity, not continuous visual acceptance.
Full JIT, one logical CPU2/mask4, Vulkan 3x EFB / 1920x1080 preset, Cubeb volume70;
no screenshots or host input. Audio profiling and probes add diagnostic overhead.
No listening/visual/physical-controller acceptance or ten-minute session.

No source/default changes or rebuild; runner matches the verified 45/45-test
Windows v5 build. Raw traces, audio and game data remain private. Complete metrics,
audio counters and hashes: evidence/windows-20260930/XBOX-V5-FRAME-REPEATS.json.


## Audio-enabled v5 plaza checkpoint - 2026-10-01

The default v5 layout completed the plaza diagnostic route with Cubeb/WASAPI
output enabled at 70% in a disposable profile. Full JIT, one logical CPU,
normal clocks and the existing Vulkan 3x EFB / 1920x1080 preset were retained.
The route exited 0. Neither DMA nor music recorded an empty read while Core
was Running. Each had two queue trims, all in bucket 0; none occurred later.
All empty reads occurred in the final bucket with Core explicitly not Running.
The maximum callback gap was 11.0398ms. Native output logged stereo 48kHz and
a 1056-frame render buffer, with no logged reinitialization or failure.

Sixteen guarded entity probes retained at least three living heroes and 3-7
positive-health opponents; every adjacent pair showed position and health
changes. Private pre-volume PCM spans 57.292s, peak 26209, zero clipped samples.
These are bounded diagnostic observations, not audible or visual acceptance.
No FPS trace, screenshots, host input or physical-controller test was performed.
The v5 route changes prevent an identical behavioral comparison with v4.

No playback, buffer or clock change is justified by this run. Audible crackling,
visual correctness and sustained combat FPS acceptance remain unverified.
Reuses the verified Windows runner from the 45/45-test v5 build; no source change
or rebuild. Raw game audio/data remain private. Evidence and hashes:
evidence/windows-20260930/XBOX-V5-AUDIO.json.



## Default Xbox layout v5 and profile migration - 2026-10-01

Generated profiles now use the MUA1 Xbox 360 common layout with MUA2 LT+face
fusion selection. Matching profile sections automatically enable the guarded
adapters per controller port, without experimental environment flags. Modified
or extra bindings do not opt a port in. Title/evaluator/candidate hash guards
remain. Unmodified generated and runtime-persisted v1-v4 files upgrade with an
exact original backup and rollback on installation failure. Customized files
remain unchanged; stored device IDs win over unrelated current selections.

Supported Windows JIT build passed 45/45 tests in 5.12s, exit 0; no compiler/linker
warnings/errors, existing CMake warnings remain. An initial build was deliberately
stopped to correct a v4-template replacement length; it is not a passing build.
Tests cover real expression evaluation, trigger threshold, modifier/release cases,
legacy backup/custom preservation and per-port recognition. An independently
saved v4 fixture guards against a self-consistent but incorrect migration template.

Five plaza runs completed without override flags: all direct hero slots, expected
LT+B fusion partner and idle cleanup, statue mission flag 1->3, persisted-v5
camera/release, and a mixed managed/custom controller profile. Only managed port0
was rebound in the mixed run; custom port1 kept its native use descriptor and
profile settings. Original v4 backups were byte-identical; reopening persisted v5
did not add another backup. One logical CPU, normal clocks, Vulkan3x, Null audio;
no screenshots/host input. Some functional probes overlapped build tail; no FPS claim.

Back's menu readiness/return timing, visual/physical-controller/audio acceptance,
fusion cancellation/revival, vertical camera and other pointer interactions remain
open. Actual untouched user profiles upgrade on the next normal launch; customized
profiles require deliberate selection of the new layout. Performance acceptance
is unchanged. Evidence/binary hashes: evidence/windows-20260930/XBOX-V5-MIGRATION.json.


## Experimental Back / Hero Management - 2026-10-01

With OPENMUA2_HERO_BUTTONS=1 and the matching experimental profile, Back now
requests native HeroManagement40. The adapter consumes its camera/pause markers
and shared menu aliases. The profile excludes Start from the camera-enable source,
so Start + right stick remains pause/resume and cannot synthesize Hero Management.
Defaults remain v4; this is not an installed physical-controller migration.

Windows JIT build passed 45/45 tests in 9.07s, exit 0; no compiler/linker warnings
or errors, existing CMake warnings remain. Live Back probes show only action40
besides the standing cursor baseline; release and port isolation pass. Gameplay
entity updates stop after Back. B alone did NOT resume, and early A then B also
failed. After an eight-second neutral wait, A then B restored entity updates;
a separate longer sequence also returned. This is evidence of a timing-sensitive
return path, not proof of menu contents or a particular confirmation dialog.
No fixed delay or forced menu exit is added. Start+right-stick regression passed.

Six routes completed with exit 0; one earlier exit route was rejected for a
duplicate output filename, then corrected in a separate run. Preserve those
functional failures; command success alone is not menu acceptance. First two
probes overlapped build tail. All use one logical CPU, normal clocks, Vulkan3x,
Null audio, no screenshots/host input. Visual menu readiness, broader button
combinations, physical controls/audio, default migration and FPS remain open.
Evidence/profile/binary hashes: evidence/windows-20260930/BACK-HERO-MANAGEMENT.json.


## Experimental camera/menu separation - 2026-10-01

The experimental hero profile restores horizontal right-stick camera input;
LT reserves that stick for aiming. With OPENMUA2_HERO_BUTTONS=1, the guarded
adapter consumes MenuViewBoosts99, MenuSwitchToPrevHero104 and MenuReallocate122
from the shared camera-enable source, preserving signed CameraX and turn inputs.
The initial private probe exposed those aliases; it was not shipped as a default.

Windows JIT build exited 0: 45/45 tests, 9.04s; no compiler/linker warnings/errors,
existing CMake warnings remain. Three subsequent plaza probes exited 0. Two
11-snapshot camera probes preserve positive/negative camera values, LT suppression,
release and port isolation without the three menu aliases. The 11-snapshot paused
probe keeps entity position/health frozen during stick input, then resumes their
updates after Start. The final run used the corrected checked-in profile; parsed
persisted bindings match (comments/whitespace are normalized on serialization).

One logical CPU, normal clocks, Vulkan3x, Null audio; no screenshots or host input.
The first two runs overlapped build tail; none is FPS evidence. Native routing and
entity observations do not establish visual camera/menu correctness or physical
controller/audio acceptance. Defaults remain v4. Back/hero management, vertical
camera support, broader menu/fusion interactions and default migration remain open.
Evidence/profile/binary hashes: evidence/windows-20260930/CAMERA-MENU-ADAPTER.json.


## Experimental Start pause/resume - 2026-10-01

The guarded input observer now maps Start to native Pause39 and MenuBack90 when
OPENMUA2_HERO_BUTTONS=1. It suppresses attack/grab/use, interaction shake/lift and
hero-menu actions during Start. The experimental profile still maps only Start to
the native pause source; no synthetic B source is needed. Defaults remain v4.

Two single-logical-CPU, normal-clock, Vulkan3x plaza probes exited 0. Guarded
entity position/health changed before pause, froze between paused observations,
and changed again after the second Start. Both 200ms Start and 1200ms Start+A
worked; active input returned to CursorY-only baseline on release, other ports
remained inactive, and Start carried no attack/use or hero-menu action. Start+A
retains native MenuAccept89; the first analysis assertion was too narrow and was
corrected after inspecting that input. The discarded synthetic-B prototype leaked
SmashAttack10. Null audio, no screenshots or host input. Numeric evidence does not
verify visual menu behavior, audible quality, physical controllers or combat FPS.

Supported Windows JIT build exited 0: 45/45 native tests, 8.38s. No compiler/linker
warnings or errors; existing CMake configuration warnings remain. The first probe
overlapped the build tail and is not performance evidence. Full control migration,
other simultaneous menu/fusion/interaction inputs, audio and FPS acceptance remain
open. Evidence/binary hashes: evidence/windows-20260930/START-MENU-ADAPTER.json.

## Audio queue trims and native output log - 2026-10-01

Added opt-in counters for queue-trim events, discarded granules and Running/not-
Running attribution. The analyzer preserves older logs as lacking trim evidence
and rejects inconsistent new counters. Playback, buffer limits and defaults are
unchanged; this is diagnostic work, not an audible crackling fix.

Native application logs show WASAPI requested/mix formats both at stereo 48kHz,
a 1056-frame (22ms) render buffer, and no logged device reinitialization/failure.
The instrumented plaza route had three trims per main channel, confined to buckets
0 and 56. Buckets 1-55 had neither queue trims nor empty reads. Replacing its closing
pause/resume sequence with equal-duration neutral gameplay left only bucket-0
trims and ZERO empty reads while Core was Running. All later empty reads in that
run were not-Running. This supports a transition association, not exact timing or
an audible-clean claim. Do not enlarge buffers or change clocks from these counts.

All three runs exited 0. The no-menu run decoded 16 guarded entity probes (at least
three living heroes, 4-7 positive-health opponents), captured 57.772s of private
pre-volume PCM with zero clipped samples, and recorded maximum callback gap 11.1441ms
and work 1.6604ms. Full JIT, one logical CPU, normal clocks, Vulkan3x, muted Cubeb;
no host input, screenshots or FPS measurements. Physical output used during the
user's earlier crackling test has not yet been confirmed. Listening/device output,
full control migration and combat performance acceptance remain open.

Windows build: 45/45 native tests, 5.87s, exit 0; Python: 169 passed, 1 skipped (170 run,
42.995s). No compiler/linker warnings/errors; existing CMake warnings remain. An
initial build was deliberately stopped to correct trim-only bucket retention.
Evidence and binary hashes: evidence/windows-20260930/AUDIO-QUEUE-TRIMS.json.


## Experimental direct D-pad hero selection - 2026-10-01

OPENMUA2_HERO_BUTTONS=1 connects an experimental D-pad profile to the native
single-index hero handoff. Up/Right/Down/Left select ordered roster slots 0/1/2/3.
The adapter validates title/code, per-player input ownership, live full handles,
and the current caller's actor array. It changes the requested index and limits
the native search to one attempt; the game retains eligibility and handoff logic.
No temporary actor-array pointer is retained. Default profiles remain v4.

The supported Windows JIT build passed 45/45 tests in 7.66s, exit 0, with no
compiler/linker warnings or errors; existing CMake warnings remain. The final
21-snapshot plaza probe selected all four expected heroes, left self/diagonal
requests unchanged, preserved selection during a hold, suppressed gameplay power
bits, and returned to the initial CursorY-only baseline on release. Only injected
port 0 responded. An initial synthetic-plus menu exit/skill-point leak was fixed;
those actions are absent in the repeat. LT+B fusion also selected Iron Man and
returned to idle with the request owner cleared while the new observer was enabled.

Fixture: project/lib/ModernGekko/tests/data/mua1_hero_experimental.ini (process-local
test device; requires both HERO_BUTTONS and FUSION_BUTTONS flags, not an installed
physical-controller profile). Tests used one logical CPU, normal clocks, Vulkan3x,
Null audio, no screenshots or host input. Numeric native behavior is not visual,
audible, physical-controller or full menu validation. Dead/co-op cases, default
migration, camera/menu integration, cancellation, audio and FPS acceptance remain
open. Evidence/binary hashes: evidence/windows-20260930/DIRECT-HERO-BUTTONS.json.



## Fusion timeout and retry evidence - 2026-10-01

Two new prepared-plaza runs distinguish delayed native exit from release-to-cancel.
After the tutorial Ready confirmation, releasing LT left phase 1 at the 500ms
snapshot and reached phase 0 later. Holding LT for 6500ms also reached phase 0.
Both retained request_owner; that field alone is not evidence of a stuck request.
A subsequent LT request advanced the sequence from 1 to 2, LT+B selected the
expected Iron Man actor, and the native flow returned to phase 0 with owner cleared.
Immediate release cancellation is still unimplemented/unverified. Do not clear
native globals merely because the owner remains populated during phase 0.

Both corrected routes completed with exit 0. An initial hold/retry route was
rejected for a boolean button value; the numeric-input correction used a new
output directory. No runtime source changes or rebuild: the previous 44/44
Windows result still applies to 539b16a8. Single logical CPU, normal clocks,
Vulkan 3x, Null audio, process-local input; no screenshots or visual/audio/FPS
acceptance. Full Xbox migration and the performance goal remain open.
Evidence: evidence/windows-20260930/FUSION-TIMEOUT-RETRY.json.

## Experimental native fusion partner selection - 2026-10-01

OPENMUA2_FUSION_BUTTONS=1 now substitutes a validated roster actor at the native
picker boundary, preserving the game's subsequent eligibility/resource processing.
It requires the matching experimental marker profile; it is OFF by default and
must not be treated as a completed v4/custom-profile migration. A/B/X/Y address
ordered roster slots 0/1/2/3; self and other human-controlled actors are rejected.
No host selection cache is retained. Title/code, input ownership and live full
entity handles are checked. Runtime start hooks now support multiple addresses.

Windows build passed: 44/44 tests, 7.76s, no compiler/linker warnings or errors;
existing CMake warnings remain. In the prepared plaza team, LT+A/B/Y selected
three distinct expected teammates and returned to idle. LT+X (self) selected no
partner. All four runs passed marker/power suppression, release and port-isolation
checks. The existing X/A/B statue interaction still completed its mission flag.

The first B attempt failed because its profile suppressed tutorial confirmation.
The corrected test fixture keeps manual aiming/confirmation separate from native
partner selection. Fixture: project/lib/ModernGekko/tests/data/mua1_fusion_experimental.ini.
This is a process-local test-device profile, not the user's installed layout.

Release-to-cancel, revival, other teams, multiplayer, menu polish and default
profile migration remain open. Numeric selection/state evidence does not verify
animations, effects or audible output. One logical CPU, normal clocks, Vulkan3x,
Null audio; no screenshots, host input or new FPS claim. Evidence and binary
hashes: evidence/windows-20260930/NATIVE-FUSION-BUTTONS.json.


## RT/LT and direct-selection research - 2026-10-01

A disposable profile routes RT+face to all four powers. The corrected 15-snapshot
probe passes modifier suppression, release and port isolation. An initial
both-trigger Block/PowerShift2 leak was found and corrected in that private
profile. These are routing results, not power animation/effect acceptance.

LT enters the native fusion-request state in two plaza runs. Neither release nor
subsequent B cancels it in the prepared tutorial save. Partner selection and
fusion completion are not established. The ordered native four-actor roster is
now identified and matched against live actor handles/types; target/sequence
encoding is located, but its complete eligibility/consumption contract remains
under investigation. Fixed pointer corners must not substitute for direct selection.

Production defaults and source binaries are unchanged. No rebuild was needed;
the preceding 43/43 result applies to dd302e0a. One logical CPU, Vulkan3x, normal
clocks, Null audio, process-local input and read-only memory probes; no screenshots,
visual/audio acceptance or new FPS claim. Next: native partner selection/cancel,
then profile integration/migration. Evidence: evidence/windows-20260930/TRIGGER-FUSION-RESEARCH.json.


## Experimental context-aware button gestures - 2026-10-01

The opt-in OPENMUA2_CONTEXT_X=1 adapter now replaces shake/lift with A/B only
inside a verified human-owned co-op interaction. X enters using the existing
normalized grab/use chord. Repeated A presses supply shake edges; B supplies
lift. The corresponding light/heavy attacks are consumed inside this context.
The adapter checks live handles, owner, actor/node/target types and current
string generation. It remains OFF by default; v4 default controls are unchanged.

Supported Windows JIT build passed: 43/43 tests, 8.08s, exit 0. No compiler/linker
warnings or errors; existing CMake warnings remain. Two original-save plaza runs
completed the statue interaction (mission flag 1 -> 3), with context entered and
exited. Nine input snapshots per run confirm gesture routing and release. A
separate 14-snapshot probe preserves ordinary A attack outside the interaction,
release and port isolation; it does not establish the complete Xbox layout.

Only process-local input was used, on one logical CPU with normal clocks,
Vulkan 3x and Null audio. No screenshots, visual/audio acceptance or FPS sweep.
Other interactions, menus, multiplayer and holding inputs across context exit
remain unverified. Historical default-off LB failures remain unresolved.
LT + indicated face button remains the fusion target; this patch does not
implement fusion selection. Full control migration, audio and performance remain
open. Evidence and binary hashes: evidence/windows-20260930/CONTEXT-BUTTON-GESTURES.json.


## Experimental X value normalization - 2026-10-01

The guarded adapter now runs after evaluation and preserves the original active
bits while converting the accepted use chord to digital 1 and inactive partial
chords to 0. It remains opt-in and OFF by default. Unexpected output is rejected.
Windows build passed, 42/42 tests in 6.47s, no compiler/linker warnings or errors;
existing CMake warnings remain. No changes to the single-core rendering preset.

Two normalized X plaza runs completed the statue interaction. A 14-snapshot
probe confirmed X use value 1, zero on partial/released inputs and other ports,
with expected action routing. However, two current default-off LB comparisons
failed despite earlier stock success. This unresolved comparison prevents a
reliability/regression-free claim. Do not enable by default or call the full
Xbox remap complete. Default profiles remain v4; LT fusion target is unchanged.

No screenshots, host input, visual/audio acceptance or new FPS sweep. Null audio
was used. Details and binary hashes: evidence/windows-20260930/CONTEXT-X-NORMALIZED.json.
Audio, remaining controls and the sustained performance goal remain unfinished.


## Contextual X behavioral failure - 2026-10-01

The same statue-use-ready plaza state now has a guarded, read-only mission
completion check. Stock LB completes the interaction; experimental X does not.
Adding the original block input to X also fails. The adapter remains OFF by
default. Passing active-action bits did not establish working contextual use.

Four bounded routes completed with exit 0 using the existing 36c73706 binary:
stock LB, experimental X, experimental LB negative control, and X plus block.
Only stock LB set the script completion flag. These are behavioral results from
mission state, not visual verification. Null audio, one logical CPU, normal
clocks, Vulkan 3x; no screenshots, host input or new FPS sweep. No rebuild was
needed for this evidence/documentation checkpoint. Existing 42/42 build tests
remain scoped to the previous build. Audio and full Xbox remapping remain open.

The next control fix must resolve native context/consumption behavior; neither
simultaneous grab/use nor binding-value magnitude is yet proven as the cause.
Evidence: evidence/windows-20260930/CONTEXT-X-EXPERIMENT.json.


## Live Xbox action routing probe - 2026-10-01

## Experimental X-use separation - 2026-10-01

Added an opt-in action-descriptor adapter (OPENMUA2_CONTEXT_X=1, default OFF).
It verifies executable/live-code identity and the current input object's layout
before rebinding use to X's grab chord. The original game evaluator and action
queues still run. Unexpected descriptors are left unchanged; repeated application
is idempotent. A runtime-owned start observer is cleared at title shutdown and
retained across hook reload; settings-reload behavior is not separately tested.

Supported Windows build passed: 42/42 native tests, 6.05s, exit 0; no compiler or
linker warnings/errors. Existing CMake warnings remain. Two plaza routing probes
passed with 14 guarded snapshots each: enabled X -> grab/use, LB -> block only;
disabled retains v4. Tested actions clear on release, only injected port 0 responds.
Normal clocks, CPU2/mask4, Vulkan3x, Null audio, no screenshots or host input.
These are routing results, not completed interaction, visual, audio or FPS proof.

The first live attempt rejected the executable due to digest text case, and its
fixed guest start was rejected too. Both failures remain in private logs. Corrected
the digest comparison; successful routing runs start immediately. Full evidence
and binary hashes: evidence/windows-20260930/CONTEXT-X-EXPERIMENT.json.

Fusion target is corrected to MUA2 Xbox 360: hold LT plus the indicated partner's
A/B/X/Y button, as requested. This and RT powers/direct D-pad selection remain
unfinished. Default controls remain v4; performance goal remains unmet.

## Corrected control target - 2026-10-01

The user rejected v4's routing as the desired layout and specified MUA1 Xbox 360
as the template. RT must modify powers, D-pad must select heroes directly, X must
handle contextual use/grab, and LB must block/dodge independently. The proposed
MUA2 extension is LT fusion/partner selection, including fallen-hero revival.
The current binary remains v4; no completed-remap claim. Implementation and
validation requirements are in docs/MUA1-XBOX-CONTROL-PLAN.md. Audio/controls
remain the priority; no renewed FPS sweep. No runtime change or rebuild here.

## Hero ownership research - 2026-10-01

A bounded plaza route used eight RT presses and nine identity-guarded entity
snapshots. All four heroes remained alive. One candidate control flag followed
one hero through two identical four-hero cycles; code inspection also found it
used when counting controlled team actors. An indexed team-identity lookup was
identified, but its identities are not yet mapped to live fixed D-pad slots.
This is research evidence, not a finished ownership or direct-selection adapter.
Dead-hero and multiplayer behavior remain untested. Contextual X remains open.

Harness/native exit 0; 4.5 guest seconds, CPU2/mask4, Vulkan3x, normal clocks,
silent Null audio. No source change, rebuild, screenshots, host input, guest
memory writes, visual/audio correctness or FPS acceptance. Existing df47dfab
binary retained. Raw data stays private; aggregate evidence:
evidence/windows-20260930/XBOX-HERO-OWNERSHIP-RESEARCH.json.
Audio/controls remain the active priority; no renewed FPS sweep.

A bounded tutorial-plaza probe confirmed the input layer's active-action bits:
A -> light attack; X -> grab; LB -> block AND contextual action; RT -> next hero;
LT -> previous hero; D-pad Up -> power 4. Only the object corresponding to
injected controller port 0 responded. Tested actions cleared on every release.
Partial nonzero binding sums are not activated actions; the active bitset was
checked separately. This confirms routing, not animations or selected heroes.

Four input objects were found in the existing save and checked in execution.
The first probe cancelled on a transient saved pointer guard. A read-only
follow-up confirmed object/vtable addresses but disproved that pointer's
stability. The successful route guarded each object's vtable and first two
descriptor IDs/names before every snapshot: 14 snapshots, 12 guards each,
5.6 guest seconds. An intermediate pointer-only guest sequence was rejected
because timed input was required; corrected with a brief neutral hold.

No mapping/source changes or rebuild. Used the df47dfab Windows binary (41/41
native tests at that build). Successful harness/native exits 0. CPU2/mask4,
normal clocks, Vulkan3x, default formatter, silent Null audio, no screenshots,
host input, guest writes, frame trace or FPS acceptance. Raw research and probes
remain private. Evidence: evidence/windows-20260930/XBOX-LIVE-ACTION-ROUTING.json.

Contextual X and direct hero-slot selection remain open. The next implementation
must integrate with per-player game actions/context rather than assume a raw
D-pad remap selects a hero. No claim that the full performance goal is complete.


## Xbox rumble and persisted profile migration - 2026-10-01

Fixed a concrete output mismatch: generated profiles used Wiimote-only Motor,
while SDL/XInput gamepads expose Motor L and Motor R. Profile v4 routes rumble
to both. A process-local output test through the real expression parser verifies
strength and release on both motors; the old expression resolves zero outputs.
No physical controller is driven. Physical/game-triggered vibration is unverified.

The runtime saves INI profiles without the generated banner or empty sections.
The previous exact-template upgrade detector consequently missed its own saved
profiles. It now accepts both exact generated and known persisted forms for
v1-v3, preserving device selections and exact .pre-v4-*.bak backups. Modified
bindings or extra comments remain untouched. All six forms are regression-tested;
a copy of the actual prior run's v3 profile also upgraded with an exact backup.
Legacy templates contain only our own controller configuration, not game assets.

Replaced the bottom quick-reference table in docs/PERFORMANCE-GOAL.md with the
implemented layout. X grabs; LB handles the tested statue use; LT/RT cycle
heroes; D-pad is menus/direct powers; Start pauses and B resumes. Contextual X,
direct D-pad hero selection and full fusion/revive remain unfinished.

Supported Windows build passed: 41/41 native tests, 9.02s. Existing CMake warnings
remain; no compiler/linker warnings/errors. No game launch, screenshot, host
input, new audio/FPS measurement or acceptance claim. Full binary hashes and
scoped evidence: evidence/windows-20260930/XBOX-RUMBLE-PROFILE-V4.json.
The performance goal remains unmet; user priority remains audio/controls.


## Audio running-state attribution and B resume - 2026-10-01

Added opt-in diagnostic counters for empty mixer reads while Core is Running
versus not Running, reusing the existing state read. Playback and defaults are
unchanged. The analyzer accepts old profiles without inventing state evidence,
and requires new state counts to sum to the retained totals.

One plaza route completed: 50 DMA and 76 music empty reads in final bucket 57
were ALL observed while the core was not Running. During Running: ZERO DMA and
ONE music empty read (second 53). The prior uninstrumented run cannot be labeled
retroactively. In-game menus still count as Running; the remaining music event
cannot yet be precisely assigned to combat or a menu. No audible-clean claim.
Maximum Cubeb callback gap: 11.5406ms. Private pre-volume PCM: 57.132s, peak30846,
zero clipped samples. These numbers do not establish device playback quality.

Start paused entity activity; B resumed it (eight position and one health
changes between the resumed probes). All 15 guarded combat probes decoded,
living heroes minimum three, identified positive-health opponents 4-7. Profile
migration again retained an exact backup. Native game and harness exited 0.
No game left running. No host input, screenshots, visual/listening assessment,
physical controller operation or FPS measurements. Same CPU2/mask4, Vulkan3x,
normal clocks and default formatter; JIT budget/reserve OFF.

Supported Windows build passed 41/41 native tests (6.45s). Python: 167 passed,
one skipped (168 run, 37.962s). An initial incomplete call-site patch failed
compilation; corrected and rebuilt. Final compile/link had no warnings/errors;
existing CMake warnings remain. Binary hashes and all scoped results:
evidence/windows-20260930/AUDIO-CORE-STATE.json.

Audio is not broadly starved in this bounded run; avoid treating shutdown reads
as combat dropouts. Listening/device playback and remaining Xbox contextual-use,
hero selection and fusion/revive gaps remain open. The performance goal remains
unmet. Continue the user's audio/controls priority without FPS sweeps.


## Audio counter interpretation correction - 2026-10-01

Read-only analysis of the last plaza profile shows NO main-channel empty reads
in complete seconds 0-52. DMA and music each have one empty read in second 53.
The other 61 DMA / 92 music reads occur in final bucket 57. Prior totals were
correct, but must not be presented as that many combat dropouts. Each count is
a granule read, not an audible event. The final burst remains in the evidence;
its coincidence with the end does not prove shutdown caused it. Guest probes
lack host timestamps, so precise combat/menu/stop attribution remains unknown.

Added tools/analyze_audio_profile.py to retain total and final-bucket counts
separately, with explicit coverage checks. Missing/duplicate rows and mixed
clock origins fail rather than silently becoming zero underruns. It neither
measures FPS nor establishes audible quality. Existing PCM and profile remain
private; the aggregate and profile hash are in AUDIO-CONTROLS-CONTINUITY.json.

Python regression suite: 164 passed, one skipped (165 run, 22.662s), including
seven new audio analysis tests.

No runtime source change, rebuild, game launch, capture or FPS sweep in this
checkpoint. The prior runner hash remains verified unchanged. Audio listening,
device-path behavior and remaining Xbox control gaps are still open; the full
performance goal remains unmet. Continue audio/controls, per user direction.


## Audio continuity and Xbox profile upgrade - 2026-10-01

Current user priority: audio crackling and Xbox controls/menus. Stop the previous
FPS hiccup investigation and comparison sweeps. The 30 FPS goal remains unmet.

Continuous DMA/music audio now primes once to half the configured buffer before
consuming samples. At the default 80ms capacity this adds roughly 40ms of onset
latency. Short auxiliary sounds do not wait. No clocks/pitch/input timing changed.
The new offline test disables gap filling and verifies sample-identical output
with and without a 20ms producer interruption for both 32kHz DMA and 48kHz music.
Measured synthetic onset: 47.9062ms / 45.3229ms; these are not hardware latencies.

Unmodified generated Xbox v1/v2 profiles now upgrade automatically to v3, keeping
the selected devices and an exact .pre-v3-*.bak original. Customized profiles are
preserved. Normal startup checks existing profiles even when config.ini has no
controller selection. Tests cover old versions, devices and custom preservation;
the actual runner also upgraded a private v2 copy and retained an exact backup.

One 51.16-guest-second plaza route completed with muted Cubeb processing and
57.202 seconds of private pre-volume PCM: peak 30846, zero clipped samples.
However DMA/music still recorded 62/93 empty dequeues; maximum callback gap was
11.7508ms. This does NOT establish clean sound. Crackling remains unresolved.
All 15 guarded entity probes decoded; living heroes fell to three, opponents
with positive health ranged 4-7. Start-to-resume expectations failed.

A short targeted plaza menu check, with four heroes alive, established the
numeric behavior: active positions/health changed; Start froze them; a second
Start left them frozen; B restored position/health changes. Use Start to pause
and B to resume. This is process-local functional evidence, not menu visual QA.
No screenshots, host input, listening, physical controller or FPS measurement.
Both native game processes exited 0; no game left running. Whole process on
CPU2/mask4, Vulkan 3x/1080p preset, normal clocks; experimental formatter, JIT
budget and backpatch reserve all OFF. Raw game data/probes/audio stay in .local.

Windows Build.cmd --cpu jit --jobs 2 exited 0; 41/41 native tests passed (8.89s).
Python: 157 passed, one skipped (158 run, 23.205s). Existing CMake deprecation,
Wayland, object-path and CMP0069/lz4 warnings; no compiler/linker warnings/errors.
Runner: 16111104 bytes, SHA256
41944d85527927ffbedf466f755905506e2889b47c67ae2b75be34ff1c270523.
Other binaries, scoped results and failed checks:
evidence/windows-20260930/AUDIO-CONTROLS-CONTINUITY.json.

Next work remains audible/device-path crackling diagnosis and remaining Xbox
contextual-use, direct hero-selection and fusion/revive gaps. Do not resume FPS
sweeps without new user direction. Preserve user StaticRecomp edits and launcher.


## Guest-clock plaza sequence and matched-state repeats - 2026-10-01

Added a preloaded process-local xbox_sequence command and benchmark option
--guest-sequence-start. Timed inputs, read-only snapshots and byte guards run
on one absolute guest-time schedule; snapshots/receipts flush afterward. No
host input, screenshots, guest memory writes, clock changes or default runtime
setting changes. Guard mismatch cancels remaining sequence input and releases
its synthetic controller. See docs/TESTING-GUEST-SEQUENCES.md.

Prior receipt inspection found 3.174-4.276 seconds of inter-hold gaps in roughly
50-second routes, with individual gaps up to 419.835ms. New sequences executed
106 holds over 47.46 guest seconds with ZERO gaps. All six used the same start
tick; maximum boundary lateness was 69 guest ticks. Every run's 15 entity-pool
snapshots matched the first control byte-for-byte. All four heroes remained
alive and 5-7 identified opponents had positive health at all sampled boundaries.
Each run recorded 22 hero health decreases, 27 same-slot opponent health decreases
and 56 hero position changes. Slot reuse remains a caveat for damage inference.

Budget ON means the existing experimental 4000us soft JIT budget. Order below
is actual execution order; the last pair is reversed. Primary statistics use
identical guest frames after TWO GUEST SECONDS of warmup, not two wall seconds.

| Run | New FPS | P99 ms | Worst ms | >50ms frames |
| --- | --- | --- | --- | --- |
| off1 | 29.837607 | 36.0059 | 212.9583 | 4 |
| on1 | 29.970241 | 35.2895 | 42.8217 | 0 |
| off2 | 29.969997 | 35.1034 | 55.8543 | 1 |
| on2 | 29.970141 | 35.0907 | 41.3686 | 0 |
| on3 | 29.472984 | 54.9239 | 104.3845 | 26 |
| off3 | 29.783237 | 48.4563 | 81.2486 | 9 |

Full-route on3 was worse: 190.1731ms maximum and 36 frames over 50ms. Full-route
off3 had 11 over50ms; other maxima/counts were unchanged by warmup. All six routes
and native processes exited successfully, but pacing failures remain failures.
The intentional wrong-guard test failed the harness as expected, cancelled the
later input and stopped the native process cleanly. No probe/state assets in Git.

Supported Windows build passed 40/40 native tests; expanded sequence cases also
passed in the final 4.83s native suite. Python: 157 passed, one skipped, 19.444s.
Existing CMake warnings only; no compiler/linker warnings/errors found. Runner:
16092160 bytes, SHA256 3d549b1e8a8393710ed43e91415d29fed66d1d847596919d7aaf60d1ea63248f.
All six produced-binary hashes, full/primary timing and failures are recorded in
evidence/windows-20260930/PLAZA-GUEST-SEQUENCE.json. Preserved user static edits.

Single CPU2/mask4, full JIT, Vulkan3x/1080p preset, normal clocks, muted Cubeb
processing, experimental formatter/identity cache ON, reserve524288. Runtime
profiling OFF. No screenshots or audible/visual correctness claim. Goal UNMET;
this is not three qualifying repeats or ten-minute continuous combat acceptance.
Keep the JIT budget default-OFF. Matching sampled state and input deadlines now
support more controlled investigation, but do not prove all CPU/render behavior.
Next correlate intermittent stalls with target-process CPU measurements and
separate runtime-span diagnostics; profiling itself changes budget decisions.
Continue mixed JIT/interpreter correctness work before promoting experiments.


## Default-off soft JIT compilation budget prototype - 2026-10-01

Added OPENMUA2_JIT_BUDGET_US (decimal 250-16000 microseconds; unset/0/invalid is
OFF). It tracks host compilation time over approximately 1/30-second guest-time
windows. After the allowance is spent, a dispatcher-only path executes up to
256 cold instructions, charges their reported cycles and updates memory mapping.
Revisited PC/CPU-feature keys trigger ordinary JIT compilation immediately,
even above the allowance. Hooks, debugging, stepping and static fallback retain
ordinary compilation. Cache clearing resets budget and hints. The dispatcher
explicitly checks timing after interpretation and handles shutdown.
The initial hard 4ms allowance FAILED: 18.2773 FPS, 905.027ms worst frame and
659,574,606 interpreted instructions. Reuse promotion reduced interpreted work
to roughly half a million instructions in early revised trials. It is now part
of every enabled budget configuration; the hard-only version was rejected.

All timing below uses a 4ms soft allowance when ON; off variants disable it.
Runtime profiling is OFF. First row is the rejected initial implementation.

| Variant | New FPS | P99 ms | Worst ms | >50ms frames |
| --- | --- | --- | --- | --- |
| on1 | 18.2773 | 801.947 | 905.027 | 56 |
| promote-on1 | 29.9698 | 35.687 | 41.612 | 0 |
| off1 | 29.9707 | 35.481 | 55.136 | 1 |
| promote-on2 | 29.5728 | 53.185 | 177.381 | 18 |
| off2 | 29.8997 | 47.048 | 79.242 | 10 |
| promote-on3 | 29.8626 | 46.689 | 62.326 | 4 |
| cpu-on2 | 29.9698 | 35.959 | 45.611 | 0 |
| cpu-off2 | 29.9700 | 35.295 | 56.501 | 1 |
| final-off | 29.9703 | 35.949 | 86.612 | 2 |
| final-on | 29.9700 | 36.272 | 46.271 | 0 |

All ten routes completed with 15 guarded entity probes each. Results are MIXED:
the final pair improved worst-frame time from 86.612 to 46.271ms, but earlier
revised runs reached 177.381 and 62.326ms. Successful pairs did not improve P99.
Do not discard failed runs, claim consistent gains or enable this by default.
Probe receipt waits can vary input publication timing; enemy counts differ, so
equivalent workload is not established. Larger intermittent drops remain unexplained.

Supported Windows build passed 40/40 native tests (5.47s), including new budget
and hotness-hint regression cases. Python: 156 passed, one skipped (19.561s).
Existing CMake warnings only; no compiler/linker warnings/errors found. Final
binary OFF and ON plaza checks both exited 0. Six binary hashes and all runs:
evidence/windows-20260930/PLAZA-JIT-COMPILE-BUDGET.json.

Single logical CPU2/mask4, full JIT, Vulkan3x/1080p preset, normal clocks, muted
Cubeb processing, experimental formatter/identity cache ON, reserve524288.
No screenshots or host input. Game data, raw probes and failed launcher logs
remain private. The optional Windows counter script was blocked by execution
policy; no policy change/bypass. A corrected target-only CPU sampler completed.

Goal UNMET. This default-OFF prototype needs stronger mixed JIT/interpreter correctness
coverage and controlled-input comparisons; current tests do not exhaustively
verify MMU, exceptions, FPSCR or timing behavior. Full runtime profiling changes
allowance decisions. Numeric entity activity is not visual/audio acceptance;
three qualifying repeats and ten-minute combat validation remain outstanding.


## Read-only plaza CPU correlation - 2026-10-01

Three additional plaza runs used a target-process-only, read-only CPU sampler
at nominal 20ms intervals, with full runtime profiling OFF. All routes and
samplers exited 0, with 15 guarded entity probes each. No screenshots or host
input. Existing single-CPU, normal-clock, Vulkan 3x / 1080p preset retained;
muted Cubeb, experimental formatter/identity cache ON and reserve 524288.

| Run | New FPS | P99 ms | Worst ms | >50ms frames | Typical CPU fraction |
| --- | --- | --- | --- | --- | --- |
| 1 | 29.9702 | 35.985 | 53.825 | 2 | 69% |
| 2 | 29.9704 | 35.297 | 51.531 | 1 | 70% |
| 3 | 29.9705 | 34.969 | 58.329 | 1 | 68% |

CPU fractions bracket approximately one-second frame windows and include all
threads on the one assigned logical CPU. Windows accounting is quantized;
time not executing can mean deliberate waits or descheduling. The earlier
18/24 FPS episodes did not recur, so their cause remains unresolved. Do not
dismiss those failures or claim these diagnostics as qualifying acceptance.

No source change or performance gain. Reused the prior restored runner
(77ab0b04c79d5bcc75f11968864b60c15f781ceb4f7a76b89bb3f5dd3ac0b0bb);
its prior Windows build passed 39/39 native tests. No redundant rebuild.
Evidence: evidence/windows-20260930/PLAZA-PROCESS-CPU-CORRELATION.json.

Next investigate a bounded first-use compilation budget, with cold instructions
executed accurately until compilation can resume. Before implementation, account
for cycles, timing redispatch, exceptions, memory-base updates, hooks, stop state
and cache resets; exclude debugging and static fallback. SingleStep() cannot be
used because it overwrites timing-slice state. No tiering code is implemented or
validated at this checkpoint. Goal UNMET; audio and visual limits remain.


## Register lookahead experiment rejected; pacing failures retained - 2026-10-01

Tested sharing the JIT register allocator's forward-use scan while preserving its
lookahead boundary and scoring formula. A plaza shadow run completed without a
reported count mismatch (comparison count not recorded). Two unprofiled OFF/ON
pairs did not show a consistent worst-frame improvement:

| Run | New FPS | P99 ms | Worst ms | Frames >50 ms |
| --- | --- | --- | --- | --- |
| OFF 1 | 29.9701 | 35.125 | 52.553 | 1 |
| ON 1 | 29.9704 | 35.073 | 48.823 | 0 |
| OFF 2 | 29.9699 | 35.233 | 48.924 | 0 |
| ON 2 | 29.9700 | 35.136 | 49.935 | 0 |
| Restored original | 29.7825 | 51.774 | 132.493 | 17 |
| Restored repeat | 29.6546 | 49.642 | 69.375 | 14 |
| Restored, profiled | 29.9701 | 35.561 | 60.656 | 2 |

Rejected the candidate; patch/binary remain private. Six allocator source files
were restored byte-for-byte. Both supported Windows builds passed 39 native
tests (candidate 7.21s; restored 7.18s). Existing CMake warnings only; no compiler
or linker errors/warnings. Python was not rerun because no Python or retained
source changes resulted. The previous Python result remains 156 passed/1 skipped.

All eight routes (including shadow) completed with 15 guarded entity probes each.
The two unprofiled restoration runs FAILED pacing: rolling one-second minima
were 18 and 24 FPS. Most larger stalls in the first clustered at seconds 31-33.
The profiled follow-up's worst frame included 30.8754 ms of JIT compilation, but
it did not reproduce those larger slowdowns; their cause remains unresolved.
Do not discard the failures or attribute them solely to first-use compilation.

Plaza only, one logical CPU (2/mask 4), normal clocks, full JIT, Vulkan 3x EFB /
1920x1080 preset, muted Cubeb, formatter/identity cache enabled, reserve 524288.
No screenshots or host input. Memory probes and profiling limit timing inference;
entity counts varied, and these are not equivalent maximum-load acceptance runs.
No audible or visual correctness claim. No performance optimization shipped.
Goal UNMET. Detailed timing, entity counts, hashes and restoration results:
evidence/windows-20260930/PLAZA-REGISTER-LOOKAHEAD-REJECTED.json.


## Continued plaza combat and rejected host PGO - 2026-10-01

Three successive plaza passes in one process completed with 45 entity probes,
matching generation 13. No screenshots or host input. CPU 2 / mask 4, full JIT,
Vulkan 3x / 1920x1080 preset, normal clocks, muted Cubeb, formatter/identity cache ON,
reserve 524288. Runtime profiling and memory/timing probes make these diagnostics,
not three independent qualifying repeats.

| Pass | New FPS | P99 ms | Worst ms | >50ms | JIT compile ms | Observed living opponents |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 30.0274 | 39.149 | 53.086 | 3 | 636.078 | 4-7 |
| 2 | 29.9698 | 34.580 | 35.849 | 0 | 27.009 | 3-5 |
| 3 | 29.9699 | 34.525 | 38.491 | 0 | 49.985 | 1-4 |

Each pass covered about 52 seconds. Initial worst frame included 25.154 ms of JIT
compilation. Later passes were steadier, but enemy counts also fell: do not call
this an equivalent maximum-load comparison or attribute all improvement to
warm code. The first boundary includes slight catch-up (100.191% guest speed);
subsequent passes were approximately 100%. Zero dropped frame samples.

Tested MSVC host PGO via the existing Windows build path: /GENPROFILE:EXACT
and /FASTGENPROFILE:EXACT,COUNTER64, each with /LTCG and private databases.
Both builds passed 39 native tests (7.79s/4.92s), but both training attempts timed
out before usable gameplay: zero observed frames/probes, no collected .pgc,
failed route and runtime/harness exit 1. Elapsed 320.141s / 200.141s include shutdown
allowance. Second planned training runs did not start. No optimized USEPROFILE
candidate was built. Cause unproven; reject this experiment instead of shipping it.
Experimental source/options/tests/docs were preserved privately and removed from
the checkout; tracked CMakeLists is restored. No profiler installation or global
environment changes. Private profiles and game data remain outside Git.

Restored normal supported Windows JIT build passed: 39/39 native tests (6.91s),
Python 156 passed / 1 skipped (157 tests, 19.624s). Existing CMake warnings only;
no compiler/linker warnings/errors. PGO flags and profiler DLL dependency absent.
A fresh restored-runner plaza run exited 0 and completed 15 guarded probes with
four positive-health heroes and 6-8 identified positive-health opponents. Its
probe-instrumented timing was 29.9699 FPS, P99 34.996 ms, max 53.161 ms, one >50 ms frame;
this verifies recovery and supplies diagnostic timing, not a new FPS gain.
Runner SHA256 80cde740c3e08f72c36a1f4b46df1e672f0e5102e6388b8d76e8cab253129cb5.
Five binary hashes, failures and numeric phase evidence are recorded in
evidence/windows-20260930/PLAZA-CONTINUED-COMBAT-PGO-REJECTED.json.
Goal UNMET. First-use compilation spikes remain; sustained combat, qualifying
repeats and audio/visual limits are not resolved by this checkpoint.


## Read-only plaza entity observations - 2026-10-01

Added tools/analyze_entity_probe.py and seven synthetic regression cases. This
stdlib-only offline tool reads native memory-probe files using a private,
state-specific numeric contract. No process access, desktop input, screenshots,
game-memory writes or embedded proprietary strings/assets. Raw RAM, save states,
contracts and the diagnostic wrapper remain private under .local.

The entity-name word has an eight-bit generation tag and 24-bit identity;
searching the full word as an untagged name ID had missed the real actors.
The inspected pool has 0xa38-byte slots. Position is at +0x4c; recovery and
maximum-health normalization code supports current/recovery/maximum health at
+0x2a0/+0x2a4/+0x2a8 on the entity itself (not its attribute pointer). The tool
requires the expected generation and unique heroes, bounds all reads, rejects
non-finite classified fields, and excludes empty, stale and unclassified slots.
It does not model the whole entity lifecycle, hostile AI activity or visibility.

A live short plaza diagnostic completed: 15 probes over 51.767 guest seconds,
3-7 identified opponents with positive health, 3-4 heroes with positive health,
17 adjacent-sample hero health decreases, 29 same-slot/same-identity opponent
health decreases and 46 hero position changes. Seven same-slot/same-identity comparisons crossed
from positive to nonpositive health. These are sample deltas, NOT exact damage
or kill-event counts: same-identity slot reuse is possible. All samples remained
in generation13; maximum hero distance from the inspected plaza reference was
802.844 units, within the private wrapper's conservative1800-unit stop boundary.
One hero reached nonpositive health. Live generation-change abort was not triggered.

Start/short historical states decode to4/4 positive-health heroes and7/6
identified positive-health opponents. The old long-run endpoint uses generation14
and fails the original generation13 contract. This strengthens the reason that
it cannot count as uninterrupted combat; neither cause nor transition time is
proven. All15 live probes rechecked with the final guarded analyzer. No claim
about unsampled intervals, visual correctness or a qualifying combat repeat.

First live attempt failed in the private wrapper: read_memory has no timing
sidecar. The wrapper stopped before its first input batch; runtime shutdown
returned0, harness1. The corrected route explicitly uses read_timing and exited0.
Do not conceal or count the failed attempt as validation.

Unchanged native runner SHA256:
2f5cdc56e309d8e3514d5697a338654affa2151ed24695b318e9a271fd5bedd0.
One logical CPU2/mask4, full JIT, Vulkan3x/1920x1080 preset, normal clocks, muted
Cubeb, formatter and identity cache ON, reserve524288. Process-local input only;
no screenshots. Memory/timing reads perturb execution, so this is not a new FPS
comparison. Native code unchanged: prior Windows build/39-test result retained.
Final Python suite:156 passed,1 skipped (157 tests,22.251s), including7 entity
cases. Evidence: evidence/windows-20260930/PLAZA-ENTITY-PROBES.json.

Goal UNMET. This improves combat-state evidence, not frame rate. Remaining work
includes compile bursts/other slow frames, three qualifying repeats, ten-minute
verified plaza combat and audible quality. Retain the plaza-only/no-screenshot
constraint and do not count idle, stale, reloaded or failed-route time as combat.


## Plaza JIT metadata reservation experiment - 2026-10-01

Testing remains tutorial-plaza ONLY, with NO screenshots, host input or desktop
capture. These instructions supersede older visual-capture/varied-scene plans.
A completed input route and rendered frames do not prove continuous combat or
correct visuals. No new visual or audible correctness claim is made.

An instrumented reference found first-use JIT bursts in the slowest frames:
1,158 new blocks occupied 41.363ms of a 70.076ms frame; metadata insertion
accounted for 20.076ms inside that compile time. A separate intrusive resident
census found 36,404 executed blocks, only 1,068 executed once. Most matched
burst addresses were reused. This weakens the case for a first-visit-only
interpreter tier; no tier or speculative JIT prewarming was implemented.
Cross-run address matches are not deterministic paired execution, and invalidated
blocks are absent. Save-state WRITE does not clear this JIT; state READ does.

Added nested jit_backpatch_rehash timing and a default-off host bucket-capacity
hint: OPENMUA2_BACKPATCH_RESERVE=524288. Strict decimal values 0..1048576 are
accepted; 0 retains normal growth, and 1 aliases the initial 131072 experiment.
Invalid values retain ordinary growth. Reservations never shrink existing
capacity and preserve value references. No guest code/data is precompiled or
changed. Native tests cover pointer stability, capacity retention/rounding,
reuse, parser bounds/rejection; Python verifies nested span accounting.

Initial 131072 reservation FAILED to remove growth: 64 rehashes in both runs,
40.452ms OFF versus 37.280ms ON; worst frame 66.806 versus 88.958ms. This is a
failed trial, not evidence of a gain. The bounded revision logs actual buckets:
524288 produced 8192 buckets per shard, retained over subsequent clears.
Same-binary profiled comparison: OFF had 64 rehashes/44.968ms, ON had zero in the
measurement window. Slow metadata insertion fell from 46.945 to 2.654ms;
compile elapsed coverage 435.238 to 423.277ms (different block counts).
Worst frame 60.979 to 56.235ms. These nested spans overlap; do not add them.
Backpatch spans measure metadata insertion during compilation, not runtime faults.

Repeated comparisons WITHOUT detailed runtime profiling, all exit0:

| Order | Capacity hint | New FPS | P99 ms | Worst ms | Frames >50ms |
| --- | --- | --- | --- | --- | --- |
| 1 | OFF | 29.9706 | 35.233 | 62.933 | 1 |
| 2 | 524288 | 29.9701 | 35.283 | 51.450 | 1 |
| 3 | OFF | 29.9700 | 35.452 | 58.445 | 2 |
| 4 | 524288 | 29.9701 | 35.023 | 57.804 | 2 |

Full JIT, whole process CPU2/mask4, Vulkan3x/1920x1080 preset, normal clocks,
muted Cubeb with audio profiling, formatter ON and identity cache ON; indirect
experiments OFF. Existing short queued plaza route, two-second warmup, endpoint
before save. No screenshots. Mixed/modest frame-tail benefit; keep opt-in.
No tens-of-FPS gain or solid-30 claim. Callback counters cannot prove clean audio.

Supported Build.cmd --cpu jit --jobs 2 completed successfully with local LTO ON.
39/39 native tests passed (7.21s); Python 149 passed/1 skipped (34.877s).
Existing CMake deprecation, Wayland, object-path-length and lz4 IPO warnings;
no compiler/linker diagnostics. Five runtime executables and hashes are recorded
in evidence/windows-20260930/PLAZA-BACKPATCH-RESERVE.json. Runner SHA256:
2f5cdc56e309d8e3514d5697a338654affa2151ed24695b318e9a271fd5bedd0.
Private source data, saves, profiles and game assets remain outside Git.

Goal UNMET: >50ms frames persist; continuous combat, audio quality, three
qualifying repeats and ten-minute combat remain unverified. Investigate remaining
first-use compile bursts and establish read-only combat-state telemetry before
another long run; do not treat idle/menu/failure time as combat validation.


## Live byte-identity cache for formatter guards - 2026-10-01

Added default-off OPENMUA2_FORMAT_IDENTITY_CACHE=1 inside the existing formatter
experiment. Captured formatter-body/caller-epilogue bytes must pass the existing
SHA-1 identities, then every eligible invocation compares all live bytes against
those verified copies. Mapping, layout, ABI and fallback guards remain active;
mutations reject, including after state loads/JIT clears. No unchecked identity
result or guest pointer is cached. Captured game bytes stay in process memory.
Native coverage checks initial failure/retry, every-byte mutation/restoration,
wrong lengths and a live-source change during captured-copy validation.

Supported Windows JIT build passed with local LTO ON: 39/39 native tests (5.64s).
Existing CMake deprecation/Wayland/path-length/lz4 IPO warnings; no compiler or
linker diagnostics. Python tooling unchanged, not rerun. The initial edit script
hit a newline assertion; its accidentally started preliminary build was cancelled
and is not validation evidence. The completed build uses the full reviewed patch.
Runner SHA256 fa202fa923e7d1fcc48a5c9208f385b7638889fcbbcdaeb0df4aaa67001231a3.

Shadow: 406,667 completed comparisons, zero output/FPSCR/ABI errors, one pending
at shutdown; two hashes and813,336 live byte checks. This is scoped verification,
not full-game correctness. Same-binary alternating plaza diagnostics, all exit0:

| Order | Cache | New FPS | P99 ms | Worst ms | Frames >50ms |
| --- | --- | --- | --- | --- | --- |
| 1 | OFF | 29.7073 | 44.118 | 119.495 | 9 |
| 2 | ON | 29.9703 | 35.121 | 57.491 | 2 |
| 3 | OFF | 29.9700 | 36.288 | 62.311 | 2 |
| 4 | ON | 29.9701 | 35.340 | 56.423 | 1 |

All: whole process CPU2/mask4, JIT, Vulkan3x/1920x1080 preset, normal clocks,
muted Cubeb, formatter ON, indirect experiments OFF, no screenshots/memory probes
or detailed runtime profiling. Two-second warmup, endpoint before state save.
ON runs used two hashes each plus4,045,618/4,096,354 byte checks; OFF runs used
4,260,396/3,830,478 hashes. CPU-time counters unavailable: no measured CPU-time
saving is claimed. Both pairs favor ON, but the second gain is modest and >50ms
frames persist. Keep opt-in; no tens-of-FPS or solid-30 acceptance claim.
Combat continuity, audio quality, three qualifying repeats and ten-minute combat
remain unverified. The prior long-run transition/stall remains unresolved. Goal
UNMET. Evidence: evidence/windows-20260930/PLAZA-FORMATTER-IDENTITY-CACHE.json.


## Read-only plaza transition probes - 2026-10-01

Ran two private diagnostics with the existing runner, one logical CPU2, full JIT,
Vulkan3x/1920x1080 preset, normal clocks, muted Cubeb and formatter experiment ON.
No screenshots, host input or game-memory writes. The existing native read-memory
command samples the previously identified dialog slot; private guards reject its
failure/retry option group before publishing further input. Offline fixtures
accept baseline/short state, reject the known long-endpoint failure signature,
and reject a truncated probe (four checks). Live transition abort is unexercised.

Sequential run: exit0, 41 probes over212.296 observed guest seconds (180.190s
commanded). Batched run: exit0, 86 probes over394.388 observed guest seconds
(380.940s commanded). Both keep the baseline slot hash throughout. Neither
reproduces the prior retry signature or1.234567s guest presentation gap. Largest
guest interval33.367ms; host gaps still reach83.119/86.268ms respectively. These
are intrusive diagnostics, not acceptance or a performance improvement. Command
delivery differs from the original endurance run; no prior failure is dismissed.

The guard detects one failure signature, not enemy activity, health or every
transition. Establish actual combat continuity before another endurance acceptance
attempt, then return to measured CPU/JIT stall optimization. No fresh visual or
audible verification. No native/tooling source change or rebuild; prior build and
tests retain their recorded scope. Proprietary probe files/helpers remain private.
Evidence: evidence/windows-20260930/PLAZA-TRANSITION-PROBES.json. Goal UNMET.


## Offline plaza state audit - 2026-10-01

Investigated existing baseline, short CPU2 endpoint and long endurance endpoint
without launching the game or taking screenshots. Bounded private LZ4 decoding,
three executable anchors, the native memory serialization layout and the dialog
manager accessor/vtable identify the same dialog slot in all three states.
The long endpoint contains a newly populated failure/retry option/callback group;
the baseline and short endpoint retain their prior options. This supports a
failure/retry-path inference during the long run. Its late near-30 cadence must
not count as verified continuous combat. The slot is not the selected dialog at
the endpoints: populated historical content does not prove a currently visible
menu or timestamp/attribute the 1,220ms gap. Short-run combat continuity also
remains unverified. No fresh visual, audible, build or performance claim.

Next establish read-only health/activity or transition telemetry and stop plaza
diagnostics on failure/transition before another endurance acceptance attempt.
Keep plaza only, no screenshots, one logical CPU and existing rendering settings.
No native/tooling source changed; prior build/tests remain scoped as recorded.
Private game states, decompressed memory, strings and instructions stay outside
Git. Numeric evidence: evidence/windows-20260930/PLAZA-STATE-AUDIT.json.
Goal remains UNMET.


## Processor placement and long-run diagnostic - 2026-10-01

Added --logical-processor N to tools/run_combat_benchmark.py (Windows, current
processor group). The child inherits exactly one allowed logical CPU; the harness
is immediately restored. The runner still pins/verifies its own process, and the
benchmark now checks the requested mask, not only its bit count. No global policy,
unrelated process, native source/binary or normal launcher default changed.
Seven new tests cover setup/launch/restore failures, cleanup, allowed masks and
runtime confirmation. Tooling: 149 run, 148 passed, 1 skipped (19.762s). New option
also passed a real CPU2 plaza run. Prior native Windows build 39/39 still applies;
runner remains c5961c5d32ebeb99be9492bea5ea621757af405e0da0c754cfa0e2f4377c1311.

All runs: whole game process on one logical CPU, JIT, Vulkan3x, normal speed/clocks,
Cubeb muted, formatter experiment ON, indirect hints OFF, no screenshots. CPU
numbers below are logical processor indices, not claims about physical topology.

| Run | CPU | P99 ms | Worst ms | Frames >50ms | New FPS |
| --- | --- | --- | --- | --- | --- |
| Preliminary (harness also pinned) | 2 | 36.053 | 57.195 | 3 | 29.9702 |
| Restored-harness run | 2 | 37.519 | 49.970 | 0 | 29.9702 |
| Comparison | 4 | 36.858 | 57.384 | 2 | 29.9701 |
| Control | 0 | 45.292 | 61.772 | 6 | 29.9704 |
| Repeat 2 | 2 | 35.363 | 60.374 | 1 | 29.9698 |
| New CLI option validation | 2 | 36.471 | 60.152 | 1 | 29.9702 |

CPU2 improves the observed tail consistently enough for further diagnostics;
it is not proven universally preferable. Normal launcher stays unchanged.

A longer CPU2 run repeated the existing plaza input cycle14 times, omitting the
initial approach after cycle1; 1,458 timed holds / 628.69 commanded guest seconds.
Runtime/harness exit0. Measured after two seconds: 643.055s, 29.8948 new FPS,
P99 34.270ms, 17 frames >50ms, maximum1,220.296ms, lowest rolling second0;
no samples dropped. At138.405s a 1,220.296ms host gap coincides with1,234.567ms
guest presentation gap. At349.960s a151.890ms host gap has normal33.367ms guest
cadence. Preserve both; do not discard unverified transitions as loading.
Minutes2,4,5,7-11 have no >50ms intervals; that is NOT proof of continued combat.
Hero health, enemy activity and scene state were not observed. The long run is
NOT ten-minute combat acceptance and no clean-audio claim is made.

Next investigate the guest-side gap/possible transition and establish non-visual
activity validation before interpreting stable late cadence as combat. Keep CPU2
as an explicit diagnostic option, plaza only, no screenshots. Goal UNMET: verified
continuous combat, frame pacing and audible quality remain outstanding.
Evidence: evidence/windows-20260930/PLAZA-PROCESSOR-ENDURANCE.json.



## Guarded direct-call experiment: mixed pacing results - 2026-10-01

Added default-off OPENMUA2_INDIRECT_HINTS=origin:target1,target2,target3 for an
explicit unconditional linked bcctr call site. Runtime CTR comparisons guard
ordinary WriteExit direct-call links; mismatches retain original dispatch.
Debugging/static fallback and disabled linking exclude the experiment. No guest
addresses are baked into source defaults. Existing link management preserves
feature-context matching, downcount, BLR return prediction and SMC unlinking;
this reuse is not an exhaustive new generated-code/SMC correctness test.
Native tests cover bounded parsing, duplicate/invalid/unaligned inputs and limits.

Supported Windows JIT build passed: local LTO ON, 39/39 native tests (7.68s).
Existing CMake warnings remain; no compiler/linker diagnostics found. Python
unchanged, not rerun (prior 141 passed/1 skipped remains scoped to earlier run).
Runner SHA256 c5961c5d32ebeb99be9492bea5ea621757af405e0da0c754cfa0e2f4377c1311.
Other produced runtime executable sizes/hashes are in the evidence JSON.

Same-binary alternating plaza runs, all exit0, one logical CPU, Vulkan3x, normal
clocks/speed, Cubeb muted, formatter ON, rush/smooth OFF, no screenshots or
intrusive target counters. Two-second warmup; final input endpoint before save.

| Order | Mode | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Hints ON | 29.9742 | 42.497 | 61.597 | 2 | 29 |
| 2 | Hints OFF | 29.9702 | 40.687 | 57.089 | 5 | 29 |
| 3 | Hints ON | 29.9704 | 39.595 | 74.818 | 3 | 29 |
| 4 | Hints OFF | 29.9701 | 43.396 | 57.638 | 5 | 29 |

Decision: do not promote. Fewer >50ms frames came with worse maximum stalls in
both comparisons; no substantial/repeatable solid-30 gain is established. Keep
this narrowly scoped experiment default OFF. Do not add more speculative targets
without evidence. First-use compilation and non-JIT stalls remain the priorities.
No fresh visual/audible verification or clean-sound claim. Goal UNMET: three
qualifying repeats and ten-minute plaza combat acceptance remain outstanding.
Keep plaza only and no screenshots. Evidence:
evidence/windows-20260930/PLAZA-GUARDED-INDIRECT-CALLS.json.



## Selected indirect-call census - 2026-10-01

Added default-off OPENMUA2_INDIRECT_PROFILE_PC diagnostic: one aligned hexadecimal
bcctr instruction address, 64 destination/feature-flag counters, explicit total
and overflow, flushed after CPU shutdown. Disabled emits no runtime counter call.
Both taken branch forms retain ordinary dispatch and timing; guest register caches
are flushed and the destination scratch register is saved around the helper.
Selector validation, distinct translation contexts, overflow and reconciliation
are covered by native tests. Counts cover the whole process, including boot;
they are intrusive diagnostics, not an FPS benchmark or correctness proof.

Supported Windows JIT build passed, local LTO ON: 39/39 native tests in 6.21s.
Existing CMake warnings remain; no compiler/linker diagnostics found. Python
tooling unchanged and not rerun (previous 141 passed/1 skipped remains scoped).
Runner SHA256 d2e0314de0d66725d60608b6b224cbea5ffb73e82fb57675d9769e027282b849.
Runtime binary sizes/hashes are recorded in the evidence JSON.

Selected 8036f9b4 in the plaza route: 114,958,803 calls, seven destinations, all
feature flags3, zero overflow, exact count reconciliation; runtime/harness exit0.
Three busiest targets each receive about26% of calls (combined78%). Private
instruction inspection identifies short float-vertex-attribute graphics FIFO
writers. No proprietary code/data is included in the evidence or source.
This rules against a simple one-target predictor; it does not establish the
benefit of a multi-target cache over the existing direct entry-point dispatcher.
Next test a guarded polymorphic dispatch optimization only with preserved
feature flags/downcount/return prediction/cache invalidation and measured gains.
Separately address first-use JIT compilation bursts; avoid guest-cycle hacks.

Counter-disabled follow-up also exited0 and emitted no profile summary. After
two seconds: 46.581s, 29.9696 new FPS, P99 43.534ms, maximum101.092ms, five frames
>50ms, lowest rolling second28, guest speed99.9986%. No performance gain claimed.
Both runs: one logical CPU, Vulkan3x, normal speed/clocks, Cubeb muted, formatter
experiment ON, rush/smooth OFF, no screenshots. Audio crackle and fresh visual
correctness remain unverified. Goal UNMET; three qualifying repeats and ten-minute
plaza combat acceptance are still outstanding. Preserve plaza-only/no-capture.
Evidence: evidence/windows-20260930/PLAZA-INDIRECT-TARGETS.json.



## Plaza pacing hypothesis rejected; refreshed hot-block profile - 2026-10-01

Tested existing RushFramePresentation=True plus SmoothEarlyPresentation=True in
an isolated copied profile. Both no-screenshot plaza runs exited 0, with full
JIT, one logical CPU for the whole process, Vulkan3x, normal clocks/speed, Cubeb
muted and formatter explicitly ON. No source, binary or normal-profile changes.
Current Windows build/tests from the previous checkpoint still apply.

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Rush/smooth 1 | 46.708 | 29.973 | 45.738 | 63.824 | 6 | 29 |
| Rush/smooth 2 | 46.625 | 29.984 | 49.667 | 90.571 | 13 | 28 |

Throttle calls fell from 17,122 in the prior trace to about one per rendered
frame. Positive-deadline wait overshoot maxima were 2.108/8.715ms versus prior
2.687ms. The first run's worst frame still included 38.709ms compilation.
Fewer waits do not give a repeatable frame-pacing fix; do not promote the pair.
Interior audio buckets had DMA/stream empty-dequeue counts 1/0 and 2/3; these
muted diagnostics do not prove clean audible playback.

An additional intrusive resident-JIT-block profile completed in 217.125s,
exit0, 36,141 resident blocks. Pages 803f6000, 8036f000 and 8028e000 account for
11.70%, 11.24% and 10.37% of instrumented block time. These are not reliable
release CPU-time shares: short blocks incur disproportionate profiler overhead
and invalidated blocks are omitted. Private code inspection identifies an
indirect-call loop at 8036f998, executed 101,824,758 times in this profile;
its JIT path currently uses WriteExitDestInRSCRATCH. This is a lead, not proof
that dispatch is the next largest bottleneck. No proprietary code/data included.

Next characterize that loop's indirect targets and measure dispatch overhead;
a proposed call-site cache must preserve feature flags, checked-entry/downcount,
return prediction, SMC invalidation and fallback. Do not bypass guest scheduling
or speculatively invoke arbitrary Jit calls. First-use compilation remains a
separate hitch source. Goal UNMET: no three qualifying repeats or ten-minute
plaza combat acceptance, no fresh visual/audible verification. Keep plaza only
and no screenshots. Evidence: evidence/windows-20260930/PLAZA-PACING-HOTBLOCKS.json.



## Bounded formatter spill experiment - 2026-10-01

Plaza-only, no screenshots. Added guarded EABI stack-overflow argument reads and
bare/8-digit hexadecimal formatting to the existing DEFAULT-OFF formatter
experiment. Unsupported formats still fall back; original code/caller hashes,
float restrictions and transactional output protections remain. This avoids
some formatting work without changing clocks, effects or the one-CPU constraint.

Supported Windows JIT build succeeded with local LTO enabled: all 39 native tests
passed (8.20s); Python tooling 141 passed, 1 skipped (50.301s). Existing CMake
warnings cover deprecations, unavailable Wayland, long object paths and lz4 IPO
policy; no compiler/linker errors. Current moderngekko-run.exe is 16,064,512 bytes,
SHA256 7a4e0b314737c8ab9bf32673787ce91c8a7887323fb9efa0f0ca7175653582db.
Other runtime executable hashes/sizes are recorded in the evidence JSON.

Shadow run: 1,803,814 completed comparisons, zero output/FPSCR/preserved-ABI
mismatches, one pending at shutdown (unverified). This is scoped evidence, not
universal correctness. Three replacement-mode repeats and one trace exited 0:

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Spill 1 | 46.682 | 29.969 | 43.093 | 73.992 | 7 | 29 |
| Spill 2 | 46.947 | 29.970 | 43.580 | 115.656 | 4 | 28 |
| Spill 3 | 46.781 | 29.970 | 49.490 | 69.252 | 12 | 29 |
| Spill spans | 46.614 | 29.969 | 48.771 | 67.529 | 10 | 29 |

Same intact-statue plaza state/route, Vulkan3x, whole-process affinity mask1,
normal clocks/speed, Cubeb Volume0, formatter explicitly ON; two-second warmup.
These are short diagnostic runs, not qualifying acceptance runs. Earlier repeats
had 11 frames >50ms each; improvement is inconsistent and the worst new stall
is larger. The old binary control failed twice before combat with 0x80000003
(KERNELBASE breakpoint); cause unresolved. Preserve those failures. Historical
runs therefore are not a contemporaneous controlled A/B result.

Trace: 13,295 JIT compilations / 501.490ms versus prior 14,295 / 591.331ms;
measured throttle time 12.985s versus 9.088s. These observations suggest reduced
work but do not establish a causal FPS gain. The worst new trace frame includes
39.052ms JIT compilation; another 63.800ms frame includes none. Nested spans
must not be summed. First-use compilation and non-compilation stalls remain.

No fresh visual or audible validation; muted audio telemetry cannot establish
whether crackling is fixed. No further screenshots. Goal remains UNMET: robust
frame pacing and ten-minute plaza combat acceptance are outstanding. Next isolate
remaining JIT bursts and non-JIT stalls, retaining plaza-only/no-capture scope.
Evidence: evidence/windows-20260930/PLAZA-FORMATTER-SPILL.json.



## Plaza timing repeats and hitch attribution - 2026-10-01

Current runner 2824e94c0a9b4bd9c0c18110eccaf272e5f06f5c14b10534eff6ce9d48f0367d,
JIT, whole-process mask1, Vulkan 3x EFB, normal clocks/speed, Cubeb muted,
formatter experiment ON (normal launch default OFF), no screenshots. Same
xbox-timed-hero-combat route and statue-target private state; two-second warmup,
last input endpoint before save. These are short diagnostic repeats, not passes.

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Timing 1 | 46.621 | 29.987 | 48.517 | 101.724 | 11 | 27 |
| Timing 2 | 46.683 | 29.968 | 47.470 | 94.454 | 11 | 28 |
| Timing 3 | 46.847 | 29.970 | 46.411 | 72.992 | 11 | 28 |
| Runtime spans | 46.649 | 29.990 | 50.143 | 83.185 | 14 | 28 |

All four command runs exit0; guest speed 100.00-100.07%. The profiled worst
frame includes 49.624ms JIT compilation, 32.620ms emission, 11.325ms finalization
(nested, do not sum), and 0.0037ms GPU fence wait. Across the window 14,295 JIT
compilations cost 591.331ms elapsed. Other 60-61ms frames had zero/negligible
compilation; there are multiple causes. CPU counters remain quantized and
cannot prove frequency or scheduling causes. Next investigate first-use compile
bursts and non-compilation stalls separately in the plaza. Do not precompile
arbitrary blocks if that changes guest architectural state or cache semantics.

No source/binary changes or rebuild this checkpoint. Prior supported Windows
build 39/39 native, tooling 141 passed/1 skip still applies. No fresh visual or
audible verification; numeric audio counts are recorded, not proof of clean sound.
No further screenshots after user instruction. Earlier street exploration is
retained privately; its paused harness timed out at 600s (child 0/harness 1).
Goal unmet: frame pacing and ten-minute plaza combat remain outstanding.
Evidence: evidence/windows-20260930/PLAZA-ONLY-BASELINE.json.


## Primary benchmark scope - user direction, 2026-10-01

Use the tutorial plaza only for performance and bottleneck testing. The user
identifies it as representative of the game's maximum simultaneous enemy load,
including effects, sounds and animations. Keep the statue intact in the dense
combat baseline so its scripted completion does not remove the repeated spawns.
Later streets are not required benchmark work. Three repeat runs and ten minutes
of plaza combat with varied movement, heroes, attacks and effects remain required;
the nominal-30 cadence and correctness requirements are unchanged.

The user has requested no more screenshots. Disable capture commands for all
future runs (--no-screenshots); use frame timing, CPU/JIT and audio telemetry.
Previously inspected native evidence remains scoped to those earlier runs.
Do not imply fresh visual verification from telemetry. The latest visual plaza
run completed before this instruction, but its ten captures were not inspected;
no new visual-correctness claim is made from it. Audible playback remains
unverified. These user instructions supersede earlier route-expansion and new
screenshot requirements in this document.


## Xbox Safeguard gestures and route validation - 2026-10-01

Generated Xbox profile v3 adds Back/View + right-stick sideways for shake and
Back/View + right-stick up/down for swing. Camera rotation is suppressed while
Back/View is held; LB+RB fusion aiming suppresses these gestures. Existing user
profiles remain preserved. Real expression-parser tests cover these mappings
and separation from camera/fusion controls. Downward swing is not game-verified.

Healthy Iron Man reached the statue prompt by flying onto its pedestal. LB
starts the interaction. The complete generated-v3 route toppled the statue in
two runs, with native captures showing both prompts, the fallen statue, and
Fury's completion response. All six captures were inspected sequentially.
Use sideways input until the upward prompt, then promptly move the stick up.
The route uses 1500ms sideways and 800ms upward; 500ms sideways was too short,
while 2500ms followed by upward failed. A neutral delay and repeated upward
pulses after the long shake also failed. Midpoint restore tests alone were
insufficient; the final route completes without a midpoint reload.

Build.cmd --cpu jit --jobs 2 passed, 39/39 native tests (7.06s). Tooling:
141 passed, one skipped (59.553s). First build ran the old Xbox expectation
and failed 1/39; corrected final rebuild passed. Existing CMake warnings:
deprecation, missing Wayland, long object paths, lz4 CMP0069 IPO policy.
No compiler/linker error in final build. Produced moderngekko-run.exe,
ModernGekko.exe, moderngekko-port.exe and moderngekko-module-info.exe;
sizes and SHA-256 hashes are in the evidence JSON. Local build uses LTO ON.

The earlier paused exploration hit its 900s harness watchdog; child exit0,
harness exit1, and its incomplete route remains failed. Private screenshots,
saves and extracted mission data stay outside Git. No host input/capture used.
These are interaction diagnostics: JIT, one logical CPU, Vulkan 3x EFB, normal
speed, formatter experiment ON (normal default OFF), Cubeb muted. Audio was
not heard; screenshots do not prove continuous animation or FPS acceptance.
Three qualifying repeats and ten-minute varied combat remain outstanding.
Next extend combat beyond the statue using the verified endpoint in private
.local/automation/safeguard-v3-repeat-20261001/statue-completed.sav.
Goal unmet. Evidence: evidence/windows-20260930/XBOX-SAFEGUARD-VALIDATION.json.
Route: tools/routes/xbox-statue-safeguard.json.


## Combat-route interaction investigation - 2026-10-01

Three private route probes completed with exit0; all ten native screenshots
were inspected sequentially. Ground approach, jumps and attempted flight did
not activate the statue interaction. Two probes include Iron Man knockout;
these low-health endpoints must not become acceptance starting states.
Read-only inspection of the private mission package confirms a scripted statue
interaction whose completion stops repeating enemy spawns. A raised trigger
region is a navigation lead, not a verified input fix. Next restore an earlier
healthy state and establish the pedestal approach/use activation before changing
motion mappings. Extracted mission data and captures remain private under .local.

No source, binary or normal profile setting changed. No new FPS acceptance or
audible playback claim; Cubeb ran muted, formatter experiment ON (default OFF).
Prior build/tests remain 39/39 native and 141 tooling passed, one skipped; not
rerun for this evidence-only checkpoint. Three qualifying repeats and ten-minute
varied combat remain required. Goal unmet. Evidence:
evidence/windows-20260930/STATUE-ROUTE-EXPLORATION.json.


September 29, 2026. This file tracks the current reconstructed source on `main`; the older LOCAL01 recovery boundary is no longer an accurate description of the checked-in implementation.

## Audio buffer diagnostic, normal settings unchanged - 2026-10-01

Compared isolated copied profiles with Core.AudioBufferSize 160/80/160ms, in
that order. Existing mixer default is 80ms; Cubeb requests max(512 frames,
device minimum latency). This does not establish actual device output latency.
The mixer can replay older granules when starved, so fewer empty dequeues alone
does not prove clean playback. Increasing the queue may add audio latency.

Same verified Windows runner, JIT, one logical CPU, Vulkan 3x EFB, normal speed,
formatter experiment ON (normal launch default OFF), Cubeb muted; frame/audio
telemetry without detailed runtime profiling or screenshots. Same extended
hero route; two-second warmup and endpoint before state save:

| Queue limit ms | Measured s | FPS | Worst frame ms | Lowest rolling second | DMA/stream empties |
| --- | --- | --- | --- | --- | --- |
| 160 first | 46.6464 | 29.9702 | 62.3037 | 28 | 0/0 |
| 80 control | 46.9289 | 29.9176 | 114.7865 | 25 | 9/8 |
| 160 repeat | 46.7130 | 29.9702 | 84.9601 | 28 | 0/1 |

Empty counts cover 45/46/46 complete interior seconds respectively. All runs
exit0. Control also suffered worse frame stalls, so this is not a clean causal
comparison or a demonstrated frame-rate gain. 160ms did not eliminate starvation.
Normal launch stays 80ms; retain candidate only in private test profiles. Audio
still needs real playback verification; no new visual verification occurred.
A post-run read-only check found Balanced power plan; the reported 3801MHz WMI
snapshot cannot prove runtime frequency or throttling. No system setting changed.

No source/binary change or rebuild; existing Windows validation remains 39/39
native tests and 141 tooling passed/one skipped. User static edits/assets/saves
preserved. Numeric evidence: evidence/windows-20260930/AUDIO-BUFFER-EXPERIMENT.json.
Next broaden the combat route and correlate audio starvation with stalls across
varied gameplay, rather than treating short saved-state runs as acceptance.
Three qualifying repeats and ten-minute varied combat remain required. Goal unmet.

## Frame-boundary CPU counters and repeated hitch diagnostics - 2026-10-01

Added opt-in callback-thread/process cumulative CPU time, thread cycles and
thread identity at newly rendered frame boundaries. Runtime profiling enables
the OS queries; ordinary frame traces retain unavailable (-1) fields. Analyzer
reports whole-window and per-frame deltas, rejects unavailable/regressing or
changed-thread readings, and does not convert cycles into wall time. Windows
CPU times in these runs advance in 15.625ms increments; sequential queries are
not atomic and cannot precisely separate scheduling from execution per frame.

Build.cmd --cpu jit --jobs 2 initially failed a Windows min macro collision;
fixed NOMINMAX in the new header, then the supported build passed 39/39 native
tests in 8.87s. Final tooling: 142 run, 141 passed, one skipped in 19.929s.
No compiler/linker warning or error in final build; existing CMake warnings
remain. Local LTO ON, source default OFF. User static changes remain untouched.

Same extended hero route, JIT, one logical CPU (mask1), Vulkan, 3x EFB, normal
speed, Cubeb muted, formatter experiment ON (normal launch default remains OFF).
Two-second warmup, endpoint before save; frame and audio telemetry remain on:

| Run | Seconds | New FPS | P99 ms | Worst ms | >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Detailed profile 1 | 46.9803 | 29.9700 | 38.2873 | 66.4966 | 2 | 29 |
| Detailed profile 2 | 47.0796 | 29.9705 | 43.4118 | 96.9200 | 9 | 28 |
| Detailed profile 3 | 46.8468 | 29.9701 | 43.3577 | 80.2466 | 5 | 28 |
| No detailed runtime profile | 46.5130 | 29.9701 | 41.7470 | 62.7387 | 3 | 29 |

All completed successfully, near 100% guest speed. Average 29.97 is nominal 30;
remaining issue is intermittent hitches, not a sustained 17-18 FPS collapse in
this route. These are not three qualifying acceptance runs. Repeat2 frame14642
has 96.92ms host/33.3667ms guest, no JIT compilation, 88.53 million thread cycles
versus 97.34 million in the preceding 33.90ms frame, and 62.5ms quantized CPU time.
It is not explained by extra instruction cycles alone; scheduling/frequency and
runtime costs are not individually resolved. Other long frames contain large
compilation bursts. Hitches remain without detailed runtime instrumentation;
one comparison does not quantify profiler overhead or establish a host cause.

Interior DMA/stream empty dequeues: 0/2, 1/3, 1/1 and 1/1 respectively. No new
native visual captures or audible/device verification; prior route snapshots
do not prove current continuous visual behavior. Crackling remains unresolved.
Next narrow non-compilation host/runtime stalls and separately address cold
compilation bursts; preserve normal-speed/correctness constraints. Three
qualifying repeats and ten-minute varied combat remain outstanding. Goal active.
Evidence/binary hashes: evidence/windows-20260930/FRAME-CPU-COUNTERS.json.

## CPU graphics dispatch tracing and affinity diagnostic - 2026-10-01

Added opt-in RunGpuOnCpu timing for dispatches >=100us and analyzer coverage
regression. Short calls are omitted: recorded coverage is a lower bound, includes
nested rendering/waits/preemption, and cannot rule out total decoding cost.
Supported Windows build passed 39/39 native tests (12.69s); tooling passed
139 tests with one skipped (140 run, 50.009s). Existing CMake warnings remain;
no compiler/linker warning or error found. Local LTO ON; source default OFF.

Extended guest-timed route, JIT, one logical CPU, Vulkan, 3x EFB, experimental
formatter ON (default OFF), Cubeb muted, runtime/audio profiles, no captures:
CPU0 after two-second warmup: 46.9778s, 29.9716 new FPS, 100.0053% guest speed,
P99 40.5253ms, worst 67.0546ms, five >50ms, minimum rolling second 29.
A 52.3424ms frame has no compilation and only 1.489ms recorded slow dispatch;
a 50.0322ms frame has no compilation and 3.6289ms recorded slow dispatch.
This leaves time unexplained; it does not establish CPU execution or a GPU cause.

A diagnostic parent constrained the child to logical CPU4 (verified mask 0x10):
47.1471s, 29.9700 new FPS, 100.0000% guest speed, P99 38.3621ms, worst 67.5393ms,
four >50ms, minimum rolling second 29. Later hero cycles still reach 61.8698ms.
This single non-deterministic comparison is not a demonstrated fix; production
affinity stays unchanged. Investigate compilation bursts and non-compilation
stalls separately, with total CPU execution/scheduling coverage next.

Separate CPU0 visual run completed; all ten native captures inspected in order.
Street, team, enemies, HUD and effects visible; RT cycles through all four heroes.
Final Spider-Man position is near a building without an adjacent enemy, so this
is not proof of uninterrupted varied combat or continuous animation correctness.
No audible playback verification; audio crackling remains unresolved. See
numeric evidence and binary hashes in evidence/windows-20260930/GPU-DECODE-AND-AFFINITY.json.
No proprietary output committed. Three qualifying repeats and ten-minute varied
combat remain outstanding. Goal active/unmet; no new FPS improvement claimed.

## Extended hero combat and recurring stalls - 2026-10-01

Added tools/routes/xbox-timed-hero-combat.json: 106 timed holds totaling 47.46
guest seconds, extending the visible route with RT hero cycling, movement,
light/heavy attacks, powers and block inputs. Visual, separate timing/profile,
and formatter-shadow runs all exited 0. All ten visual captures inspected in
order: active control cycles Spider-Man -> Captain America -> Iron Man ->
Wolverine -> Spider-Man amid enemies, hits/projectiles, team, street and HUD.
Spider-Man's HUD is red in the Iron Man capture; do not claim every hero stayed
healthy. These snapshots do not validate every intermediate frame/animation.

Same JIT runner, one logical CPU, Vulkan, 3x EFB, Cubeb muted. Formatter ON
for visual/timing runs, shadow for comparison; default remains OFF. No runtime
changes or rebuild; prior supported Windows build passed 39/39 native tests.
No-capture runtime/audio-profile diagnostic after two seconds: 46.7131s,
29.9702 newly rendered FPS, guest speed 100.0005%, P99 40.9289ms, worst 67.8137ms,
five frames >50ms, lowest rolling second 29. Later hero cycles alone: 35.8325s,
29.9728 FPS, P99 39.2988ms, worst 53.2186ms, two frames >50ms. This is not acceptance.
One late 53.2186ms frame has no JIT compilation, 6.0611ms traced waits; another
51.8684ms frame has 17.0198ms compilation. A separate stall remains unexplained.
Next instrument CPU-side graphics FIFO decoding (RunGpuOnCpu), absent from the
current wait timers, before calling unclassified time guest CPU execution.

Shadow completed 1,845,049 comparisons, 710,670 floating calls, zero output/FPSCR/
preserved-register mismatches and abandoned calls; one pending at shutdown is
unverified. This broadens comparison coverage but does not prove full timing or
world-state equivalence. No audio listening/device verification; crackling is
unresolved. Three qualifying repeats and ten-minute varied combat still needed.
Evidence: evidence/windows-20260930/EXTENDED-HERO-COMBAT.json. Game captures,
states and logs remain private. Goal active/unmet; no new FPS gain claimed.

## JIT metadata allocation pool rejected - 2026-09-30

Tested a small-node pool shared by the 64 backpatch metadata hash tables.
The supported Windows candidate build passed 39/39 native tests in 5.60s,
including temporary value-lifetime, clear/reuse and allocation-release checks.
Tooling: 139 run, 138 passed, one skipped in 53.784s.
Two candidate diagnostics did not demonstrate better compilation or frame tails:

| Run | Compilations | Compile elapsed ms | Worst frame ms | Lowest rolling second |
| --- | --- | --- | --- | --- |
| Original prior | 7431 | 291.2170 | 75.1227 | 29 |
| Pooled first | 7428 | 328.2433 | 89.8683 | 28 |
| Pooled repeat | 7975 | 358.6358 | 94.9575 | 27 |
| Original restored | 9685 | 330.6934 | 78.3300 | 29 |

Same short guest-timed clear-combat route, full JIT, one logical CPU, Vulkan,
3x EFB, experimental formatter ON, Cubeb muted, runtime/audio profiling and
no captures. Runs are not deterministic; spans include preemption and nest.
These are diagnostic results, not a controlled proof of allocator causality.
No new visual or audible validation. No frame-rate gain is claimed.

Rejected the pool change; both edited source/test files restored exactly to HEAD.
Rebuilt the original implementation using Build.cmd --cpu jit --jobs 2: exit0,
39/39 native tests in 5.70s. No compiler warnings/errors found; existing CMake
deprecation, Wayland, long-path and lz4 IPO warnings remain. Local LTO ON,
source default OFF. Runtime/port/module-info/launcher binaries and hashes recorded
in evidence/windows-20260930/METADATA-POOL-REJECTED.json. Trial patch/binary and
proprietary test outputs retained privately in .local; user static work untouched.

Next investigate guarded precompilation and non-compilation stalls, not further
allocator tuning without new evidence. Formatter remains default OFF; solid-30
and audio unresolved, three qualifying repeats and ten-minute varied combat
still required. Goal active/unmet.

## Clear combat compilation-burst diagnosis - 2026-09-30

One additional diagnostic on the new guest-timed route exited 0: existing JIT
runner, one logical CPU, Vulkan, 3x EFB, experimental formatter ON, Cubeb muted,
no screenshots, runtime/audio profiling. No runtime changes or rebuild.
The 10.9009-second post-warmup window contained 7431 JIT compilations totaling
291.217ms elapsed. Worst frame 75.1227ms included 35.5118ms compilation and
1502 newly encountered blocks (34749 guest instructions); none of those PCs
had compiled earlier in this run. A 55.547ms frame compiled 410 new blocks,
23.6967ms total. Another 55.1219ms frame compiled only two blocks/0.5261ms;
compilation is not the sole remaining cause. GPU fence spans totaled 2.5173ms.

The jit_backpatch timer measures metadata insertion, not runtime fault handling.
Its 13.6976ms coverage in the second spike is nested inside emission/compilation,
not extra time to add to those spans. All spans include preemption; unclassified
time is not assumed to be CPU work. No new visual/audible quality validation.

Next investigate same-thread JIT prewarming during loading to move bulk cold
compilation out of combat, while separately diagnosing non-compilation stalls.
Direct arbitrary Jit calls are not safe: analysis failures can modify guest
npc/Exceptions and invoke CheckExceptions. Any warming entry point must fail
closed on invalid code/context, preserve translation and feature flags, retain
GQR/speculative-register guards, ordinary SMC invalidation and cache lifetime,
and account for loading cost. No warm-cache implementation or success claim yet.
Formatter remains default OFF. Solid-30 and audio goals remain unmet; three
qualifying repeats and ten-minute varied combat are still required.
Evidence: evidence/windows-20260930/CLEAR-COMBAT-COMPILE-BURSTS.json.

## Visible guest-timed combat and audio comparison - 2026-09-30

Added tools/routes/xbox-timed-clear-combat.json: 26 guest-clock holds totaling
12.26 seconds, covering approach, light/heavy attacks, powers, block and camera.
Five fresh frames before the first capture avoid the observed magenta restore
frame; open-street movement avoids the previous tree occlusion. This is a short
route improvement, not a general visual-readiness fix or ten-minute session.
Both visual runs and both separate no-screenshot timing runs exited 0. Every
hold completed; maximum release lateness was 72 guest cycles at 729MHz. All six
native captures per visual run inspected sequentially: visible living player,
combat poses/web effects, enemies/team, street background and HUD. Grab/use,
charge behavior, exact animation speed and world/interrupt equivalence remain
unproven. Combat outcomes differ; this is not a deterministic input movie.

Same existing Windows runner, JIT, one logical CPU, Vulkan, 3x EFB, Cubeb muted.
No runtime changes or rebuild this checkpoint. After two seconds warmup and
ending at the last input hold (before saving), no-screenshot diagnostics:

| Formatter | New FPS | Guest speed | P99 ms | Worst ms | >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Original/default | 26.1435 | 100.1500% | 59.6310 | 79.9998 | 35 | 24 |
| Experimental on | 29.9708 | 100.0025% | 55.1822 | 70.9986 | 5 | 28 |

Audio profiling remains enabled, so these are diagnostics, not acceptance runs.
Ten full interior audio seconds: original DMA/stream empty dequeues 1/0,
candidate 1/1; maximum callback gaps 17.4256/12.8155ms. Screenshot runs had
41/36 and 49/47 empty dequeues respectively and substantial capture stalls;
do not use those runs as unintrusive performance evidence. These counters do
not establish audible crackling or device underruns. No listening/device-output
verification; sound remains unresolved. Formatter remains default OFF and the
solid-30 goal unmet. Three qualifying repeats and ten-minute varied combat are
still required. Next investigate remaining stalls/audio starvation without
capture overhead and extend healthy varied combat/correctness coverage.
Evidence: evidence/windows-20260930/GUEST-TIMED-COMBAT.json. Private images,
states and logs stay outside Git; user static work remains untouched.

## Guest-clock input timing and movement comparison - 2026-09-30

Added process-local xbox_time commands: port, milliseconds (1..600000), input
fields, optional release, and a required path for the timing receipt. Input
release runs in CoreTiming rather than waiting for rendered frames or a host
sleep. Receipts record start/end ticks, requested duration, clock rate, lateness
and completion. The benchmark rejects inconsistent/incomplete receipts, reused
receipt paths and releases more than 1ms late. Controller polling retains its
usual guest cadence. Between-command gaps are not a deterministic input movie.

First live testing caught event-name reuse: RegisterEvent keeps the old callback,
so the second hold stalled. Original timed out; an initial candidate was started
too early and terminated. Neither attempt is accepted. Fixed one registered
event per core lifetime with a current hold, rebuilt, then ran the retries serially.
Final Windows build exited 0: 39/39 native tests in 7.46s; tooling 138 passed,
one skipped of 139 in 20.767s. No compiler warnings/errors; existing CMake
deprecation, Wayland, long-path and lz4 IPO warnings remain. Local LTO stays ON,
source default OFF. Runtime executables rebuilt; hashes are in the evidence.

Both fixed runs exited 0 using JIT, one logical processor, 3x EFB and Cubeb muted.
Two-second movement hold: original 9 guest cycles late, candidate 0. Half-second
neutral hold: original 50 cycles late, candidate 0 (729000000 ticks/second).
Movement window contained 56 new frames original versus 60 candidate; neutral
contained 12 versus 15. These are counts in guest-clock windows, not host FPS.
All six native captures inspected sequentially. Spider-Man moves from the street
to the same curbside area in both modes; scene, enemies, team, HUD and effects
remain present. This supports a bounded movement comparison, not numeric position,
full world-speed or interrupt equivalence. No audible/device-output verification.

Route: tools/routes/xbox-timed-movement.json, using the private prepared statue
state. Evidence: evidence/windows-20260930/GUEST-TIMED-INPUT.json. Next broaden
guest-timed movement/animation/combat comparisons and establish a healthy visible
varied route. Formatter remains default OFF; crackling and solid-30 combat remain
unresolved. Three qualifying repeats and ten-minute varied combat still required.

## Formatter contract guards and combat shadow validation - 2026-09-30

The default-off formatter now hashes the known caller epilogue as well as its
existing formatter region. It rejects overlapping format/output/caller-frame
storage, wrapped or misaligned frames, unavailable FPU and enabled FP exception
modes. Shadow validation explicitly fails on changed caller-preserved GPR/FPR/CR
state; the benchmark requires the new ABI mismatch counter. Volatile differences
remain diagnostic. These checks do not establish interrupt/timing equivalence or
full transitive helper-code identity; the experiment is still default OFF.

Supported Windows build exited 0: 39/39 native tests in 7.67s. Tooling: 138 run,
137 passed, one skipped in 20.896s. No compiler warnings/errors. Existing CMake
deprecation, Wayland, long-path and lz4 CMP0069 IPO warnings remain. Local LTO
cache remains ON, source default OFF. Produced runtime binaries include
moderngekko-run.exe, moderngekko-port.exe and moderngekko-module-info.exe.

Combat shadow: 469646 completed comparisons, 179931 floating calls, zero output,
FPSCR or preserved-register mismatches, zero pending/abandoned. All five shadow
and five replacement captures inspected sequentially: scene/HUD/enemies and hit
effects present. Replacement Spider-Man becomes downed behind the tree; this is
not a suitable sustained-combat route. All three runs exited 0. The separate
no-screenshot/no-PCM timing run measured 29.9599 FPS after two seconds, P99
46.3253ms, worst 47.4481ms, lowest rolling second 29, guest speed 99.9663%.
Cold restore maximum was 272.4009ms. This is one short diagnostic, not acceptance
or a new substantial performance gain. Cubeb enabled/muted; private pre-volume
PCM captured in the visual run, structurally checked but not listened to. Audio
crackling and device playback remain unverified.

Next fix the measurement limitation: xbox_frames holds for newly rendered frames,
so at 25 versus 30 FPS the input lasts different amounts of game time. Add
guest-clock input timing for movement/animation speed comparison, then broaden
correctness and varied combat validation. Goal active/unmet; three qualifying
repeats and ten-minute varied combat remain required. Evidence and binary hashes:
evidence/windows-20260930/FORMATTER-CONTRACT-GUARDS.json. Proprietary outputs stay
private; unfinished static work remains untouched.

## Original formatter combat cadence - 2026-09-30

The earlier near-30 statue/LTO measurements below used the opt-in formatter
replacement. That experiment remains default OFF and has not passed complete
correctness/timing acceptance. It is not the validated production baseline.

Two new runs with the original formatter, the same LTO runner and statue route,
one logical CPU, Vulkan and 3x EFB gave the following after a two-second warmup:

| Run | Newly rendered FPS | Guest speed | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Original first | 25.4255 | 99.9960% | 53.4737 | 61.6665 | 53 | 23 |
| Original repeat | 25.2343 | 100.0035% | 60.2698 | 64.8466 | 64 | 23 |

Guest intervals include 104/110 frames at 50.050ms and 187/183 at 33.367ms.
These short restored runs are not qualifying repeats or proof of original
console performance. They show that host JIT speed alone is insufficient to
remove delays already present in this measured guest timeline. Intrusive block
profiling separately attributes about 20% of resident guest cycles to the decimal
conversion page; this is not a host CPU time percentage. Next investigate a
behavior-preserving game-library optimization with explicit ABI, interrupt and
world-speed validation, alongside host stalls and improved combat routing.

All four investigation runs exited 0. All five native visual captures were
inspected sequentially: street/statue, team/HUD, enemies and hit effects present;
player partly tree-occluded and low health, statue intact. This is sampled visual
evidence, not full-frame inspection. Cubeb was active but muted. The repeat's
12 interior profile seconds recorded one DMA and three streaming empty dequeues,
maximum callback gap 14.4052ms. Those counters are not device underruns or audible
click counts, and their interval differs from the FPS window. Crackling remains
unresolved; no audible verification. No runtime/source default changed and no
new build was required for these measurements. The preceding 39/39 Windows
build result remains scoped to that build. Goal active/unmet: three qualifying
repeats and ten-minute varied combat still required. Numeric evidence and private
artifact hashes: evidence/windows-20260930/ORIGINAL-FORMATTER-CADENCE.json.

## Windows optional LTO trial - 2026-09-30

The existing ENABLE_LTO option failed with MSVC C2220/C5049 because the shared
PCH embedded absolute build paths under deterministic compilation. A conditional
PUBLIC /pathmap on build_pch maps the build root to a relative prefix; consumers
inherit the same mapping. Deterministic mode and warning-as-error checks remain.
The supported Build.cmd --cpu jit --jobs 2 path then completed: 39/39 native tests
in 17.07s. Existing CMake warnings, D9025 (/W3 overridden by /W0), and D9002
(ignored -fexceptions in a dependency) remain. No final build errors.

Short statue-route comparisons after the same two-second warmup:

| Build/run | Average FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- |
| Baseline earlier | 29.8588 | 46.4992 | 60.6316 | 2 | 29 |
| Baseline repeat | 29.9659 | 48.5620 | 55.5134 | 2 | 29 |
| LTO first | 29.9710 | 42.9138 | 46.8131 | 0 | 29 |
| LTO repeat | 30.0332 | 46.7293 | 51.0239 | 2 | 29 |

These approximately ten-second segments are not deterministic movies or qualifying
acceptance repeats. Full restored-route maxima were 270.1/293.8ms baseline and
304.7/446.1ms LTO; cold restoration is reported separately, not mislabeled as a
steady combat stall. LTO is not established as a major FPS improvement or a fix
for pacing. No default was changed: ENABLE_LTO remains OFF in source; the local
experimental CMake cache is ON and the current runner is the LTO candidate.
Original runner retained at .local/build/windows-x64/runtime/moderngekko-run-pre-lto.exe.
A renamed-baseline launch timed out before restoration; using its original name
completed. Cause unproven; future comparisons should use the original filename.

All five candidate native captures inspected sequentially: statue/street, enemies,
team, HUD and hit effects present. Tree obscures the player, health becomes low,
and statue remains intact. This route needs clearer visibility and progression.
Cubeb enabled/muted; first LTO timing run had eight full interior seconds with no
DMA/streaming empty dequeues and 10.7509ms maximum callback gap. Not a listening
or device-output check; crackling unresolved. JIT, one logical CPU, normal clocks,
3x EFB and opt-in formatter retained. No proprietary captures/saves tracked.
Evidence: evidence/windows-20260930/WINDOWS-LTO-TRIAL.json. Goal active/unmet;
three qualifying repeats and ten-minute varied combat still required. Next pursue
substantial runtime/compilation cost reductions and improve the varied route.

## Xbox fusion aiming and tutorial validation - 2026-09-30

Generated Xbox profile v2 adds right-stick aiming while LB+RB are held. This
reaches tutorial/profile icons between the existing face-button corner targets.
Press A while aiming to confirm; stick movement suppresses the fusion-request
shake and camera input. Neutral stick retains existing corner shortcuts. Existing
custom profiles are preserved, so this does not silently migrate user settings.

The final Windows build passed 39/39 native tests (7.78s); tooling passed 137
of 138 tests with one skip (19.824s). No compiler errors/warnings; existing CMake
deprecation, Wayland and object-path warnings remain. The exact final runner was
used in the successful native replay. Tests cover fractional aim, preset override,
dead-zone fallback, all directions, delayed confirmation and action isolation.

All five successful replay captures were inspected sequentially: ready prompt,
correct icon aim, dismissed dialog, Spider-Man/Captain America fusion with 738
damage, then Hero Training: Fusion (COMPLETE). Street, heroes, HUD, effects and
debris were visible. Route tools/routes/xbox-fusion-ready.json starts from the
private prepared ready-prompt state, not normal boot. An earlier pickup-inclusive
replay exited 0 but failed visually to advance the tutorial; pickup repeatability
is still open. The successful follow-up state is private under
.local/automation/xbox-fusion-ready-stick-20260930/fusion-followup.sav.

This clears a test-flow obstacle, not the performance goal. Cubeb was active but
muted; no listening/device-output verification or new audio/FPS improvement is
claimed. Physical controller testing, other fusion partners and revive remain
open. One logical CPU, Vulkan, 3x EFB and normal speed were retained; formatter
optimization was opt-in. Next establish varied encounters from the completed
fusion state and address cold compilation/audio stalls. Three qualifying repeats
and ten-minute varied combat remain required. Evidence:
evidence/windows-20260930/XBOX-FUSION-AIM.json. Goal active and unmet.

## JIT reverse-link index sharding - 2026-09-30

Finalization phase traces isolated a reverse-link hash-table growth pause:
32768->65536 buckets cost 6.4294 ms. Split the destination-address index into
64 standard maps with stable value references. Linking/unlinking feature checks
and branch patching are unchanged; empty destination entries are still erased.
No extra threads or large upfront reserve. Opt-in phase spans retain >=100us
entry-map/range/link work; slow index logging requires runtime profiling.

Worst linking phase fell from 6.515 ms to 1.300/0.3149 ms in short/full candidate
probes. Whole-finalization maxima fell from 6.5169 to 1.3015/1.0299 ms. Total
compilation cost remains substantial and routes are not deterministic. The full
candidate averaged 29.9701 FPS, P99 39.3357 ms, maximum 94.2242 ms, lowest rolling
second 28, six frames over 50 ms, guest speed 100.0002%. After five seconds of
warmup, maximum remained 51.2082 ms with two frames over 50 ms. This is a targeted
stall reduction, not the requested sustained-30 result or major overall gain.

All five native probes exited 0. All seven candidate native captures inspected
sequentially: combat, scenery/HUD, effects, damage, knockback, broken prop and
hero changes present. Fusion tutorial persists; completed fusion and varied
encounters unverified. Cubeb active/muted; 63 complete interior seconds had two
DMA and one streaming empty dequeues. Not click counts. Crackling unresolved;
no listening/device-output verification. One baseline frame also had a 50.05 ms
guest interval, so not every slow host interval is host-only delay.

Final Windows build exit0, 39/39 tests in5.87s; tooling138 ran,137passed,1skip
in21.645s. Native container test checks stable references and 200000 operations
against the old container. No compiler warnings/errors; existing CMake warnings
remain. Final comment/line-ending rebuild changed artifact hashes after probes;
exact tested and final hashes are recorded separately. Goal active/unmet;
three qualifying repeats/ten-minute varied combat remain open. Next reduce total
first-use compilation and validate fusion/varied encounters, not just this fight.
Evidence: `evidence/windows-20260930/JIT-LINK-INDEX-SHARDING.json`.

## Continuous Xbox combat and queued replay - 2026-09-30

Added `tools/routes/xbox-continuous-combat.json`: attacks, powers, movement,
blocking, jumping, fusion-selection attempts and hero changes without a long
neutral tail. `--queue-route` publishes the complete command sequence before
waiting for receipts, removing per-action host round trips through the native
50 ms command poll. Default serial mode remains available. This is not a
deterministic input movie; advisory status can remain stale while the queue
drains. Timing runs use `--no-screenshots` and end at the route's `read_timing`
guest tick, excluding capture/shutdown overhead. This changes the test route,
not the engine's performance.

The serial continuous route reproduced a 17-FPS rolling second and 156.135 ms
worst frame despite 29.8483 FPS average. Two overlapping one-second audio buckets
contained 11 DMA and 11 streaming empty dequeues. Those counts are not audible
clicks; temporal correlation does not prove the stall's cause. Later replays did
not reproduce that full burst. External CPU sampling cannot explain a burst
that did not recur in the sampled run.

Queued timing/profile runs averaged 29.9702/29.9705 FPS at normal guest speed,
with worst frames 92.225/90.7416 ms, rolling-second minima 28/28, and six frames
above 50 ms each. The profile's worst frame included 55.2543 ms JIT compilation:
32.8388 ms emission, 15.6272 ms finalization, 4.4357 ms analysis (nested spans).
A later 59.4009 ms frame had no JIT/shader/pipeline compilation and 8.4644 ms of
measured waits; remaining elapsed time is not proof of CPU execution. Prior
trace analysis found 1572/1844/1221 first-seen blocks in three cold bursts, with
no repeat addresses in those bursts. Both cold compilation and later stalls
need work; no audio fix or sustained-solid-30 claim is made.

All 14 native captures across two visual routes were inspected sequentially:
street, HUD, heroes, enemies, combat effects and hero changes present. Fusion
tutorial persists; completed fusion and varied encounters remain unverified.
Audio ran through muted Cubeb with private pre-volume PCM capture; no listening
or device-output verification. Crackling remains unresolved. These are short
one-encounter diagnostics, not qualifying repeats or ten-minute acceptance.

All seven runs exited 0 and completed their routes. Tooling: 138 tests ran,
137 passed, 1 skipped in 23.598s. Native source/binaries unchanged from bd9736ff
(prior Windows build exit0, 39/39 runtime tests). No new native rebuild needed
for this Python/route checkpoint. One logical CPU, 3x EFB, normal clocks and
formatter opt-in ON remain unchanged. Goal active/unmet. Next profile bulk JIT
finalization and correlate later stalls with CPU service in the same run.
Evidence: `evidence/windows-20260930/CONTINUOUS-COMBAT-QUEUE.json`.

## JIT backpatch rehash spike reduction â€” 2026-09-30

Thread CPU/cycle diagnostics isolated a 54.6 ms emission of only 29 guest
instructions (181.7 million thread cycles). Slow-instruction tracing located
a floating-point load; memory metadata tracing then measured a 51.0 ms map
insertion. Growth thresholds were 65537, 131073 and 262145 entries: whole-table
rehashing was a concrete source of the pause. Windows CPU time is quantized;
cycles are not converted to wall time.

Replaced the JIT backpatch table with 64 independent pointer hash tables.
Value references remain stable through insertion; missing lookup and clear/reuse
semantics are preserved. No extra threads or large up-front reserve is used.
A native test covers 300000 entries, stable references through growth,
replacement, missing keys, clearing/reuse and page-spaced keys. Fault lookup
uses the returned value pointer. Game operations/protections are unchanged.

Two candidate runs reduced maximum measured insertion from 51.0003 ms to
3.5261/1.6944 ms and longest single compilation from 51.1061 ms to
6.5887/6.9024 ms. The first candidate's affected frame12676 changed from
85.4468 to 34.7919 ms. Overall P99 stayed about 42.2 ms; worst frames were
97.1453/84.2682 ms, average FPS 29.9770/29.9711. Total compilation time did
not improve. Remaining cold bursts compile many blocks in one frame; this
is targeted spike reduction, not sustained solid-30 combat acceptance.

Opt-in diagnostics add emission CPU ns/cycles/address/block-size fields;
-1 means unavailable counters. --jit-emission-address selects one block,
or 0xffffffff for all. Profiling must be enabled; instruction/backpatch
spans below 100us are omitted. Nested spans overlap. Profiling adds overhead.

Final Windows build exit0: 39/39 tests in 7.74s. Tooling137 ran:136 passed,
1 skipped; final span6/6 passed. No compiler warnings/errors found; existing
CMake configuration warnings remain. All seven endpoint captures inspected:
scenery/HUD/characters present, varying attacks/downed heroes, some endpoints
away from the crowd. Neutral route tails do not prove sustained varied combat.
Cubeb was active but muted; private mixer recordings do not verify audible
quality. Crackling unresolved. No qualifying repeats or ten-minute acceptance.

Next address bulk-compilation bursts and improve the varied-combat route.
Goal remains active. Evidence and binary hashes:
evidence/windows-20260930/JIT-BACKPATCH-SHARDING.json.

## Audio capture and JIT compilation stalls â€” 2026-09-30

Added explicit --capture-audio with --profile-audio --audio Cubeb: retain at most
60 seconds of final stereo 16-bit mixer PCM in a preallocated ring, write WAV
only after Cubeb callbacks stop. Volume/mute is applied downstream by Cubeb, so
this records a non-silent signal while host playback stays muted. Ordinary
benchmarks clear inherited capture settings. Non-stereo capture is unsupported
and validation fails if no valid stereo file is produced. Recordings contain
proprietary audio and stay under ignored .local, never Git or source backups.
This captures neither Windows/device playback nor missing hardware callbacks.

Extended opt-in runtime spans to JIT compilation, its analysis/code-generation/
finalization phases, and Vulkan shader/pipeline compilation. Spans include
preemption and nested spans overlap; do not add parent and child durations.
The analyzer's unclassified field means non-wait elapsed time and includes
recorded compilation; it is not CPU utilization or additional work to sum.

Default-size diagnostic: 29.9701 FPS, P99 38.9769 ms, maximum 92.3095 ms,
minimum rolling second 29. All 1230 guest intervals were 33.367 ms. The worst
frame contained 54.4101 ms in JIT compilation; pipeline work was negligible.
A 1024-instruction block-limit experiment did not fix it (max101.8018 ms,
P99 42.5999 ms) and was removed from source and tooling. Final detailed trace:
29.9768 FPS, P99 40.6840 ms, maximum89.6308 ms, minimumrolling28. Its worst
frame included 56.0475 ms in JIT emission, 0.202 ms analysis and 0.3239 ms
finalization. Next separate actual compiler CPU consumption from preemption
inside emission and inspect the expensive code-generation path. No JIT limit,
clock adjustment, rendering reduction or default formatter promotion remains.

Three private PCM recordings contain 48.532/48.772/48.572 seconds of 48-kHz
stereo audio. Peaks23415/28551/25122, no full-scale samples. These signal
statistics DO NOT verify audible quality or absence of crackling. First run's
40 aligned seconds: 4000 callbacks, maximum gap13.3603 ms; DMA/streaming empty
queue counts2/2 (not audible-click counts). The user's sound report remains
unresolved; there has been no listening or device-output verification.

All three native endpoint captures were inspected sequentially. Street scenery,
HUD and heroes are present; baseline ends largely away from the crowd, while
later endpoints show enemies/combat and incapacitated heroes. This does not
establish sustained varied combat. All probes: headless JIT/Vulkan, one logical
CPU, 3x EFB, normal clocks, formatter opt-in ON, Cubeb volume0. No qualifying
three-repeat or ten-minute acceptance. Goal active/unmet; manual test deferred.

Final Windows build exit0, 38/38 runtime tests7.20s. Full tooling suite136 tests:
135passed,1skip22.132s; final targeted runner12/12 and span5/5 passed. No compiler
warnings/errors found; existing configure warnings remain. Exact artifacts,
failed experiment and measurements: evidence/windows-20260930/AUDIO-JIT-STALLS.json.

## Mixed formatter rewrite experiment â€” 2026-09-30

A bounded 64-address unsupported-format census found frequent mixed string,
signed-integer and float calls. The census run counted 399572 unsupported calls,
including 11332 overflow events beyond its first 64 addresses; it is not an
exhaustive ranking. Logs contain addresses/counts, not proprietary format text.

Extended the default-off formatter experiment with transactional bare %s, %d,
%f and %% support. It reads separately bounded GPR/FPR save-area cursors,
retains the original code/hash/caller/direct-RAM guards, rejects string/output
and string/va_list aliasing, null or unterminated strings, spills beyond saved
register arguments, unsupported formats and unsupported floats. Rejections
leave guest output and argument cursors untouched. Supported calls still create
the complete text and advance the caller's va_list; no logs are suppressed.

Shadow compared 769742 completed eligible calls with zero output/va_list/return/
FPSCR mismatches and zero abandoned calls. One of 769743 eligible calls remained
pending at shutdown and is explicitly unverified. Unsupported calls were 56158;
this different live run is not an exact causal reduction measurement. Observed
register-change masks remain GPR=00001ff9, PS0=00000003, PS1=00000000, CR=23.
The census-only run observed wider volatile floating clobbers (PS0 mask1f,PS1
mask03); register preservation is not architectural equivalence. Timing and
broader alias/caller behavior remain unproven; keep the rewrite disabled by
default. It is a library replacement, not instruction-timing equivalence.

The benchmark now requires an actual matching formatter summary, nonzero
completed comparisons/replacements, zero mismatch/abandoned counters and
reconciled eligible/compared/pending counts. Pending original calls at shutdown
are bounded and reported, never counted as verified comparisons.

The first expanded candidate measured 29.9796 FPS at 100.0319% guest speed,
P99 39.0198 ms, maximum 84.8094 ms and minimum rolling one-second FPS 28.
All 1230 guest intervals were 33.367 ms. Four host intervals exceeded 50 ms;
they occurred early (frames 12630,12662,12676,12753) with negligible GPU fence
wait and 68â€“79 ms unclassified elapsed time. Profile JIT/shader compilation and
other host work next; unclassified time alone does not identify the cause.
This single headless diagnostic still fails solid-30 delivery. Three native
endpoint captures were inspected sequentially: actual combat, effects, scenery
and HUD are present, with incapacitated heroes in the candidate endpoint. The
route's neutral tail is not proof of continuous varied combat or ten-minute
acceptance. Source/frame pacing and audio still need broader validation.

Windows build passed: 37/37 runtime tests in 7.19 s, including mixed-format
integer extremes, negative zero, literal percent and transactional rejection.
Python suite: 134 run, 133 passed, one skipped (22.789 s); after pending-count
handling changed, all 11 combat-runner tests passed (0.065 s). No compiler
warnings/errors found; existing CMake warnings remain. Exact binaries and
native diagnostic results: evidence/windows-20260930/MIXED-FORMATTER.json.
Goal active/unmet; sound crackling remains unresolved. All runs remain headless
Vulkan/Cubeb volume=0, JIT, one logical CPU, 3x EFB and normal clocks. No audible
quality claim, qualifying repeat or ten-minute combat acceptance is made.

## Formatter caller-state audit and remaining hotspots â€” 2026-09-30

Extended the default-off formatter shadow diagnostic to aggregate changed
register numbers (GPR, both paired-single lanes, and condition-register fields).
No raw register contents are written. Original code still runs in shadow mode;
replacement behavior and the default-off setting are unchanged.

A headless combat shadow run compared 398399 supported calls, including 150262
floating calls: zero output/va_list/return/FPSCR mismatches, abandoned or pending
samples. Original code changed GPR 0 and 3â€“12, floating PS0 registers 0â€“1, and
CR fields 0, 1, 5. No PS1 changes were observed. Other inspected registers stayed
unchanged. This strengthens the observed caller-state audit; it is not complete
architectural equivalence, interrupt timing, alias coverage or whole-game proof.
Register masks describe original execution, not errors; in replacement mode
there are no shadow register samples and zero masks are not validation.

An intrusive block profile with replacement enabled completed after the headless
submission fix. It recorded 36,504 resident executed blocks, 31.991 billion
cycles versus 32.541 billion guest ticks and zero skipped idle ticks. Region
803c1000 still accounts for 12.463% of recorded cycles; 8036f000 for 12.194%,
80368000 for 7.400%. Inspection identifies remaining decimal conversion and
indirect per-element/paired-single processing loops. The narrow replacement
handled 928630 calls but 918823 were unsupported; investigate those unsupported
formats and live callers before another rewrite. These whole-run formatter
counts and gated resident-block counts have different scopes. Profile timings
are heavily perturbed and must not be cited as release FPS.

A separate candidate run without block profiling averaged 29.8246 FPS at
100.0001% guest speed, P99 51.2428 ms, maximum 84.9062 ms and minimum rolling
one-second FPS 28 (frames 12620â€“13850). Guest intervals: 1074 at 33.367 ms,
84 at 50.05 ms, 72 at 16.683 ms. It still fails sustained-30 pacing. Runtime/audio
span profiling remained enabled; this is a diagnostic, not visible acceptance.
All three native endpoint captures were inspected in order. Heroes, Doombots,
street scenery, HUD and combat effects were present; the candidate endpoint
shows a fusion result and incapacitated heroes. Not sustained varied combat.

Windows build passed, 37/37 runtime tests in 7.57 s. No compiler warnings/errors
found; existing CMake deprecation, Wayland and object-path warnings remain.
Exact build artifacts and candidate timing are in
`evidence/windows-20260930/FORMATTER-STATE-AUDIT.json`. All new runs are headless,
JIT, Vulkan, 3x EFB, one host CPU, normal clocks and muted Cubeb. The user's
visible-performance and sound failures remain unresolved. Goal active/unmet;
no manual retest requested and no experimental replacement promoted.

## Headless GPU submission diagnosis â€” 2026-09-30

The user's visible 17â€“18 FPS and poor-audio report still fails acceptance.
Added opt-in bounded runtime spans (`--profile-runtime`, environment
`OPENMUA2_RUNTIME_SPANS`) for throttle, GPU pacing, worker, Vulkan fence/submit/
present and Presenter elapsed time. `tools/analyze_runtime_spans.py` merges
nested spans and isolates CPU-thread waits; unclassified time is not CPU time.
Traces are emitted after shutdown, capped at 262144 spans, and rejected when
samples are dropped. Ordinary benchmark runs clear inherited span profiling.

Found and fixed a headless-specific scheduling defect: Presenter returned before
submitting GPU work, batching many frames until resources filled. Headless
frames now flush pending vertices and submit through the existing nonblocking
backend Flush path. Visible presentation is unchanged. Do not extrapolate this
fix to the user's visible playtest or treat old headless stalls as its diagnosis.

Matched route/profile diagnostics (one host CPU, JIT, Vulkan, 3x EFB, normal
clocks, formatter off, Cubeb volume=0) reduced accumulated fence waits from
5.041 s to 0.00694 s over frames 12620â€“13850. P99 fell from 93.493 to 50.777 ms;
average FPS rose from 27.669 to 28.581, minimum rolling 1 s FPS from 21 to 24.
Repeat: 27.679 FPS, P99 51.746 ms, total fence waits 0.00658 s.
Scene progression differs; these are bounded diagnostics, not a controlled
visible-game FPS gain. Guest cadence still includes 50.05 ms frames. The goal
remains active/unmet, and sound is unresolved. Native endpoint captures were
inspected; they show street scenery, heroes/Doombots, combat and HUD. This does
not establish continuous varied combat, normal boot, or audible quality.

Windows build: `Build.cmd --cpu jit --jobs 2`, exit 0; 37/37 CTest tests passed
in 7.21 s. Python suite: 133 run, 132 passed, 1 skipped, 23.292 s. Configuration
warns about vendored deprecations, missing Wayland and long object paths; no compiler errors/warnings
found. Exact binaries, run hashes, repeat results and limits are recorded in
`evidence/windows-20260930/HEADLESS-SUBMISSION-DIAGNOSIS.json`.
Next: profile remaining guest frame-production stalls and the visible rendering
path without host UI automation; do not request another manual test yet.

## Audio and frame-cadence diagnosis â€” 2026-09-30

Added an offline synthetic mixer benchmark and opt-in numeric audio profiling.
Use --profile-audio --audio Cubeb with tools/run_combat_benchmark.py; explicit
profiling permits a headless Cubeb diagnostic, while ordinary headless runs
remain silent. Inherited profiling is cleared for ordinary benchmarks. Counters
are bounded, written after callbacks stop, and contain no audio samples.
Channel IDs: 0 DMA, 1 streaming, 2â€“5 remote speakers, 6 portal, 7â€“10 GBA,
11 Cubeb callback. Producer counts cover only 0/1; other producer fields are
unavailable. Final traces include steady-clock anchors for frame correlation.

The synthetic full mixer used about 1% of one CPU, so removing auxiliary mixers
is not a credible tens-of-FPS solution. Live traces found subnormal fades in
auxiliary channels, but all mixing occupied only 1.21523 s across 44 aligned
seconds (2.762% elapsed time, including preemption). DMA/streaming queues emptied
2/7 times in that interval; those counts are not audible-glitch counts.

All new gameplay probes were headless Vulkan/Cubeb with isolated volume=0,
one logical CPU, 3x EFB and formatter off. They are diagnostics, not visible
playtest or audible-quality acceptance. With mid-route screenshots the worst
interval was 533.65 ms; without captures it fell to 100.83 ms. Scene progression
varied, so do not label this a controlled gameplay optimization.

The no-capture run averaged 27.117 FPS at 100.073% guest speed. The last instrumented run
measured 27.180 FPS over frames 12620â€“13850, P99 93.91 ms, maximum 112.59 ms and
minimum rolling 1 s FPS 20. It still fails. Its guest cadence included 237
50.05-ms intervals among 1,230 intervals. A five-second process sample used
66.62% of the single allowed CPU; that is limited headless evidence, not proof
about the visible 17â€“18 FPS manual test. Investigate frame production and waits
before investing in small mixer optimizations.

All four initial native captures were inspected sequentially; the last instrumented run's
one endpoint capture was inspected too. Street scenery, heroes, Doombots,
damage/effects and HUD were present. The final scene has Wolverine active and
Spider-Man down. The neutral-input tail and incapacitation do not establish
ten-minute varied combat. The final screenshot is outside the measured interval.
Audio was muted; the user's poor-audio report remains unresolved.

Windows build/tests passed; exact timings, binaries, hashes and diagnostic
limits are in evidence/windows-20260930/AUDIO-FRAME-DIAGNOSIS.json. No FPS gain,
audio fix or acceptance is claimed. Next separate throttle sleep, CPU execution,
GPU submission/waits and irregular guest-frame production. Keep all normal-speed
and visual requirements, the default-off formatter, and the whole-process
one-core constraint. Goal remains active/unmet. User testing stays deferred;
future manual launches start normally. User edits and proprietary assets remain
preserved outside this checkpoint.

## Xbox controls and failed user playtest â€” 2026-09-30

The physical Xbox One controller was detected, but the first manual launch used
the old generic mapping and a combat savestate. The user reported wrong controls,
roughly 17â€“18 FPS and poor audio, and deferred further playtesting. That FPS is
user-observed, not an instrumented interval. Earlier short 27â€“29 FPS results do
not establish playability. Sustained 30 FPS and acceptable audio remain unmet.

New profiles map actions: A attack/confirm, B heavy/back, X grab, Y jump, LB
block/use, Start pause, RB+face powers and right-stick camera rotation. Power
chords suppress ordinary face actions. LT/RT cycle heroes temporarily; D-pad
still navigates menus/activates direct powers. Direct D-pad hero selection and
contextual X use remain open. Physical Wiimotes are not a supported port target;
physical motion and IR pass-through bindings were removed. Existing custom
profiles are preserved, and fresh CLI profiles select a connected SDL gamepad.

Fusion now has a gamepad-only candidate: LB+RB generates the internal request;
face buttons provide preset aiming and delayed confirmation. A captured
Spider-Man/Captain America sequence produced 723 damage, 6 KOs and completed
Hero Training: Fusion. The final-binary repeat produced 738 damage and 6 KOs.
Fusion was underway before the face-selection command,
so this does not establish every partner choice or revive behavior. Wider
validation remains required. Original UI prompts have not been rewritten.

The new process-local Xbox test device exercises the actual generated profile
and normal controller reports. It creates no OS controller or desktop input.
Tests cover action/chord isolation, axes, release, delayed confirmation and bad
automation values. Native captures confirm pause/resume, hero cycling and camera
rotation. Crowded captures do not individually prove jump, grab or every power.
All captures are reviewed sequentially; this is sampled evidence, not every frame.
Cubeb was active in the action probe; audible quality failed per the user report.
The fusion probe was headless/silent and provides no FPS or audio acceptance.

Windows build: Build.cmd --cpu jit --jobs 2, exit 0; 37/37 runtime tests (6.89 s),
29 profile cases, 127 tooling tests (126 passed, 1 skipped; 24.124 s).
No final compiler errors/warnings found; existing CMake deprecation, Wayland and
object-path warnings remain. An intermediate diagnostic compile error was fixed.
Artifact hashes and scoped results: evidence/windows-20260930/XBOX-CONTROLS.json.

Future manual launches must use normal boot/title, without --load-state. The
requested relaunch was deferred after the user's FPS/audio feedback. No game is
left running. Normal startup with the new profile is not newly visually verified.
Before another manual test, select the new generated Xbox profile explicitly;
existing old profiles were preserved and are not silently migrated.
One logical processor, Vulkan, 3x EFB, widescreen and formatter default-off remain.

Resolution correction: 1920x1080 names the 3x EFB preset, not a fixed output size.
Earlier measured EFB: 1920x1584; raw game frame: 1920x1410; window image excluding
borders: 1536x866. The 2501x1410 PNGs are aspect-corrected native captures.

Next: correlate sustained frame times/guest speed with audio starvation and CPU
costs under the whole-process one-core limit, then close remaining control gaps.
Private assets, saves, captures and logs stay in .local. User StaticRecomp edits,
Build-With-Log.cmd and the unfinished static experiment stash remain preserved.

## Whole-process one-core correction â€” 2026-09-30

The user's requirement is to run the entire game on one CPU core. The earlier
CPUThread=False setting only serialized CPU/GPU runtime work; helper threads
could still use other host cores. Earlier results do not demonstrate the required
whole-process limit and are excluded from acceptance under that requirement.

The Windows runner now selects one already-allowed logical processor before
RunMain, applies process affinity and reads it back. It fails startup if the
restriction cannot be confirmed. All game threads share this processor, including
later audio/I/O/shader workers. It replaces the optional largest-cache mask and
does not change any other process or global Windows setting. The benchmark now
requires both this one-bit host affinity and serial CPU/GPU confirmation.
This limits the game to one hardware thread, including on SMT processors.

The existing JIT backend still translates Wii PowerPC instructions at runtime
using Dolphin components. Affinity controls CPU placement; it does not change
that architecture into a native original Xbox port. The future Xbox port remains
an optimization/design constraint, not a demonstrated implementation.

Windows build exited 0: 36/36 runtime tests in 7.15 s and 8/8 tooling tests.
The OS test checked affinity and 1,000 CPU samples in each of four new workers.
Independent process reads confirmed mask 0x1 at startup and during actual combat
(the latter process had 52 threads, all confined by its process mask).
Current runner: 15,722,496 bytes, SHA256
82e4896d898baf8f828a69e63a4220f46823c0962247b218870b77663bfc63fc.
No compiler warnings/errors found; existing CMake deprecation, Wayland and long
object-path warnings remain. Build log: .local/logs/jit-one-host-core-build.log.
Exact hashes of all four produced executables are in the evidence.

Two short captured combat runs at 1920x1080 preset/3xEFB, Vulkan/Cubeb:

| Formatter | Average FPS | P99 ms | Maximum ms | Lowest rolling 1 s FPS | Guest speed |
| --- | ---: | ---: | ---: | ---: | ---: |
| off | 27.3288 | 60.63 | 488.54 | 16 | 97.471% |
| on | 29.1423 | 58.33 | 461.83 | 18 | 97.370% |

Frames 12620-12990, 370 intervals each; both routes completed and exited 0. Neither
passes solid 30 or normal-speed acceptance. These one-off runs include a midpoint
screenshot and varying combat progression. Severe stalls need a capture-overhead
comparison before attributing them to gameplay. The live end-of-route counter
was lower than the analyzed-window average; it is not the combat-window FPS.

All six native captures were inspected sequentially for actual fights, effects,
HUD and scenery. Actual captures are 2501x1410; sampled visuals and Cubeb activation
do not establish every-frame correctness or audible quality/synchronization.
The formatter remains default-off with its documented timing/correctness limits.
Goal active/unmet; qualifying repeats, ten-minute varied combat and full Xbox
controls remain open. Private assets, outputs, user edits and stash are preserved.

Evidence: evidence/windows-20260930/JIT-ONE-HOST-CORE.json.

## Guarded formatter rewrite experiment â€” 2026-09-30

Single-core CPU/GPU emulation remains enforced. The new default-off
`--simple-format shadow|on` experiment replaces a narrow game-library formatter
operation with portable host formatting: literal text and bare `%f` for bounded,
finite binary32 values. Live code hashing, caller/buffer/ABI and direct-RAM guards
restrict eligibility; unsupported cases execute the original instructions. The
caller still rotates its four buffers and the logging sink still filters/emits.
No messages, effects or game assets are removed or patched.

This is a high-level game-library rewrite, **not instruction-timing equivalence**.
It does not retain the original formatter loop's latency or internal interrupt
points. Clock rates and existing JIT return/downcount handling are unchanged,
but less guest work executes. Normal emulation speed alone cannot validate that
timing change. Keep the experiment off pending broader correctness/timing review;
these results do not satisfy the goal's correctness or sustained-FPS acceptance.

Shadow mode executed the original and matched 284,398 eligible calls, including
104,852 calls with floating conversions. It compared all 1,024 output bytes (untouched tail
included), return count, 12-byte va_list and full FPSCR: zero mismatches, abandoned
or pending samples. This is bounded observed-state evidence, not all architectural
state, exhaustive guest rounding/alias/guard coverage or full-game equivalence.
Host component tests compare supported random floats with snprintf; unsupported
formats and values retain the original path.

Three short runs per mode used the same runner, save, profile and input sequence
at 1920x1080 preset / 3x EFB, Vulkan/Cubeb, normal clocks and single-core mode.
Each measures frames 12620â€“12990, 370 newly rendered frame intervals:

| Mode / run | Average FPS | P99 ms | Maximum ms | Lowest rolling 1 s FPS |
| --- | ---: | ---: | ---: | ---: |
| Off 1 | 26.4021 | 51.37 | 73.57 | 22 |
| Off 2 | 28.3425 | 50.47 | 77.03 | 26 |
| Off 3 | 26.1663 | 66.73 | 83.41 | 23 |
| On 1 | 29.6495 | 51.06 | 67.49 | 27 |
| On 2 | 29.8070 | 61.08 | 67.54 | 28 |
| On 3 | 29.4533 | 51.39 | 73.44 | 26 |

Mean of run averages: 26.9703 off versus 29.6366 on
(+2.6662 FPS,
+9.89%). Input receipt
timing and combat progression vary; this is provisional evidence, not a precise
causal gain. All six runs fail solid-30 pacing. The earlier literal-only pair
measured 25.9815 versus 26.3175 FPS and was insufficient by itself.

All 18 native captures from these six short runs were inspected sequentially:
heroes/Doombots, attacks, beam/impact effects, damage, street/building/tree scenery,
HUD and fusion tutorial were visible. Aspect-corrected native captures are 2501x1410 (not window dimensions).
This is sampled visual evidence, not every frame. Mid-route capture can add
stalls. Cubeb activation is confirmed; audible quality/synchronization is unverified.

**Route correction:** seven captures from the full original-code shadow replay
show attacks carrying Spider-Man out of the crowd and into an empty alley late
in the sequence. Earlier full-route averages remain diagnostic and must not be
described as continuous crowded combat. An uncaptured original-code shadow run
also produced an unexplained 56.4917 FPS outlier; no replacement was active.
Retain it as unresolved evidence, excluded from FPS-gain or frame-cap claims.

Windows `Build.cmd --cpu jit --jobs 2` exited 0: 36/36 runtime tests in 4.97 s,
8/8 tooling tests. Current runner: 15,722,496 bytes, SHA256
4a0ad7d58d5d9123ca9b78c697ba6a13f32dec8075a660197767e3b009c242aa.
ModernGekko.exe, moderngekko-port.exe and moderngekko-module-info.exe were also
produced; artifact hashes are in the evidence. No proprietary game DLL is needed.
Final compiler diagnostics: no warnings/errors found. Existing CMake deprecation,
missing Wayland and object-path warnings remain. An initial compile failed on a
missing Memmap include and reported two size-conversion warnings; both were fixed
before the successful builds. Log: .local/logs/jit-simple-format-build.log.

Next expand guest correctness/guard/timing validation, repair the combat route
to stay engaged, then profile remaining slow frames. The three qualifying repeats,
ten-minute varied combat session and full Xbox controls remain outstanding. The
Xbox control table stays at the bottom of the to-do file. Goal remains active.
Proprietary data/raw logs/captures stay ignored; user StaticRecomp edits,
Build-With-Log.cmd and the unfinished static experiment stash are preserved.

Evidence: evidence/windows-20260930/JIT-SIMPLE-FORMAT.json.

## Single-core requirement and active JIT callers â€” 2026-09-30

The user requires single-core execution and intends an eventual original Xbox
port. CPU/GPU emulation now has a per-run CPUThread=False override, including
imported per-game settings. The benchmark writes the same setting into its
isolated profile and rejects missing/dual-core effective runtime confirmation.
Threaded comparisons below are historical and excluded from promotion. Favor
portable algorithms, less work and lower memory use. This Windows x64 backend
is not an Xbox port or proof of whole-process performance on one Xbox CPU;
Windows audio/I/O/shader workers remain possible.

Added opt-in --jit-profile-callers with --jit-block-profile: up to 32 selected
blocks, 64 distinct LRs per block, first argument/raw-stack examples and explicit
overflow counts. Ordinary runs disable inherited profiling. This is intrusive,
resident-block-only evidence, not release FPS or a complete call-stack trace.

The crowded caller probe completed with exit 0 in 70.266 s. Three selected
blocks recorded 11,107, 319,639 and 222,615 entries; counts reconcile with the
block dump, with zero overflow. Sampled formatting serves animation diagnostics.
Five 32-byte active code samples match the original DOL. The live logger has
one stream with mask 7: observed channels 0x20 and 0x1000 are discarded after
formatting; channel zero remains enabled. Next prove filter and temporary-buffer
side effects before guarded lazy formatting. No logging was disabled, original
game data changed, clock/cycle charges altered or FPS gain claimed.

Windows Build.cmd --cpu jit --jobs 2 exited 0. Runtime tests: 35/35 in 6.93 s;
tooling tests: 8/8. Runner: 15,593,984 bytes, SHA256
af4b30488e8a4293bb0f6a10614a643b434c94a38fe6bf7701c8d672f1708164.
Build log: .local/logs/jit-caller-single-core-final-build.log. No compiler
warnings/errors found. CMake deprecation, missing Wayland and object-path
warnings remain. ModernGekko.exe, moderngekko-port.exe and module-info were also
linked; exact artifact hashes are in the evidence. No generated game DLL needed.

An actual Cubeb combat replay with conflicting imported per-game CPUThread=True
completed with exit 0 in 27.219 s and confirmed dual_core=0, 3x EFB and normal
clocks. All three native captures were inspected sequentially: crowded heroes/
Doombots, attacks, beam/impact effects, damage, scenery, HUD and fusion tutorial.
The preset is 1920x1080; actual window captures are 2501x1410. This short captured
route validates settings and sampled content, not FPS acceptance or every frame.
Audio quality/synchronization remains unverified. Sustained 30 FPS, three-repeat
and ten-minute varied combat acceptance, and full Xbox controls remain open.

Evidence: evidence/windows-20260930/JIT-CALLERS-SINGLE-CORE.json.
Private data/captures remain ignored. User static edits, Build-With-Log.cmd and
the unfinished static experiment stash are preserved. Goal remains active.


## JIT block profile and crowded control comparison â€” 2026-09-30

Full JIT remains primary; sustained 30 FPS is unmet. Added opt-in
`--jit-block-profile` to the process-local combat harness, with reset/dump
commands and an explicit disabled-profile error. It changes only the copied
benchmark profile. Ordinary runs explicitly disable inherited JIT profiling
and debugging. This profiler is intrusive and reports resident blocks only;
invalidated blocks are absent. Its timings are not release-FPS evidence.

The crowded profile completed with exit 0: 35,597 resident blocks,
24.387 billion recorded guest cycles versus 24.780 billion elapsed guest ticks,
and zero skipped idle ticks. Read-only inspection identifies decimal conversion
and 64-bit division among hot loops. Region 803c1000 contributes 19.05% of
recorded cycles. Trace active callers, including indirect calls, before choosing
an optimization. Debug configuration files alone do not prove active logging;
no original game code/configuration or cycle charges were changed.

Sequential uninstrumented same-runner controls at 1920x1080 preset / 3x EFB,
Vulkan/Cubeb and normal clocks measured 27.524 FPS single-core and 26.318 FPS
threaded over frames 12620â€“13690. P99: 50.66 / 52.63 ms; maximum: 83.40 /
84.41 ms; rolling one-second minimum: 22 FPS both. Input receipt delays can
change exact guest timing. These one-off results do not establish a gain or
acceptance; threaded execution and the presentation queue remain unpromoted.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; 34/34 runtime tests (5.18 s)
and 7/7 tooling tests passed. Runner: 15,589,376 bytes, SHA256
bbc951840accbe8ee3fddad92e506d91c92aa56aa6306975bbf556f93e3fc740.
CMake deprecation/Wayland/path-length warnings remain; final compiler errors
and warnings: none found. Two earlier attempts failed on misplaced parser
tests; corrected before the successful build. Log:
.local/logs/jit-block-profile-build-final.log.

An intentional negative run inherited an enabled diagnostic profile, disabled
it, rendered combat, then correctly rejected the dump command without creating
a misleading file. Harness exit 1 was expected; runner cleanup exited 0.
All three native combat captures were inspected sequentially: heroes/Doombots,
attacks, effects, damage indicator, HUD and scenery. The earlier portrait/camera
probe also exited 0; all four captures were inspected. The game confirmed the
previous fusion with 724 damage and six KOs. The active hero did not change;
camera motion overlaps fusion completion and needs an independent check.
Full Xbox mapping remains pending. Cubeb activation is confirmed; audible
quality/synchronization and every-frame visuals remain unverified.

Evidence: `evidence/windows-20260930/JIT-BLOCK-PROFILE.json`.
Three-repeat and ten-minute varied combat acceptance remain outstanding.
Private assets, raw profiles, disassembly and captures remain outside Git.
User static edits, Build-With-Log.cmd and the static experiment stash are preserved.

## Presentation experiment and native fusion input â€” 2026-09-30

Full JIT remains primary. Goal active/unmet. A clean heavier fight measured
26.0101 FPS over 41.14 seconds at the same 1920x1080 preset / 3x EFB and
100.008% emulation speed: P99 66.43 ms, maximum 107.19 ms, rolling one-second
minimum 22 FPS. Guest copy intervals also slow (median 37.69 ms, P99 62.81 ms).
Next profile guest frame production; presentation smoothing alone cannot fix it.

The default-off `MODERNGEKKO_PRESENT_QUEUE=1` / `--present-queue` experiment
copies XFBs into independent GPU textures and presents a bounded FIFO at half
refresh. It never advances the game or invents frames. One short single-core
test retained 29.969 FPS and reduced P99 to 36.18 ms / maximum 41.24 ms.
This is not a solid-30 pass and adds latency. Rush+smoothing regressed to
23.10 FPS with 35 queue underflows and is rejected. The earlier cache-lock
prototype falsely counted duplicates at ~60 FPS and is explicitly rejected.
The final snapshot design preserves cache identity and duplicate detection.

Added process-local Wii/Nunchuk acceleration and native Nunchuk shake inputs.
`pad_frames` accepts `release=0` to retain a chord across captures; its default
still releases. Updates use the controller state lock. The new input test checks
actual serialized Nunchuk acceleration/buttons, clearing, and shake waveform.
Native X-axis shake with Z opened the fusion tutorial. Subsequent pointer-ready
confirmation and Captain America portrait selection consumed stars and showed
attack effects. All four entry, four intermediate and five corrected-selection
captures were inspected sequentially. The tutorial banner remained. This proves
the tested input path, not completed Xbox controls or every-frame correctness.

The PS2-derived Xbox target table is at the bottom of docs/PERFORMANCE-GOAL.md.
Full mapping, camera and direct hero selection remain pending. No UI changes.
The user does not need to test yet. Fusion now has a usable process-local route
for extending combat: .local/automation/jit-fusion-ready-20260930/fusion-ready.sav.

An earlier crowded replay failed command 128; its exact cause was lost by the
old status-file handling. Failed commands now retain a per-command reason before
publishing their receipt, and the harness saves exceptions in result.json.
An intentional pause/pad_frames negative run confirmed correct rejection and
preserved detail; a later full crowded replay completed. The old failure is not
reclassified as a pass or claimed fixed without a diagnosis.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; 34/34 runtime tests and 6/6
combat tooling tests passed. Runner: 15581184 bytes, SHA256
05841f688879dd9e707823630641f82b7beedebe00a1a4db17fde39efd82fc25.
CMake deprecation/Wayland/object-path warnings remain; no compiler warning/error
diagnostics found. Private build log: .local/logs/jit-native-shake-input-build.log.
Cubeb activation verified; audible quality/sync unverified. Three-repeat and
ten-minute sustained combat acceptance remain outstanding. Evidence:
evidence/windows-20260930/JIT-QUEUE-AND-INPUT.json.
All proprietary data stays private. User static diagnostics and Build-With-Log.cmd
remain uncommitted; the unfinished static experiment remains stashed.

## JIT pacing diagnostics and profile-directory fix â€” 2026-09-30

The JIT primary path is committed at 0642cd3e. Additional opt-in
`MODERNGEKKO_PRESENT_TIMES` telemetry records XFB-copy, before-present and
after-present events, intended/queued timestamps and effective runtime settings.
`--trace-presentation` exposes it in the combat harness. Buffers report overflow;
copy counts are not FPS, queued timestamps are not display scanout. Copy guest
ticks are recorded only in single-core mode to avoid reading the CPU clock from
a separate video thread. Guest/VI timestamps do not prove CPU-only causality.

Fixed a startup omission: create the profile directories before UICommon Init.
The previous fresh profiles lacked Config and Cache parents, preventing config
and shader-cache persistence. Actual fresh-profile validation now found six
shader-cache files and saved Dolphin.ini. This is not a claimed combat FPS gain.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; runtime tests 32/32, including
bounded presentation-event serialization. Combat tooling tests 5/5 passed.
Runner: 15572480 bytes, SHA256
f7bc2cf579f763e8667345323e7506605ac0913026575114b05c6511213305e9.
Same CMake deprecation/Wayland/path-length warnings; no compiler warning/error
diagnostics. Private build log: .local/logs/jit-presentation-cache-build.log.

At the same 1920x1080 preset / 3x EFB, one-off instrumented short runs measured:
default max 52.89 ms; immediate 47.40 ms; immediate+smoothing 50.07 ms;
dual-core+immediate 43.77 ms; dual-core+immediate+smoothing 49.98 ms.
These private setting experiments were not promoted. Immediate mode needs
further visual/composition validation and is not unique-frame acceptance proof.
The final cache-fix runner's default run measured 29.9700 FPS, P99 51.29 ms,
maximum 55.77 ms, 99.9998% emulation speed. No solid-30 pass.

Default before-to-after presentation is usually below one millisecond. Copy
production already varies: guest copy intervals in the final single-core run
had median 33.2944 ms, P99 39.2912 ms and maximum 45.0064 ms. VI quantization
adds 16.683/33.367/50.050 ms presentation intervals. More host CPU throughput
alone cannot be assumed to remove this cadence. Next profile production and
extend real continuous combat. Ten-minute acceptance remains unverified.

Evidence: evidence/windows-20260930/JIT-PRESENTATION-DIAGNOSTIC.json.
The final runner also completed a longer mixed visual route with Cubeb and exit
0. All four native captures were inspected sequentially: Spider-Man selected,
destructible scenery, an enemy-introduction cutscene, and crowded combat effects
with a fusion tutorial. This is not continuous-combat FPS evidence. Native game
Options confirmed A attack, B smash, D-pad powers, Z block/use, C jump, +/- hero
switching, 1 camera, 2 pause, and Z plus Nunchuk motion for fusion. The existing
harness exposes buttons/sticks, but not Nunchuk motion. Private route/save:
.local/automation/jit-varied-visual-20260930/varied-combat.sav.
All original data, generated output, caches, native captures and raw logs stay
private. User static diagnostics and Build-With-Log.cmd remain uncommitted.

## JIT primary path built and repeated â€” 2026-09-30

The user explicitly selected full JIT. Build.cmd and Run.cmd now default to JIT;
--cpu staticrecomp retains the native path and --native-rel still implies it.
The runner accepts --cpu jit|staticrecomp, overriding legacy environment selection.
JIT validates the original local game and runs without a generated module DLL.
Backend-specific build receipts retain binary verification. The combat runner
requires the selected CPU backend to be confirmed in runtime.log.

Build.cmd --cpu jit --jobs 2 exited 0: 32/32 Windows runtime tests passed.
Tooling: 123 run, 122 passed, one existing skip. After adding legacy receipt
preservation coverage, targeted workspace tests: 46 passed, one existing skip.
Actual runner negative checks
rejected invalid backend names and rejected a missing static module despite a
conflicting JIT environment setting. CMake reported deprecations, unavailable
Wayland and object-path-length warnings; no compiler error/warning diagnostics.
Runner: 15569920 bytes, SHA256
d8f0e926a4a827dfdc30a410d28f3c8e6f6412a25f8a8ef9f571f8df7e9a62c7.

Three module-free JIT/Cubeb combat repeats at the same 1920x1080 preset / 3x EFB
completed with clean receipts and exit 0: 29.9712, 29.9697, 30.0676 FPS over
frames 11472-11625, approximately 100% game speed. P99: 51.02, 51.23, 50.51 ms;
maximum: 52.03, 53.90, 54.06 ms. These short runs do NOT prove solid 30 FPS.
Guest VI/presentation intervals also alternate among 16.683/33.367/50.050 ms.
The analyzer now reports guest cadence and host-minus-guest interval residuals;
these are not a CPU-only profile or proof of game-simulation causality.

A rebuilt-runner extended visual replay completed with Cubeb and exit 0. All
five native captures were inspected in sequence: heroes/enemies, movement,
damage/effects, HUD, scenery and a downed hero were visible. This sampled review
does not establish every-frame correctness or audible audio quality/sync.
Tooling tests overlapped the visual run, which is excluded from FPS comparisons.

Goal active/unmet: next investigate presentation/producer timing and extend the
route for genuine ten-minute combat. No repeated-save-load substitute for that
acceptance. No proprietary assets, translations, captures or raw logs committed.
The unfinished direct-chunk patch is retained in the stash named "Preserve
unfinished direct chunk experiment before JIT priority". The user's three static
diagnostic edits are restored and remain uncommitted; Build-With-Log.cmd retained.
Evidence: evidence/windows-20260930/JIT-PRIMARY-CHECKPOINT.json.

## Aggregate native execution profile â€” 2026-09-30

The opt-in dispatch profiler now totals every PC before truncating its ranking.
It records the first/last dispatch timestamp without adding new clock reads.
This separates measured generated-call time from the enclosing wall span; the
remainder includes profiling, scheduling, fallback, synchronization and runtime
work and must not be described as dispatcher CPU time alone.

Runtime rebuilt with exit 0. All 32 scoped moderngekko tests passed. An initial
unrestricted ctest invocation exited 8 because third-party fullbench/fuzzer/
zstreamtest executables were unbuilt; that remains a scoped validation limit.
Two Clang combat profile runs completed with clean receipts and exit 0:
13.481/21.894 seconds generated/span, then 15.164/24.358 seconds. The first
run overlapped runtime tests; the repeat began after they ended. XEMU background
load remained. Both attribute about 62% of instrumented wall time to generated
calls. Neither run is release-FPS evidence. Aggregate consistency checks passed.

Evidence: `evidence/windows-20260930/AGGREGATE-DISPATCH-PROFILE.json`.
The 30 FPS goal remains unmet. Next investigate generated-code entry/return
frequency and helper overhead, not only the external dispatcher. Preserve
native eligibility/hash checks, exceptions and interrupt/timing boundaries.

## Compiler candidates completed; combat still below target â€” 2026-09-30

Both build sessions finished with exit 0. Clang/O2 produced a 170525696-byte
module, SHA256 47922fb187294b601b2b8667805c42bfda639ecfe9ffc014d36b7d1a0bcb6d51,
with all 524 chunk hashes verified. MSVC/O2/Ob2/c1024 produced a 407890432-byte
module, SHA256 bd4ea9876c137637a4d1947468dabd68aba4bde7c27ec881813a9e3ca18b6b00,
with all 2094 chunk hashes verified. Both use module ABI 3 / CPU ABI 4.
MSVC pipeline tests: recompiler 19/19, runtime 32/32. Clang recompiler tests:
19/19; a private probe of its actual module helpers passed 16/16 signed
single/double halfway-rounding cases across all four guest RN modes. This is
bounded validation, not complete floating-point or gameplay equivalence.

At the existing 1920x1080 preset / 3x EFB, sequential same-runner combat tests
measured baseline 6.1936 FPS, Clang 10.9880 FPS, MSVC c1024 9.4445 FPS over
frames 11472â€“11625. P99 frame times: 283.17, 167.95, 170.13 ms respectively.
All routes completed and exited 0 with command receipts validated. Compiler
load had ended; XEMU was using CPU in the background. These single comparisons
are diagnostic, not repeated or sustained acceptance. Goal remains unmet.
Clang is the stronger compiler candidate; the production default is unchanged.

A Clang windowed Cubeb run completed with exit 0. All four native captures were
inspected sequentially: heroes/Doombots fighting, damage indicators, hit effects,
HUD and scenery were visible. These are sampled images, not every frame.
Audible quality/synchronization remains unverified. A gated Clang dispatch
profile completed: leading entries include 803682c4 and 8035ba00; frequent
scheduler/context entries remain. Next measure generated execution versus
runtime dispatcher/continuation costs before another expensive compiler sweep.

Exact metrics, hashes, warnings and profile samples are in
`evidence/windows-20260930/COMPILER-COMBAT-COMPARISON.json`. Clang reported one
nested-comment warning and unused CMake variables. MSVC reported generator
SMC warnings and CMake deprecation/Wayland/path/unused-variable warnings; no
compiler error diagnostics. Private assets, generated C, images and raw logs
remain under .local. Earlier statements below about live builds are historical.

## Idle-cycle hypothesis ruled out for this fight â€” 2026-09-29

Added read_timing to the process-local automation protocol. It snapshots
CoreTiming ticks and idle_ticks under a CPU-thread guard, then releases the
guard before writing the requested private file. It never generates host input
or changes CPU timing policy. Missing-path parsing is rejected. Runtime rebuilt
with exit 0 and no reported warning/error; all 32 Windows runtime tests passed.

Completed native and JIT combat probes both had zero idle-tick delta:
native over 3651052829 ticks; JIT over 4385648130 ticks. Both exited 0 with
clean command receipts. The differing command-route intervals are not paired
FPS windows. Compiler load was active; these timing-only runs had no captures
or audible audio. IDLE-CYCLE-PROBE.json records the measurements. Do not pursue
idle skipping as the explanation for this fight's native/JIT speed gap.

Two module builds remain active: session 99026 for the isolated c1024/Ob2
MSVC experiment, and session 66634 for an isolated Clang/O2 module with the
original 4096-instruction chunks. Clang 22.1.2 already exists at
C:/Program Files/LLVM/bin/clang.exe and targets x86_64-pc-windows-msvc.
No toolchain was installed. The private .local/scratch/build-clang-module.cmd
uses workspace CMake helpers and the existing Windows SDK; its output is
.local/build/windows-x64/module-mg01-rel-clang-o2/gRMSE52_recomp.dll.
Clang uses O2, fp-contract=off, no-fast-math, IPO off. Its script audits with
the existing verifier and writes .local/receipts/clang-module-experiment.json
only on success. It does not replace the default build receipt. See
.local/ACTIVE-PERFORMANCE-BUILDS.json for active commands/logs. Neither module
has a completed audit or performance result yet. Goal remains active and unmet.

## Ob1 build completed; no combat speedup â€” 2026-09-29

The long-running session 60093 finished with exit 0. Windows native module:
.local/build/windows-x64/module-mg01-rel-o2-ob1/gRMSE52_recomp.dll,
591055360 bytes, SHA256
11b4d161ffc02f1ff9c49292973961651c999249e43ac0adf3b0a38dbc24f631.
Audit passed all 524 chunk hashes, module ABI 3 / CPU ABI 4. Recompiler tests
19/19; runtime tests 32/32. Full build warning counts: D9025 276 (275 W3/W0,
one expected O2/Od), C4711 4364, C5045 93, D9002 1 (ignored -fexceptions).
No compiler error diagnostics. Exact results: INLINE-EXPERIMENT.json.

After this project's compile/backup ended, the same-runner 3x timing pair
measured baseline 6.4243 FPS versus Ob1 5.8556 FPS over the same 153 intervals.
Both completed with exit 0 and clean receipts. This rejects Ob1 as a performance
win; it is not a repeated acceptance result. The DLL also grew from 314896384
to 591055360 bytes. Production default remains Ob0. A separate windowed Cubeb
run completed; all four native captures were inspected in order and showed
combat, damage/effects, HUD and scenery. Audible quality/sync remains unverified.

Next experiment: Build.cmd --native-rel --c-chunk-instructions 1024
--module-msvc-inline 2 --jobs 4. The new explicit chunk-size option isolates
DOL/REL generation keys and module output, records configuration and overrides
inherited generator settings. Generation verifies the DOL section-derived chunk
count instead of assuming 325 chunks. Default remains 4096. Tooling tests:
118 run, 117 passed, one skipped. This experiment is running in session 99026,
log .local/logs/combat-c1024-ob2-build.log. Do not restart a live build.
Smaller functions may permit stronger optimization but add dispatch boundaries;
no performance gain is assumed. Goal remains active and unmet.

## Combat-only dispatch profile â€” 2026-09-29

The earlier per-dispatch profile included boot/restoration. Added optional
STATICRECOMP_PROFILE_GATE_FILE: native profiling waits for a private marker,
checked at 65536-dispatch intervals until it opens. The benchmark publishes
that marker only after the restored frame threshold. One latched decision
covers both timestamps, preventing a gate transition between start/end.
Normal runs clear this variable. The profile includes initial neutral frames;
it is not exactly the narrower frame-analysis window.

The rebuilt Windows runner completed the gated combat route with exit 0,
clean command receipts and one gate-open event. The ranking corroborates the
old profile: 8035ba00 (1233.5 ms), 803682c4 (1165.9 ms), context/scheduler and
other frequent short entries remain expensive. These intrusive timings under
compiler load are for ranking only, not release FPS. No captures or audio were
collected in this timing-only run. See COMBAT-GATED-PROFILE.json for exact data.

Runtime build: exit 0, no reported warning/error. Tests: 32/32 Windows runtime
and 5/5 benchmark tooling passed. Runner SHA256:
cd561d89b1018078272a429d82f50822cafb77851ac790abc3bed8639e518457.
Use the same current runner for both native DLL comparisons.
The Ob1 build is still live in session 60093, last seen 443/534. No candidate
module result yet; goal remains active and unmet.

Source inspection found the direct cross-chunk call emitter's table setter
has no callers in the current DolRecomp source. Do not simply enable it:
its direct calls would need native eligibility/hash/host-call and REL checks,
bounded cycle handling and regression coverage before a safe game experiment.
This is a future design lead, not an implemented optimization.

## Selective JIT profiling checkpoint â€” 2026-09-29

Added opt-in STATICRECOMP_FALLBACK_USE_JIT=1 for forced fallback ranges.
Default execution remains unchanged; without the flag (or without a JIT),
forced ranges still use the interpreter. The benchmark accepts --jit-ranges,
validates address ranges, rejects combination with --jit-diagnostic, clears
inherited diagnostic variables, and records the selected ranges in metadata.
Native eligibility/hash checks, host-call handling and timing remain active.

The runtime rebuilt through the existing MSVC environment with exit 0 and no
reported warning/error in its incremental build log. All 32 Windows runtime
tests passed. Tooling: 116 tests run, 115 passed, one skipped. An existing
source-pattern test needed its expected condition updated for the new opt-in
JIT gate; the empty-range short-circuit remains covered.

Concurrent-build diagnostics at the unchanged 3x preset: fresh native control
5.7365 FPS; scheduler-region JIT 6.1493; broader math/library-region JIT 6.7784;
80000000-80500000 region JIT 9.1802. These are not clean acceptance benchmarks
or a production speedup. All four scheduler-run native captures were inspected
in order and showed ongoing combat, damage/effects, HUD and scenery; other runs
were timing-only. No sustained-30, every-frame or audible-quality pass.
A complementary upper-code-region run measured 5.8206 FPS over 151 intervals
(two fewer than the other 153-interval diagnostic windows).
See evidence/windows-20260929/SELECTIVE-JIT-DIAGNOSTICS.json. Raw data is private.

The isolated native Ob1 module build remains live in session 60093, last seen
at 432/534; no candidate DLL/audit/FPS result yet. Do not restart it merely for
a stationary log. New runner SHA256:
9fd119537f37407911838cdb447e511b6bdb296d90084329f43eb993f4f51807.
Compare both baseline and Ob1 using this same runner after the build finishes.
Goal remains active and unmet; production defaults have not been changed.

## Command-publication race fixed; extended combat route â€” 2026-09-29

The extended route exposed a Windows sharing failure while renaming a command
from .txt.tmp to .txt. Root cause: ListCommandFiles consumes every regular
file in the watched directory, including temporary files. The runner now
stages in a separate sibling directory, publishes a closed file atomically,
and retries only bounded Windows sharing/lock violations. Cleanup stop uses
the same publication path. Completion now requires exactly the expected
processed command names and zero failed command files. Four focused tests
passed; full tooling suite: 115 run, 114 passed, one skipped.

Audited prior completed combat command receipts and exact payloads. The
3x baseline, 3x JIT, native profile, trace-off retry and both runtime/audio
checks passed. The old 1x run has failed/000003.txt.tmp and is EXCLUDED from
controlled resolution comparisons; COMBAT-BASELINE.json marks it invalid.
The aborted trace-off attempt and first extended-route attempt remain failed.
See evidence/windows-20260929/COMMAND-RECEIPT-AUDIT.json. The valid same-3x
native-versus-JIT comparison still supports a CPU-path bottleneck.

The corrected extended native route completed with exit 0 and clean receipts.
All five native captures were inspected in order: movement away from the
lamppost, active melee, damage/effects, reduced enemy health and defeated
Doombots. This is sampled visual evidence, not every-frame validation. The
power attempt did not establish activation; a separate three-capture ability
probe also completed but did not clearly demonstrate a shield throw. Do not
claim a power-effect pass. Both runs used Null audio and concurrent compiler
load, so neither is an optimization or sustained-FPS result.

Private extended route: .local/scratch/combat-route-extended.json; completed
run .local/automation/combat-extended-control-build-load-retry contains
combat-extended.sav (frame around 11832). Ability probe lives in
.local/automation/combat-power-control-build-load. Preserve the original
short route for direct baseline/candidate comparisons.

The Ob1 build remains live in session 60093, still compiling large generated
functions with no reported failure. No candidate DLL/audit/FPS result yet.
Continue the live build, then benchmark with the corrected publisher. Goal
remains active; no production speedup or sustained-30 claim is made.

## Rebuilt-runtime combat and audio-path checks â€” 2026-09-29

The Ob1 module build is still live in terminal session 60093; it has passed
chunk0165 and chunk0167 with no reported compile failure. Large generated
functions can take many minutes and several GB of compiler memory. Do not
restart this build merely because its log is temporarily unchanged.

While it compiles, the rebuilt runtime completed two baseline-module combat
replays, both exit 0 and route_completed=true. All four native screenshots
from each replay were inspected in chronological order: heroes, Doombots,
attacks, damage numbers/flashes, HUD, scenery and destructible props appeared.
These are sampled captures, not inspection of every frame. The headless run
used No Audio Output; the windowed run reported Cubeb. Audio-enabled execution
is confirmed, but audible quality/synchronization is not. No host input or
screen capture was used; windowed replay used process-local automation.

Added --windowed and --audio to tools/run_combat_benchmark.py. The tool rejects
non-silent audio requests in headless mode because the runtime forces silent
audio there. CLI rejection was checked (exit 2 before output creation), Python
compilation passed, and the windowed Cubeb replay completed. Defaults preserve
silent headless timing runs.

Common frames 11472..11625 under concurrent compiler load measured 5.335 FPS
headless and 6.320 FPS windowed/Cubeb. These are functional checks under load,
not a controlled compiler comparison or a performance improvement. Private
runs: .local/automation/combat-ob1-control-build-load and
.local/automation/combat-ob1-control-audio-build-load. Both use the unchanged
baseline Ob0 module. The Ob1 module has not linked/audited or been benchmarked.
Next: finish the live build, audit, compare baseline/candidate without compiler
load, and continue toward the unchanged sustained-30 combat goal.

## MSVC helper-inline experiment in progress â€” 2026-09-29

The sustained-30 goal remains active. Added opt-in `--module-msvc-inline 1`
with separate module output and build-receipt metadata; default remains Ob0.
Existing strict floating point, IPO-off setting and chunk0201 Od workaround
are unchanged. Windows Build.cmd --native-rel --module-msvc-inline 1
--module-suffix o2-ob1 --jobs 2 is still running in the local workspace.
Runtime compilation/link succeeded and ModernGekko tests passed 32/32;
recompiler tests passed 19/19. Tooling suite ran 110 tests, 109 passed and one
skipped; after adding output-isolation coverage, workspace suite ran 42 tests,
41 passed and one skipped. No complete candidate module or FPS result yet.

A separate MSVC O2/Ob1/fp:strict compile of the historically troublesome
chunk0165 succeeded (769562-byte private object). Full module compilation
has started (534 build steps). Nonfatal dependency/CMake warnings and D9025
were observed. Private log: .local/logs/combat-ob1-build.log. Keep the build
running; inspect completion before restarting anything. Its terminal session
is 60093. Probe session 83777 completed with exit 0.

The rebuilt runner SHA256 is
9c5af2645a5544bdea5e87dba76e27ada82d70b3539176423740349def6fffba.
The build receipt remains stale until the full Build.cmd finishes its module
audit. Baseline DLL remains .local/build/windows-x64/module-mg01-rel-o2/
gRMSE52_recomp.dll. Candidate output is module-mg01-rel-o2-ob1 in the same
build root. Existing user diagnostic edits were stashed during runtime build,
then restored; they are not in this runner or this checkpoint commit. The
preservation stash remains available. Build-With-Log.cmd is untouched.

Next: finish module build/audit, replay the same combat save, and repeat the
baseline beside the candidate. Another unrelated project was compiling;
record contention and do not stop it. Inspect native captures before claiming
correctness. No production speedup, audio pass, or sustained-30 pass is claimed.

## Combat timing and CPU comparison â€” 2026-09-29

Goal remains active; no production speedup or sustained-30 pass is claimed.
Added opt-in bounded unique-frame timing at after_present, independent of the
status-file loop; no file I/O on the video thread. Added the replay runner and
analyzer documented in PERFORMANCE-GOAL.md. Timestamp tests cover duplicate
presents, rewind epochs and overflow; analyzer rejects lost/discontinuous data
and reports frame tails, rolling FPS and normal-speed ratio. The optional
status read tolerates transient Windows sharing errors; an aborted attempt was
retained and a complete retry succeeded.

Windows/MSVC runtime rebuild exited 0, ModernGekko 32/32 tests passed. Tooling
suite 108 passed / 1 skipped (109 run); subsequent analyzer suite 5/5 passed.
Native DOL+REL audit remains 524/524 PASS. Compilation had nonfatal dependency
warnings (including D9025 and C4711), with no compile/link failure.

Replayed combat.sav with repeated A-button attacks. Native captures showed
Doombots, attacks, reduced health, hit effects and debris. All measured runs
exited 0 and completed the route. Common frames 11472..11625 (153 intervals):
- Native 3x with captures: 6.062 FPS, p99 302.36 ms, 20.23% normal speed.
- Native 1x with captures: 5.762 FPS, p99 274.39 ms, 19.16% normal speed.
- Native 3x without captures: 5.899 FPS, p99 264.21 ms, 19.62% normal speed.
- Diagnostic JIT 3x with captures: 30.069 FPS, p99 88.98 ms, 100.00% speed.
These short comparisons point strongly to the native CPU path, not resolution.
The JIT run is not a substitute for native optimization or sustained acceptance.
A trace-off replay took 23.907 s after warmup versus 25.750 s trace-on; one pair
cannot resolve tracing overhead from run variability. Per-dispatch profiling is
intrusive; use its cost ranking, never its FPS as release performance.

Next: pursue major generated-code/dispatch improvements, starting with the
Windows global /Ob0 workaround versus explicit helper inlining
and repeated short-block overhead. Keep all guards and timing. Time-heavy
blocks include 8035ba00, 803682c4, 803ee9e8 and 803eea68; fixed-stride dispatch
frequency alone was not a useful cost ranking.

Private runs under .local/automation/combat-* contain traces, copied saves,
metadata and captures. Public aggregate: evidence/windows-20260929/COMBAT-BASELINE.json.
Current runner SHA256 d349b67d272b526dc3875fa362a1201baf20696ef71f5f899ca3c5245791cab7.
The main build receipt still describes the prior runner; regenerate it with the
normal Build.cmd --native-rel before normal receipt-verified launch. This
checkpoint used explicit hashed runner/module paths. Original binaries are
preserved under .local/perf-baseline-789e77f9. Pre-existing diagnostic edits are
restored after the telemetry build and remain excluded from tested binaries.

## Active sustained-30-FPS goal â€” 2026-09-29

The user requested major frame-rate improvements and will not accept less than
solid 30 FPS in combat. The active goal and acceptance criteria are recorded in
`docs/PERFORMANCE-GOAL.md`; small improvements do not complete it.

Planning is complete and execution has begun. GitHub main was checked at
f14ea4a8. Existing runtime diagnostic edits remain untouched and excluded from
the running binaries. First enemy encounter reached via process-local inputs;
native captures showed multiple Doombots, enemy health UI, projectile/effect
activity and reduced player health. This is a checkpoint inspection, not full
combat correctness or sustained-30 validation. Private run:
`.local/automation/20260929T164700-combat-route`, clean exit 0 after 395.797 s.
`route-progress.sav` (first encounter) and `combat.sav` (enemies surrounding
Captain America) are private reusable checkpoints. Saves were created; replay
repeatability remains to be checked. No speedup implemented or claimed yet.

The diagnostic run recorded 3,291,406,422 native dispatches, 9,200,590 bursts,
985,217 JIT fallback entries, 514,671 native exceptions and zero failed chunks.
Hot dispatch samples include 803f2b60, 803f2b38, 803f680c and 80007448. These are
frequency samples, not time attribution, and fixed-stride sampling can alias
loops. Do not infer the bottleneck or a speedup from the counts alone.
Next: replay combat, implement low-overhead unique-frame timing and benchmark
analysis, establish uninstrumented combat baseline, then profile CPU/GPU and
short-block/OS scheduling costs. Preserve all timing and verification guards.

## Windows performance checkpoint â€” 2026-09-29

OpenMUA2 was actually run using the audited Windows O2/indexed native DOL+REL
binaries from the c54bde7 source checkpoint. Native application captures confirmed
the opening cinematic, Latveria level environment, four heroes, HUD/minimap,
player movement and party-following. Combat was not reached or measured.

- Ryzen 7 8745HS, Radeon 780M (driver 32.0.23033.1002), 27.8 GiB usable RAM.
- Configured 1920x1080 preset maps to 3x EFB scale (1920x1584 backing buffer).
  Native aspect-corrected captures were 2501x1410; this was an offscreen Vulkan
  run with Null audio, not a measured 1080p window presentation.
- Opening-level traversal/idle/obstacle-contact interval: **1,366 new frames /
  219.516 seconds = 6.223 FPS**, mean reported emulation speed **20.666%**.
- Counted unique XFB frames against monotonic wall time, excluding duplicate
  presentations. Headless fps/vps status fields stayed zero and were not used.
- No combat FPS, 1% lows, audio validation or full-game visual correctness claim.
  Captures were inspected at checkpoints, not every rendered frame.
- Both private runtime sessions exited 0. Source diagnostics restored after the
  build remain uncommitted and were not part of the tested binaries.
- Evidence: `evidence/windows-20260929/PERFORMANCE.json`. Raw logs, captures,
  copied saves and checkpoints remain private under `.local/automation/`;
  latest run `20260929T161554-action-fps` has `level-start.sav` and `traversal.sav`.
- Next performance work: reach an actual enemy encounter, measure a repeatable
  combat interval, then profile CPU/GPU before attributing the low frame rate.

## Windows build checkpoint â€” September 29, 2026

The existing Windows entry point completed successfully on the local x64 host:
`Build.cmd --jobs 2 --native-rel`, Release, MSVC 19.44.35225.0, Ninja, CMake 4.3.1.
Both O2/indexed modules linked with IPO off and passed the native audit: DOL-only
325/325 chunk hashes; native DOL+REL 524/524, including uncovered-address rejection.
The runtime was incrementally rebuilt; the two module trees were newly built.
The selected build receipt points to `.local/build/windows-x64/module-mg01-rel-o2/gRMSE52_recomp.dll`.

The initial DOL module failure exposed missing indexed-array support in the
module-table generator. Exact coverage parsing now handles those arrays while
preserving merged-table precedence and SMC/hash protection. Regression tests
cover holes, hashes, and malformed intervals. Version metadata no longer assumes
`master` exists, and source backups now accept the canonical handoff document.

Validation: tooling 104 passed / 1 skipped; DolRecomp 19/19; ModernGekko 31/31;
broader runtime CTest 50 passed with `playTests` disabled and the CI exclusions;
GXRuntime 1/1. These runtime suite counts overlap. Nonfatal upstream CMake and
test-fixture deprecation warnings remain. Detailed warnings, binary hashes and
sizes are in `../evidence/windows-20260929/VALIDATION.json`; the latest handoff
records private local log paths. Pre-existing uncommitted runtime diagnostic
edits are preserved separately from this build and source commit.

This is a verified Windows build/module audit, **not a game execution**. RMSE52
was not launched, and no current gameplay or FPS result is available.

## Current reconstructed source

The repository still originates from the recoverable MG01 + FPC01 workspace, but the following missing pieces have now been reimplemented from the retained source/evidence and committed as new work rather than presented as recovered MR01 bytes:

- Native DOL + REL packaging and runtime eligibility are present. RMSE52's absolute linked REL section-table pointer is accepted with guest-RAM bounds checks, and native REL code remains protected by the existing chunk-hash / SMC verification path. Different or modified code is not made native merely to advance boot.
- REL metadata generation now binds the tracked live audit to the exact REL bytes it validated. Commit `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` requires audit status `LIVE_TEXT_MATCH` and an exact SHA-256 match against `source_rel_sha256` before replaying relocations or emitting native REL metadata, so a stale audit cannot silently authorize changed REL bytes.
- REL relocation replay is now bound to the retained live-text audit result as well. Commit `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771` requires the zero-adjust comparison to be an exact match, requires well-formed and equal expected/observed text SHA-256 values, and verifies the replayed executable text bytes against that hash before native REL metadata is emitted. Same-size but incorrectly replayed text is therefore rejected.
- DolRecomp emits `dcbf`, `dcbst`, `dcbi` and `icbi` through the dedicated cache-control hook instead of the generic instruction-fallback callback. The Dolphin cache invalidation semantics and generated cycle accounting remain intact.
- The generated scalar FMA helper has been repaired to use the same instruction-shaped arithmetic/rounding path as the runtime floating-point implementation. Targeted regression coverage compares result bits, FPSCR state and write/no-write behavior across rounding/NI/VE/FI/FR cases and edge values.
- The lockstep checker contains local-loop boundary alignment: when native execution yields back at an inlined loop header, the interpreter shadow continues until it has performed the native charged work rather than comparing different loop iterations at the same PC.
- The runtime floating compare path preserves the fifth FPRF classification bit while replacing only FPCC, with a targeted runtime regression.
- MEM2 is now included in lockstep memory journaling/restoration. MEM1 and MEM2 use disjoint physical journal keys; native and interpreter MMU writes feed the same diagnostic journal, the shadow receives the pre-block image, and the native post-image is restored afterward.
- Merged DOL+REL dispatch keeps native eligibility bounded to generated code chunks. Alignment and overlap guards are covered, and commit `67db5f86875f6e40dca4f70b65fce63996a251ad` adds a regression proving an uncovered guest-address hole between generated ranges is not bridged into native eligibility. Commit `b777ddbe0d2c556fc00ee632bd273d0c1ee13621` additionally rejects unaligned generated chunk boundaries before emitting the merged table, and `5e7fc98e09f53c33f6c383682973af8824889d13` pins that PowerPC instruction-alignment invariant with a regression.
- Multiplatform performance is now the active development priority. Commit `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` makes DolRecomp's existing page-indexed native dispatch the default and changes the OpenMUA2 workspace native-module optimization default from O0 to O2 on both Windows and Linux. `--dispatch-lookup linear` remains available for controlled A/B comparison, and generation cache keys/receipts record the dispatch mode so measurements cannot accidentally reuse output from the other mode.

## Active development priority â€” multiplatform performance

Performance on both Windows and Linux is the primary focus. Existing correctness, SMC, audit, and eligibility guards remain enabled, but expanding lockstep/correctness coverage is deferred unless a performance change produces a concrete failure that blocks measurement or execution.

Current performance baseline on `main`:

- native module optimization default: `O2`
- generated dispatch lookup default: `indexed`
- explicit A/B control: `--dispatch-lookup indexed|linear`
- build receipts record `module_opt` and `dispatch_lookup`
- DOL and REL generation cache identities include the dispatch mode
- combined DOL+REL dispatch now uses a 4 KiB guest-page index to narrow each lookup before binary search (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`), with empty/single-chunk page fast paths from `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6`
- ordinary DolRecomp indexed dispatch now emits an exact page-local run window; empty pages return immediately, single-run pages skip the scan, and multi-run scans cannot walk beyond the current 4 KiB page (`405b81de4136a7532e966218185a190f6eb9230d`)
- x86-64-v3 capability probing is cached (`917a5d893087736a74566dbaf71de5f3989a6d90`), normal chassis dispatch is bound to the selected baseline/v3 function at module load (`4e677ac8491bc3b5256998ba3688893f61d07695`), and generated indirect dispatch reuses the same module-load choice (`5e6ad207c77affbf500bf5327ce6222e9e7fd7c1`)
- native burst host-call checks now reuse the verified chunk index and consult cached per-chunk host-call coverage before invoking exact-address lookup (`2776fa0a3e80136495a32552b9d909e16dcfcd5e`); chunks known clean skip the per-block host-call probe while candidate chunks retain the exact check
- native eligibility now skips the out-of-line forced-fallback range scan entirely when no forced-fallback ranges are configured (`12a75b2db225678368be5e2b6340218ba2aafa2f`); configured ranges retain the existing exact range check
- the interpreter/fallback branch now applies the same empty-range short circuit (`20dd90851c3a2625ddb973f8850e2494bb33ca9f`), avoiding `IsForcedFallbackAddress()` when no forced-fallback ranges exist while preserving configured-range behavior
- native host-call eligibility now reads the cached per-chunk host-call state directly and only calls `ChunkContainsHostCall()` for unknown state (`5b87d08b8f6e4daff2ca64bab75d67632ec28721`); exact address checks remain limited to candidate chunks
- chassis-entered generated dispatch now uses dedicated host-call-free helpers (`8f8c32a9c90e039c898d78eb8313c7d9d64f28c5`), avoiding a second generated `ppc_host_call()` probe after StaticRecomp has already rejected host-call PCs; replacement dispatch and physical MEM1 alias fallback remain intact, while generic/indirect generated dispatch stays host-call-aware
- native dispatchability now returns the runtimeâ†’linked PC it already resolved and the burst loop reuses that value for the immediately following dispatch (`af7938bdb63d4530ea43a6ba445800fc4171a153`); continuation eligibility similarly carries the next linked PC, removing the duplicate `ResolveNativeAddress()` immediately before every native dispatch while preserving linkedâ†’runtime translation afterward
- post-dispatch REL return translation now carries the active REL section index and fast-paths returns that remain in the same section (`7b7e412f671039e1d29e2cdff7b2e51509bc046e`); cross-section and DOL returns retain the existing full `ResolveRuntimeAddress()` scan
- continuation-side runtimeâ†’linked resolution now reuses the previous active REL section as a first lookup hint (`7a9cdc0165257ec19301972785150e4961f7f65e`); same-section continuations avoid the linear active-section scan while hint misses preserve the old full scan, direct-DOL lookup, and REL refresh fallback
- native burst continuation now preserves the linked result returned by generated dispatch and, when its invariants remain valid, verifies the next chunk directly through `FastDispatchableLinkedAt()` (`de411096f5986980ac58e1f3a7373d8f5351dc67`). Forced-fallback and exact host-call checks still use the runtime address; REL chunks must still belong to the resolved active section; lockstep-checked blocks, native exception returns, and any direct-linked miss fall back to the existing runtimeâ†’linked resolver/REL-refresh path
- the native burst back-edge now checks `ppc.downcount > 0` and CPU running state before continuation eligibility (`d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`), avoiding a chunk/REL/host-call probe when the burst must already terminate while leaving the existing continuation path unchanged for eligible bursts
- the interpreter-only fallback loop now checks `ppc.downcount > 0` and CPU running state before `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)` (`b78431f21625ad61b4f66855f5f94b859f04cf7a`), avoiding chunk verification/REL refresh/host-call probes when the fallback slice must already end while preserving the same checks before any eligible re-entry
- native-entry host-call rejection now preserves the exact `host_call_at()` result and reuses it in the fallback branch (`7b11a1869c85aec8d5384c7f044779454e37a69f`), avoiding an immediate duplicate `IsHostCallAddress(ppc.pc)` lookup while retaining the direct lookup whenever module activity/dispatchability was not proven
- no current-main game-side speedup is claimed until the proprietary RMSE52 route is measured

Historical profiling showed very high native-dispatch counts and concentrated time in tiny runtime/cross-chunk entries. The accepted dispatch work now reduces lookup cost in both the ordinary DolRecomp path and the merged native DOL+REL path, removes repeated host-feature selection from normal/indirect dispatch, skips host-call address probes in chunks already known clean, reuses runtimeâ†’linked REL resolution across eligibility and dispatch, fast-paths same-section linkedâ†’runtime and runtimeâ†’linked transitions, and reuses verified generated linked results across eligible burst continuations. The next optimization work should focus on remaining cross-section/cross-chunk transfer overhead and other chassis checks that still execute on every native block before revisiting lower-volume correctness work.

## Fresh validation of current work

For the MEM2 repair, a focused Linux validation workflow first applied only the six intended source-file changes, configured/built GXRuntime, ran its tests, configured/built the ModernGekko Linux runtime and ran the ModernGekko tests. Every step completed successfully before the source repair was integrated into `main` as commit `26334ad651567bf098401f781d1189e5f75c296c` (`Journal MEM2 in lockstep verification`).

After that integration, the normal GXRuntime matrix completed successfully on both `ubuntu-latest` and `windows-latest`. This is useful Windows portability coverage for the shared runtime change, but it is **not** a full Windows game execution test.

The cross-platform ModernGekko workflow introduced by `c1eeb1a2fb8d9ea31954c818fe21d9269b7edc25` completed successfully. Its standalone tests and full configure/build/test jobs all passed on both Ubuntu and Windows, including the MSVC/Ninja Windows path. This remains source/runtime validation rather than proprietary RMSE52 execution.

The project tooling workflow for `67db5f86875f6e40dca4f70b65fce63996a251ad` completed successfully on both Ubuntu and Windows. The later tooling run for `5e7fc98e09f53c33f6c383682973af8824889d13` also passed on both `ubuntu-latest` and `windows-latest`; that regression verifies unaligned generated chunk metadata is rejected instead of entering the merged native dispatch table.

Tooling Actions run `35938476536` for `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` also passed on both `ubuntu-latest` and `windows-latest`. Its new regression verifies that exact audited REL bytes are accepted, changed REL bytes are rejected by SHA-256, and a non-matching live-audit status is rejected.

Pull-request tooling Actions run `35939460805` for the replayed-text guard passed on both `ubuntu-latest` and `windows-latest` before merge to `main` as `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771`. The regression accepts correctly replayed text, rejects same-size replayed bytes whose SHA-256 differs from the live audit, rejects inconsistent expected/observed audit hashes, and rejects a text-section mismatch count.

Performance PR #2 (`a6328f40bb0c98a58c8f50e52a30d4da55390b7e`) was validated before merge by OpenMUA2 tooling run `35940341273` and DolRecomp run `35940341449`: both Ubuntu and Windows jobs passed. ModernGekko run `35940341332` had both standalone Windows and Ubuntu tests passing at merge time; its two larger full build/test jobs were still compiling and are not counted here as completed results.

Performance PR #3 (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`) page-indexes the combined native DOL+REL dispatcher. OpenMUA2 tooling run `35941158719` passed on both Ubuntu and Windows before merge. Its tests cover page-index emission, empty uncovered pages, alignment, overlap rejection, and the existing dispatch-hole invariant.

Performance PR #9 was merged as `405b81de4136a7532e966218185a190f6eb9230d`. It adds an exact page-local run end to the ordinary DolRecomp indexed dispatcher, fast-paths empty and single-run pages, and bounds multi-run scans to the current page. DolRecomp Actions run `36000036463` passed configure/build/test on both Ubuntu and Windows. ModernGekko Actions run `36000035131` subsequently completed successfully: standalone and full build/test jobs all passed on Ubuntu and Windows.

Performance PR #13 was merged as `2776fa0a3e80136495a32552b9d909e16dcfcd5e`. It returns the already-resolved verified chunk index from dispatchability checks and gates `IsHostCallAddress()` behind cached `ChunkContainsHostCall()` coverage in native bursts, preserving exact host-call checks only for candidate chunks. OpenMUA2 tooling run `36003571111` passed on Ubuntu and Windows. ModernGekko run `36003570930` passed all four jobs: standalone and full build/test on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #14 was merged as `af7938bdb63d4530ea43a6ba445800fc4171a153`. It carries the runtimeâ†’linked PC already produced by `DispatchableAt`/`FastDispatchableAt` into the immediately following module dispatch and through native-burst continuation, removing the duplicate `ResolveNativeAddress()` call before dispatch while keeping post-dispatch linkedâ†’runtime translation intact. OpenMUA2 tooling run `36008961778` passed on Ubuntu and Windows. ModernGekko run `36008961830` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #17 was merged as `7b7e412f671039e1d29e2cdff7b2e51509bc046e`. It carries the active REL section index alongside the linked PC and uses that section as a hint for post-dispatch linkedâ†’runtime translation. Same-section returns now translate directly; cross-section and DOL returns still use the full `ResolveRuntimeAddress()` scan. OpenMUA2 tooling run `36026447896` passed on Ubuntu and Windows. ModernGekko run `36026447968` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #18 was merged as `7a9cdc0165257ec19301972785150e4961f7f65e`. It adds the reverse same-section hint to continuation-side runtimeâ†’linked resolution: the previous active REL section is checked first, and a miss falls back to the original full section scan, direct-DOL lookup, and `RefreshRelSections()` path. OpenMUA2 tooling run `36030032034` passed on Ubuntu and Windows. ModernGekko run `36030032243` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #19 was merged as `12a75b2db225678368be5e2b6340218ba2aafa2f`. It avoids calling `IsForcedFallbackAddress()` from `FastDispatchableAt()` / `DispatchableAt()` when the configured fallback-range vector is empty, preserving the exact existing range behavior when ranges are present. OpenMUA2 tooling run `36035472440` passed on Ubuntu and Windows. ModernGekko run `36035472344` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #20 was merged as `5b87d08b8f6e4daff2ca64bab75d67632ec28721`. It reads `m_chunk_host_call_state` directly in the native burst path so cached clean/candidate chunks avoid the out-of-line `ChunkContainsHostCall()` call; unknown chunks retain lazy discovery and candidate chunks retain exact `IsHostCallAddress()` checks. OpenMUA2 tooling run `36038408893` passed on Ubuntu and Windows. ModernGekko run `36038408875` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #21 was merged as `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5`. It adds dedicated generated chassis dispatch helpers that skip the duplicate `ppc_host_call()` probe already handled by StaticRecomp, while preserving replacement dispatch and physical MEM1 alias fallback; normal/indirect generated dispatch remains host-call-aware. OpenMUA2 tooling run `36042200541`, DolRecomp run `36042200581`, and ModernGekko run `36042200621` all passed on Ubuntu and Windows, including full ModernGekko MSVC/Ninja integration.

Performance PR #22 was merged as `dc43d362d425134222d00aa02dd4dbec99fcf222`. It removes the redundant explicit `m_module_active` check from the native burst back-edge because `fast_native_continue()` already rejects inactive modules on both the REL/forced-fallback and direct lookup paths. OpenMUA2 tooling run `36047001104` and ModernGekko run `36047001119` passed on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #23 was merged as `20dd90851c3a2625ddb973f8850e2494bb33ca9f`. It extends the empty forced-fallback-range short circuit to the interpreter/fallback branch in `Run()`, preserving configured forced-fallback behavior and all native dispatch, REL, SMC, timing, and exception semantics. OpenMUA2 tooling run `36055136834` passed on Ubuntu and Windows. ModernGekko run `36055136817` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #24 was merged as `de411096f5986980ac58e1f3a7373d8f5351dc67`. It preserves the generated linked result across eligible native burst continuations so the next verified chunk can be checked without the normal runtimeâ†’linked resolution round trip, while retaining runtime forced-fallback/host-call checks, active-REL-section validation, lockstep/native-exception guards, and the existing resolver/REL-refresh fallback on any invariant miss. OpenMUA2 tooling run `36071914753` passed on Ubuntu and Windows. ModernGekko run `36071914841` completed successfully: standalone and full build/test jobs all passed on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #25 was merged as `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`. It checks cycle-budget exhaustion and CPU running state before `fast_native_continue()` at the native burst back-edge, so a burst that must already stop no longer performs another continuation chunk/REL/host-call eligibility probe. Exception breaks and the continuation behavior of still-eligible bursts are unchanged. OpenMUA2 tooling run `36074380247` passed on Ubuntu and Windows. ModernGekko run `36074380288` passed all four jobs: standalone and full build/test on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #27 was merged as `b78431f21625ad61b4f66855f5f94b859f04cf7a`. It moves the interpreter-only fallback loop's cheap cycle-budget and CPU-running checks ahead of `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)`, so an exhausted or stopped slice no longer verifies chunks, refreshes REL eligibility, or probes an exact host-call address solely to discover that the slice cannot continue. Those checks still run before any eligible re-entry. OpenMUA2 tooling run `36130494685` passed on Ubuntu and Windows. ModernGekko run `36130494695` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #28 was merged as `7b11a1869c85aec8d5384c7f044779454e37a69f`. It stores the exact host-call result from an otherwise-dispatchable native entry and reuses that result in the immediately following fallback branch, removing a duplicate exact-address host-call lookup only on the path where `host_call_at()` already proved the same PC. Non-dispatchable/module-inactive paths retain the direct lookup. The first tooling run (`36142577297`) exposed one stale source-string regression; that test was updated without changing runtime behavior. Current-head tooling run `36142703033` passed on Ubuntu and Windows. ModernGekko run `36142703037` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

The earlier current-source commits also include cross-platform DolRecomp CI for the cache-control generator change and cross-platform GXRuntime CI for the FMA/runtime changes.

## Validation boundary

No fresh RMSE52 WBFS boot/gameplay session has been run from GitHub CI after the latest cache-control, FMA and MEM2 changes because the proprietary game image is intentionally not in the repository. Historical MR01 gameplay and the 11,832-comparison differential pass remain useful evidence and reproduction targets, but they are **not** treated as fresh validation of current `main`.

The latest recorded live native-REL benchmark predates the direct cache-control generator change: it advanced game frames but remained far below the 30 FPS target. A speedup from the later source changes must be measured rather than inferred.

No new claim is made here for a complete level, long-session stability, multiplayer, audible audio, game-owned save/reload, physical-controller gameplay, or Windows game execution.

## Immediate priorities

1. Build the current `O2 + indexed` native module/runtime against the exact RMSE52 image and benchmark the same route on Windows and Linux wherever the local game workspace is available.
2. A/B `--dispatch-lookup indexed` against `--dispatch-lookup linear` with the same compiler, optimization level, route, warmup, graphics/audio settings, and sample window. Keep both raw results.
3. Profile native-dispatch/chassis overhead on the faster baseline: dispatch count, hottest dispatch PCs, burst length, host-call checks, REL address translation, native exceptions, and JIT fallback. Ordinary/merged dispatch lookup, host-feature selection, clean-chunk host-call probing, and duplicate runtimeâ†’linked resolution have now been reduced, so measure after these changes.
4. Optimize shared generated/native transfer paths that benefit MSVC and GCC/Clang together. Same-section REL translation is fast-pathed in both directions, eligible continuation reuses the generated linked result directly, empty forced-fallback scans are removed, cached host-call state is read directly, duplicate generated host-call chassis probes and native-entry exact lookups are removed, and both native and interpreter burst/fallback back-edges exit on exhausted downcount/CPU stop before expensive eligibility work. A concrete next shared target is per-slice game-ID gating: `SConfig::GetGameID()` acquires the metadata mutex and returns a `std::string` copy every timing slice before comparing it to the static module's `char game_id[8]`. Add a narrow locked predicate that compares the stored game ID in place (while still reading current metadata every slice) and use it for module activation, preserving dynamic metadata-change visibility without the repeated string copy.
5. Rebuild and re-measure on both platforms after each accepted performance change. Do not infer a speedup from source structure or CI.
6. Defer additional lockstep/correctness expansion until performance work reaches a useful plateau or a concrete failure blocks further performance measurement. Existing correctness/SMC/audit guards stay enabled.

The original extracted `sys/main.dol` remains the boot source. Any merged/generated DOL/REL reference is code-generation material only and must never replace the game's real boot DOL.
