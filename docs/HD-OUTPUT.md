# HD output

OpenMUA2's PC Options resolution cycle matches OpenXML1xbox: 1280x720,
1920x1080, 2560x1440 and 3840x2160, wrapping back to 720p.
Apply saves the choice; restart the game to apply the output size.

Settings are stored in the active user directory's Config/OpenMUA2Controls.ini:

```ini
Version=1
OutputWidth=2560
OutputHeight=1440
```

For 4K, use OutputWidth=3840 and OutputHeight=2160. Preserve the other settings
when editing an existing file. Zero dimensions preserve the existing startup
configuration; unsupported width/height pairs are rejected.

The saved dimensions set the Windows render client area, excluding window borders
and the menu bar. Internal rendering uses automatic integral EFB scaling unless
an explicit launch override is supplied. Internal dimensions therefore need not
exactly equal output dimensions. Fullscreen output also depends on the desktop.

## Verification boundary

The native Advanced menu has displayed a selected 3840x2160 mode. Automated tests
cover all four choices, persistence, wrapping and invalid combinations. Startup
configuration and Windows client sizing are implemented. Actual 1440p/4K rendering,
fullscreen behavior and menu layout at those sizes still need visual validation.
These options remain part of the unfinished PC menu work; the installed game has
not been updated with this preview.
