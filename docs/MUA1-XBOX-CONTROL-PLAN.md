# OpenMUA2 Xbox controls: MUA1 Xbox 360 target

The user's 2026-10-01 correction supersedes the earlier PS2-derived target and
v4 profile. Successful input-routing probes do not establish a correct layout.
The current binary still implements v4; this document is the implementation plan.

## Required common controls

| Input | Required behavior |
| --- | --- |
| Left stick | Move |
| Right stick | Camera, with context-specific aiming only when appropriate |
| A | Light attack; menu confirm |
| B | Heavy attack, hold to charge; menu back |
| X | Grab/pick up or contextual use, selected by game context |
| Y | Jump and character traversal |
| LB | Block/dodge; must not activate contextual use |
| RT + A/B/X/Y | Four powers; suppress ordinary face-button actions |
| D-pad | Direct hero selection in gameplay; navigation in menus |
| Start/Menu | Pause and resume |
| Back/View | Hero management target; verify the available game action |

MUA1 sources: [Xbox 360 gameplay description](https://drkwaitingroom.com/2023/04/28/game-corner-marvelua/)
and [control reference](https://strategywiki.org/wiki/Marvel%3A_Ultimate_Alliance/Controls).
The latter has incomplete Xbox columns; do not use it alone to assert unverified
Xbox-specific auxiliary bindings. RB/map and stick-click behavior need a verified
Xbox reference and a matching action in this version before being assigned.

## MUA2 additions

Fusion and partner selection need an additional binding. Proposed extension:
LT opens selection, then a hero direction chooses the partner. This replaces
MUA1's team-command use of LT. The exact selection/confirmation interaction is
not yet implemented or validated. Do not silently substitute trigger cycling.

In this Wii-derived version, revival uses fusion selection with a fallen hero.
Keep it within the same selection flow; do not import the separate health-pack
mechanic from MUA2 Xbox 360. [Wii-specific revival reference](https://www.cheatcc.com/articles/marvel-ultimate-alliance-2-review-for-nintendo-wii-wii/).

Hacking and any pointer/gesture interactions require contextual stick/button
adapters, not physical motion controls. Hacking and fusion are new MUA2 features;
boosts belong in menus rather than requiring a new combat button.
[Official guide, Wii/PS2/PSP section](https://ptgmedia.pearsoncmg.com/images/9780744010879/samplepages/1087-9_MUA2.pdf).
Exact hacking and gesture replacements remain unverified.

## Implementation sequence

1. Establish a guarded per-player game-action boundary and menu/gameplay context.
   Current profile expressions combine grab from two attack inputs and share
   one input between block and use. A global remap cannot separate these.
   Never use transient stack references as ownership guards.
2. Implement X context priority and LB block independently at that boundary.
   Preserve the game's per-player action-consumption/deduplication rules.
3. Resolve ordered team slots to eligible actors and implement direct D-pad
   selection. The observed control flag and team lookup are research leads,
   not a verified multiplayer ownership contract. Handle dead/absent heroes,
   already-selected heroes and heroes controlled by another player explicitly.
4. Move powers to RT and integrate the proposed LT fusion/revival flow, including
   selection cancel/release. Preserve ordinary attacks outside modifier contexts.
   Keep menu navigation, confirm/back, pause/resume and hero management coherent.
5. Add exact v4 generated/persisted profile migration with backup and rollback;
   preserve customized profiles. Update the bottom quick reference only when
   implemented behavior is established, and label all remaining limitations.

## Validation and release gates

- Real expression-parser tests for modifier priority, threshold boundaries,
  release, simultaneous inputs and independent controller ports.
- Game-action adapter tests for context, ownership and selection eligibility;
  tests must exercise behavior rather than mirror mapping strings.
- Supported Windows build through Build.cmd --cpu jit --jobs 2 and native tests.
- Bounded process-local tutorial-plaza validation: no host input, no screenshots,
  one logical CPU, normal clocks, existing Vulkan 3x rendering configuration.
- Confirm observable game behavior, not merely active bits or rendered frames.
  Visual/menu interaction, physical controller and audible quality remain
  unverified wherever the allowed evidence cannot establish them.
- Keep proprietary code, memory dumps, states and generated output in .local.
  Do not add hard-coded player addresses without game/version and object guards.

No new FPS sweep is authorized by this plan. The performance goal remains open;
the latest concrete user priorities are audio and corrected Xbox controls.
