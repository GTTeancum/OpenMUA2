# MUA1 alternate costumes - 2026-10-17

## Installed data

- Target: `C:/Games/MUA2/GameData/files/z/assetsfb.wad`.
- Input (unchanged): `C:/Games/MUA2/MUA1 assetsfb.wad`.
- 17 shared playable characters, 68 alternate models, 204 packages.
- Each keeps its MUA2 default, then the four MUA1 models in original herostat order as `skin_02` through `skin_05`.
- Includes the MUA1 default as an alternate; five slots does not imply five visually distinct designs where the two games reuse a costume.
- Press X on the active-characters page to cycle; existing runtime already supports full model IDs and default wrap.

| Character | MUA2 default | MUA1 donor IDs (slots 2-5) | Installed IDs |
|---|---|---|---|
| Blade | 0101 | 0101, 0102, 0103, 0104 | 9000, 9002, 9003, 9004 |
| CaptainAmerica | 25301 | 0701, 0702, 0703, 0704 | 9005, 9006, 9007, 9008 |
| Daredevil | 25001 | 0201, 0202, 0203, 0204 | 9009, 9010, 9011, 9012 |
| Deadpool | 23501 | 1701, 1702, 1703, 1704 | 9013, 9014, 9015, 9016 |
| HumanTorch | 24300 | 1301, 1302, 1303, 1304 | 9017, 9018, 9019, 9020 |
| Iceman | 24701 | 0801, 0803, 0802, 0804 | 9021, 9022, 9023, 9024 |
| InvisibleWoman | 2201 | 2201, 2202, 2203, 2204 | 9025, 9026, 9027, 9028 |
| Ironman | 25401 | 1502, 1505, 1503, 1504 | 9029, 9030, 9031, 9032 |
| LukeCage | 25101 | 15701, 15702, 15703, 15704 | 90000, 90001, 90002, 90003 |
| MrFantastic | 2001 | 2001, 2002, 2003, 2004 | 9033, 9034, 9035, 9036 |
| NickFury | 24801 | 2101, 2102, 2103, 2104 | 9037, 9038, 9039, 9040 |
| Spiderman | 24901 | 0901, 0902, 0903, 0904 | 9041, 9042, 9043, 9044 |
| Storm | 0401 | 0401, 0402, 0403, 0404 | 9045, 9046, 9047, 9048 |
| Thing | 0501 | 0501, 0502, 0503, 0504 | 9049, 9050, 9051, 9052 |
| Thor | 23601 | 1001, 1002, 1003, 1004 | 9053, 9054, 9055, 9056 |
| Warbird | 1401 | 1401, 1402, 1403, 1404 | 9057, 9058, 9059, 9060 |
| Wolverine | 0301 | 0301, 0302, 0303, 0304 | 9061, 9062, 9063, 9064 |

## Scope and checks

Only each donor actor IGB is imported. Same-width model/skeleton identity changes preserve IGB offsets. Unused IDs avoid collisions with resources already in MUA2. Each new package is cloned from the corresponding MUA2 default package; the combat-only package has no default actor model and stays unchanged. The MUA2 HUD portrait is retained byte-for-byte under the selected skin resource name. No MUA1 powers, animations, effects, sounds, talents, portraits or herostat gameplay properties are imported.

Validated every package member against its MUA2 template, preserving order, metadata and all non-model payload bytes. Checked all 68 donor identities and new model hashes, all 204 package paths, all three herostats by semantic round-trip, the entire output ZIP CRC, all 1,097 unchanged original archive entries, and all 14 paired Xbox UI manifest hashes. No embedded herostat copies were present in the source packages. Other installed files and saves were hash-compared before/after staging. Existing 40-slot test roster data is preserved.

At initial import, validation covered the packages only. The subsequent costume-cache fix verified all five Captain America costumes through two complete cycles and a return to gameplay using native captures. Other imported characters and full animation compatibility remain unverified; see the current handoff and evidence/windows-20261017/COSTUME-CACHE-FIX.json.

## Recovery

Rollback WAD: `C:\Users\smmel\AppData\Local\OpenMUA2\staging-archive\pre-mua1-costumes-20261017\assetsfb.wad`.
Installed SHA-256: `70dd5b180de5390dade7434054c83db9d52f6dedbc536715f566acd5d1e7fa91`.
Previous SHA-256: `8a0dd2633e5271608939a03d5aea05c768f872f5a311463058a6bdf2aabacb68`.

The initial data import did not change the executable. The subsequent costume-cache fix ships in the rebuilt executable documented in the current handoff. Candidate WAD was moved into place after validation and backup, leaving no duplicate candidate. Proprietary inputs, output and rollback remain outside Git. Compact private audit receipts are in the task workspace `work/mua1-costumes-report.json` and `work/mua1-costumes-staging.json`.
