# Alchemy Batch Converter checkpoint — 2026-10-20

Built Windows x64 WPF executable with Roslyn 9.0.314 against .NET Framework 4.8.1.
Build completed without compiler warnings or errors. Blender 4.5.8 LTS performs
background conversion; pinned igb-blender 4672c8d source is embedded in the EXE.

Five Python regression tests passed, including six classification cases, invalid
platform/texture/size/maps combinations and source/existing-output protection.
Seven application integration cases passed: four-character Wii batch, rejection
of all four existing outputs, byte-preserving copy of an unreadable IGB, and
MUA PC / XML2 PC / PS2 / GameCube character exports. Input hashes stayed unchanged.
Cancellation stopped the contained background worker and returned cleanly.
A synthetic cube also passed static scene import/export with XML2 output.

All four Wii output SHA256 values match the previously visually game-tested Luke
costumes exactly:
- 90000: df2d7f9b197cb8d63fa6b43d0738a06e271456ec80846f0b0d1aa73b56088aa5
- 90001: e04f0451095585cafac974b34b607c9eed2b09b843999ffef086787ff4dccbd4
- 90002: 78ef64b7a326d1dbe7ed6255339c91e9f3f14793179879bc00d3879456a23de0
- 90003: 413a156727ef5c2217c61ad66996453713a0a5729f0d51b49bf76f96f37b9911

The actual WPF UI was rendered offscreen and inspected: readable paths/settings,
asset rows, conversion statuses, action controls and red/dark layout. First capture
incorrectly lost Window resource inheritance when detached; capture was corrected,
then inspected again. No desktop input or host capture tools were used.

Limits: no new game run, no interactive browse-dialog acceptance, no destination-game
validation of additional export targets, no universal scene metadata preservation.
Animations/UI/effects/unknown formats expose copy/skip only. Complex scenes and
known unsupported source formats are gated. See tools/alchemy_batch/README.md.

Private evidence: .local/AlchemyBatchConverter/checks/20261020-094248/results.json,
its four-model-batch.png, per-worker job logs, scene-output/cube.igb, and
.local/alchemy-batch/cancel-test.json. No proprietary models are tracked.
