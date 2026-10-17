# Mission-results Continue Game fix — 2026-10-17

User testing found the Latveria results screen still displaying Wii + and unable to advance.

The direct gamepad provider chose action 90 (MenuBack) for Start because the active-menu whitelist recognized hacking results but omitted mission results. The same whitelist gates the MENU_OK Xbox prompt, so the missing class affected both behavior and display.

The supported REL identifies CMenuLoading through its RTTI and vtable at 0x8118b9a8. Its inherited menu input handler checks MENU_OK (105) at 0x80fc92b0 and invokes the accept callback. Add that active menu class to the existing Start-accept policy. Existing executable/layout guards and active-menu ownership checks remain in place; no menu state or completion is forced.

Regression coverage exercises the mission-results menu through the direct gamepad action builder and prompt resolver, checks that Start emits 105 without 90, and checks release clears all actions. Existing tests retain title/profile/hacking handling and rejection of unrelated or invalid menu objects.

Windows Release build and five focused regressions passed. Broader checks passed 70/71 initially; codegen_compile failed because the test shell lacked MSVC include paths, then passed when rerun through the documented MSVC environment. The first link attempt failed because the test process held the executable open; the successful rebuild followed termination of that process. No compiler warnings were observed in the successful build.

The self-contained executable was staged to C:/Games/MUA2/OpenMUA2.exe at the user's request. SHA256: 851001afb70a13a7b11dd8d04ac14b0fcee42258f96c06c993c59b5c7487b3b2. Embedded package verification passed. All 400 game files and 55 save files remained hash-identical. The previous executable is retained outside the game folder under %LOCALAPPDATA%/OpenMUA2/staging-archive/pre-results-fix-20261017/.

The user requested stopping automated navigation and will test progression. Mission-results continuation and its rendered Xbox prompt are not yet verified in-game. The TODO blocker remains open pending that acceptance; the other four findings are unchanged.
