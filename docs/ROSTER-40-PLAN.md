# Playable roster expansion to 40

Status: implemented and validated. The user-requested 40-slot test data is now
staged with the paired executable at `C:/Games/MUA2/`. Source/build receipt:
`evidence/windows-20261012/ROSTER-40-DELIVERY.json`; current installed state:
`evidence/windows-20261012/ROSTER-40-TEST-STAGING.json`.

## Scope and implementation

Support 40 selectable hero definitions with a four-character active team. Mods
supply the characters, packages and selection-stage artwork. Test aliases are installed locally at the user's explicit request; no proprietary
character assets are included in the source commit.

| Structure | Previous capacity | New capacity |
| --- | ---: | ---: |
| Character-info entries | 36 | 40 |
| Full character-stat records | 44 | 64 |
| Hero auxiliary records | 33 | 48 |
| Snapshot entries per bank | 33 | 48, with 64 KiB bank strides |
| Selection-location flags | 32 | 256 |
| Shared menu item map | 210 | 320 |
| Shared UI object pool | 210 | 320 |
| UI text bytes | 7,168 | 16,384 |
| External runtime hook budget | 64 | 128 |

The 350-entry general definition table, 13 NPC auxiliary records, four active team
members and 1,024 UI string handles retain their existing capacities. The complete
character-manager allocation is 271,264 bytes. The character-info singleton moves
to a 488-byte native-heap allocation; neighboring static storage is not borrowed.

The supported executable receives 693 guarded DOL edits, seven selection-menu REL
edits and 327 shared-menu REL edits. Full code/file hashes and expected instruction
words guard installation. Native navigation, map-tree balancing, object queues,
allocation bitmaps and destruction paths retain their algorithms with coordinated
storage and bounds. Native guest-heap allocations are used throughout.

## Persistence

The versioned `OMUA2R40` section stores hero records, snapshots and unlocks by name
within the existing save-stream boundary. It migrates the verified retail hero/NPC
ordering; malformed or unsupported unversioned custom orders are rejected instead
of guessing numeric identities. New saves require this build or a compatible newer
build; earlier releases cannot read the expanded section. User saves are preserved
when staging the executable.

## Mod data contract

- Declare at most 40 selectable heroes in `charinfo`, with corresponding herostat
  definitions and complete installed character packages/resources.
- Keep hero definitions together ahead of mission NPCs, as in the original
  herostat. Internal snapshot identity indices must fit the 48-entry hero domain;
  it is not a capacity for arbitrarily interleaving hundreds of NPC definitions.
- Give each selectable hero a distinct `menulocation`. It remains a byte-valued
  field. The tested roster uses sparse locations through 43.
- Author matching `padNN_focus`, `padNN_model` and `padNN_collision_model` objects
  in the team stage, and left/right/up/down neighbor references on collision
  objects. The runtime follows those authored neighbors; it does not invent a
  layout or automatically add stage objects.
- Rebuild the packaged copies as well as loose definitions. Character-specific
  talents, powerstyles and model/package dependencies must resolve by their real
  resource names. Raising the roster limit does not create missing resources.
- Keep the complete menu within the 320-object/item and 16,384-byte text budgets.
  Stage artwork and an aesthetically arranged 40-character stage are outside scope.

## Validation and evidence

The private fixture adds 13 distinctly named Captain America aliases to the 27
stock declarations, yielding 40 selectable definitions and 42 full-stat records
including default/team_character. It is a capacity fixture, not 13 delivered heroes.
Its stage adds logical pads and a deterministic navigation ring, reusing artwork.

Contained process-local controller input navigated the real title/profile/load
menus, walked to a private native SHIELD access point, selected Change Team and
replaced the first team member. No live menu state or world position was forced.
Every new position 31 through 43 and wrap from 43 back to 2 was observed. The last
entry, Roster40, entered the four-character team, returned to a visible world,
moved, attacked, opened Hero Details, saved, and survived a cold executable restart
and normal Load Game navigation. Native captures verified its name/model, complete
power list, world and HUD after reload.

The private early-Latveria access point required a native startup-script fade-in
on map reload, because that map ordinarily has no team-change access at this
stage. An earlier black-screen return reproduced without changing the team; it
was fixture setup, not accepted as a passing visual check. Talent member filenames
also needed complete aliasing; after correction the cold reload showed all four
Captain America powers under Roster40. Neither fixture change is shipped.

Retail-save migration into the expanded fixture and stock data has passed.
The scoped suite covers relocation/boundaries, nonoverlap, patch guards,
versioned serialization, malformed sections and retail order migration. Historical
preparation receipts remain in `evidence/windows-20261012/ROSTER-*.json`; they are
not final package receipts. Physical-controller, full-campaign, missing-resource
recovery, arbitrary mid-campaign mod changes, audio/FPS and four-port acceptance
are not claimed.
