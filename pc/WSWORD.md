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

**Version.** The version is `SAVAGEWIND_VERSION` in `runtime/ui/launcher.mm`: `0.3.1-pc.1`. `SW_BASE_VERSION` is the Savage Wind release it is built on, shown after the version on the configuration screen. The log's first line names both.

**An older WSword's folder** (`runtime/paths.c` `wsword_older_root`):
- **What it is.** 0.2.8-pc.x kept each game version in a folder of its own (`v182`, `v190`, with `game_version.txt` naming the one in use), on LAS Recomp's layout.
- **When.** Unzipped over such a folder, where the exe's folder has no `extracted/exefs` of its own.
- **What happens.** That version's folder becomes the data folder, with its game files (`extracted/exefs`, `extracted/romfs`) and `save/`. Savage Wind's `remove_old_image` then removes the old `romfs.bin`.

**The game code built at once** (`runtime/ui/launcher.mm`):
- **When.** The game files are set up, but the game code is missing or was built for another version (an older WSword's folder, or an update).
- **What happens.** The build starts by itself, as Set up game files does with no folder chosen.

**The Glass Slate look** (0.3.1-pc.1, from WSword 0.2.8):
- `runtime/ui/launcher.mm` `glass_main`: the start screen - a glass panel (the icon, the versions, PLAY, Saves, Settings, Game files, Quit, the game files' state and the GPU), chips (refresh rate, frame generation, battery) and a card (the last save, Resume, the modes). Savage Wind's Settings and game files pages behind it, in the same look.
- `runtime/ui/menu.mm` `tabs_body`: Settings as a rail of pages (Files, Video, Audio, Controls, Language, Saves, Quit in game) with BACK, the open page under its title; `style_init`'s navy glass and cyan; Inter (OFL, `third_party/fonts/Inter`) for the menus.
- `runtime/ui/option_rows.mm`: the rows as glass tiles, the selected row's name and description in a card.
- Settings > Files (`prelaunch_tab`): Look - the background (`look/background.png` next to the exe; `runtime/ui/splash.mm` shows it, else the shipped `launcher-background.png`) and the icon (`settings.h` `app_icon`, `runtime/paths.c` `rt_window_icon`: the window's and the taskbar's) - and Lossless Scaling (the player's own `Lossless.dll`, checked by `lsfg_check_dll`, copied to `lsfg/Lossless.dll` next to the exe).
- Modes (`runtime/settings.c` `settings_apply_mode`): Battery 30 fps with AMD FSR 1, Performance 60, Turbo 120, Unlocked - on the start screen's card, Video > Mode and the game's Options > Video.

**Lossless frame generation** (0.3.1-pc.1, from WSword 0.2.8): `third_party/lsfg-vk` (GPL-3.0, `README.WSword.md` there), `runtime/nvn/vulkan/lsfg.cpp` / `lsfg.h`, and in `device.c` `lsfg_prepare` and `present_locked`:
- **Settings** (`settings.h`): `lsfg` (0 off, 1-3 frames made: Video > Frame generation's Lossless 2x / 3x / 4x - AMD FSR 3 is off then), `lsfg_flow` (0 Automatic), `lsfg_perf`.
- **Each game frame:** its image copied into the backend's source image, that submission signalling the backend's timeline (`exec.c` `vk_next_submit_timeline`; `record.c` carries a second wait and a third signal); once the recorder has made it, the backend's passes go on the queue; each generated frame is drawn as the game's are (`vk_present_draw_view`: the same post passes) by a present that waits for its own value.
- **Pacing:** the frame rate counts the frames on screen (`hle/framepace.c`: the game's frame time x the presents per frame, `g_fg_active`); each group of presents is spaced evenly over the game's frame time (`present_wait_turn`'s pos / group). While groups are presented they go to the present thread on a second queue (`present_threaded`), made where it is safe (not NVIDIA before Turing, #36), so the game's thread doesn't wait; the swapchain has an image more for each further frame of a group.
- Paused in the menus, the map, the title screen and the F1 menu (the game draws at the full rate there), as FSR 3's.

**Packaging** (`Makefile` dist): the Inter fonts and their OFL, `launcher-background.png` and the icons (`assets/`), lsfg-vk's licence and README. `../docs/PC.md` ships as `WSword-PC-guide.md`, and Savage Wind's README as `SavageWind-README.md`.

**FFmpeg** (`tools/build_deps_windows.sh`): WSword's version, the same FFmpeg. It is configured for a neutral prefix, so no path of the build machine ends up in the DLLs.
