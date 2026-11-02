# WSword PC on Savage Wind Recomp

This folder is **Savage Wind Recomp 0.4.1-alpha.4** (https://github.com/Revivedx/SavageWindRecomp, MIT, by Revivedx
and the Savage Wind contributors, built on Peiikai's LAS Recomp), imported unchanged in its own commit. It is the
base of WSword's PC build from 0.3.0-pc.1. `LICENSE` is Savage Wind's and ships in the zip as `LICENSE.txt`.

## WSword's changes

**Names.** The release's files are WSword's:
- `WSword.exe`, its import library `WSword.dll.a` (the game code library links against it), and `WSword Configuration.exe`;
- `WSword.log` and `WSword.prev.log`, and the settings file `WSword.ini`;
- the zip `WSword-<version>-windows.zip`, with a top folder `WSword/`;
- WSword's icon (`docs/icon/wsword.ico`, `wsword_1024.png`).

The name shown in the windows, menus, messages and translations is WSword. Comments still say Savage Wind's in a few places.

The `SAVAGEWIND_*` environment variables, `swgame.dll`, `swgen/` and the Linux and macOS names are Savage Wind's.

**Version.** The version is `SAVAGEWIND_VERSION` in `runtime/ui/launcher.mm`: `0.3.0-pc.1`. `SW_BASE_VERSION` is the Savage Wind release it is built on, shown after the version on the configuration screen. The log's first line names both.

**An older WSword's folder** (`runtime/paths.c` `wsword_older_root`):
- **What it is.** 0.2.8-pc.x kept each game version in a folder of its own (`v182`, `v190`, with `game_version.txt` naming the one in use), on LAS Recomp's layout.
- **When.** Unzipped over such a folder, where the exe's folder has no `extracted/exefs` of its own.
- **What happens.** That version's folder becomes the data folder, with its game files (`extracted/exefs`, `extracted/romfs`) and `save/`. Savage Wind's `remove_old_image` then removes the old `romfs.bin`.

**The game code built at once** (`runtime/ui/launcher.mm`):
- **When.** The game files are set up, but the game code is missing or was built for another version (an older WSword's folder, or an update).
- **What happens.** The build starts by itself, as Set up game files does with no folder chosen.

**Packaging** (`Makefile` dist): `../docs/PC.md` ships as `WSword-PC-guide.md`, and Savage Wind's README as `SavageWind-README.md`.

**FFmpeg** (`tools/build_deps_windows.sh`): WSword's version, the same FFmpeg. It is configured for a neutral prefix, so no path of the build machine ends up in the DLLs.
