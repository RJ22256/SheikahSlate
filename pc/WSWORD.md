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

**Version.** The version is `SAVAGEWIND_VERSION` in `runtime/ui/launcher.mm`: `0.3.3-pc.1`. `SW_BASE_VERSION` is the Savage Wind release it is built on, shown after the version on the configuration screen. The log's first line names both.

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
- Settings > Files (`prelaunch_tab`): Look - the background (`look/background.png` next to the exe; `runtime/ui/splash.mm` shows it, else the shipped `launcher-background.png`) and the icon (`settings.h` `app_icon`, `runtime/paths.c` `rt_window_icon`: the window's and the taskbar's) - and Lossless Scaling (the player's own `Lossless.dll`, checked by `lsfg_check_dll`, copied to `lsfg/Lossless.dll` next to the exe) - and NVIDIA DLSS (the player's own `nvngx_dlss.dll`, copied next to the exe).
- Modes (`runtime/settings.c` `settings_apply_mode`): Battery 30 fps with AMD FSR 1, Performance 60, Turbo 120, Unlocked - on the start screen's card, Video > Mode and the game's Options > Video.

**Lossless frame generation** (0.3.1-pc.1, from WSword 0.2.8): `third_party/lsfg-vk` (GPL-3.0, `README.WSword.md` there), `runtime/nvn/vulkan/lsfg.cpp` / `lsfg.h`, and in `device.c` `lsfg_prepare` and `present_locked`:
- **Settings** (`settings.h`): `lsfg` (0 off, 1-3 frames made: Video > Frame generation's Lossless 2x / 3x / 4x - AMD FSR 3 is off then), `lsfg_flow` (0 Automatic), `lsfg_perf`.
- **Each game frame:** its image copied into the backend's source image, that submission signalling the backend's timeline (`exec.c` `vk_next_submit_timeline`; `record.c` carries a second wait and a third signal); once the recorder has made it, the backend's passes go on the queue; each generated frame is drawn as the game's are (`vk_present_draw_view`: the same post passes) by a present that waits for its own value.
- **Pacing:** the frame rate counts the frames on screen (`hle/framepace.c`: the game's frame time x the presents per frame, `g_fg_active`); each group of presents is spaced evenly over the game's frame time (`present_wait_turn`'s pos / group). While groups are presented they go to the present thread on a second queue (`present_threaded`), made where it is safe (not NVIDIA before Turing, #36), so the game's thread doesn't wait; the swapchain has an image more for each further frame of a group.
- Paused in the menus, the map, the title screen and the F1 menu (the game draws at the full rate there), as FSR 3's.

**Settings over the game** (0.3.2-pc.1, from WSword 0.2.8): `runtime/ui/menu.mm` `overlay_enabled` is on (Savage Wind had it off: its settings only in the game's own Options, which still has them). F1, Back + Start or the guide button opens Settings in the Glass Slate look - Video, Audio, Controls, Language, Quit - and BACK, B or Esc closes it. `SAVAGEWIND_OVERLAY=0` turns it off. The game's Options > Video keeps Savage Wind's 19 rows (the game's screen has room for no more): WSword's Frame generation choice (Off, AMD FSR 3, Lossless 2x / 3x / 4x) takes the FSR 3 toggle's place; Mode and Lossless's own options are in the F1 Settings.

**NVIDIA DLSS** (0.3.2-pc.1): Video > Upscaler's NVIDIA DLSS (`settings.h` `UPSCALE_DLSS`, `output_scale.c` `rt_dlss`), on NVIDIA RTX GPUs with the player's own `nvngx_dlss.dll`.
- **The bridge:** NVIDIA's NGX library is MSVC's, which this MinGW build can't link, so `tools/dlss_bridge/wsword_dlss.cpp` is built with MSVC into `wsword_dlss.dll` (by `build-windows.yml`, against NVIDIA's DLSS SDK from https://github.com/NVIDIA/DLSS) with a plain C interface (`runtime/nvn/vulkan/dlss_bridge.h`). `runtime/nvn/vulkan/dlss.c` loads it.
- **The device** (`device.c`): on an RTX GPU, the instance and device extensions NGX names are enabled (with buffer device address), if the driver has all of them.
- **Each frame** (`exec.c` `dlss_scene`, where FSR 3's upscale is): the scene at the render size, its depth (`fsr3_depth.frag`) and motion vectors (`dlss_mv.frag`, RG16F), FSR 3's jitter (in at least 8 phases) and motion-vector scale, into a storage image at the output size; Sharpening's CAS after it. Anti-aliasing (SMAA, TAA) is off with it, as with FSR 3. A second composite of the same scene in a frame reuses the first's output.
- **On the recorder thread** (0.3.3-pc.1): NGX records into the frame's command buffer from a `record.c` `rec_call` packet, in order with the commands around it, so the translating thread doesn't wait for the recorder (`rec_drain`) every frame. It drains only to start NGX and when the feature is made again (other sizes, mode or model: `wsdlss_release_feature`, the device idle first). A failed frame shows the scene blitted plainly into DLSS's output.
- **The model** (0.3.3-pc.1, Video > DLSS model, `settings.h` `dlss_model`, `dlss.c` `dlss_preset`): NGX's render preset for every mode. Automatic is NVIDIA's default on RTX 40 and newer (`device.c` `renderer_gpu_rtx40`: PCI device id 0x2600 and up) and the transformer K on RTX 20 / 30, where a 310.5+ `nvngx_dlss.dll`'s defaults for Performance (M) and Ultra Performance (L) cost 2-5x K's. CNN (E), Transformer (K) and Transformer 2 (M; L for Ultra Performance) pick one.
- **Its GPU time:** a pair of timestamps around it each frame, read back 8 frames later without waiting; the average in the performance overlay, Upscaler's description and the log every 1800 frames.
- **Failures:** NGX not starting, or DLSS failing three frames in a row, turns DLSS off for the run, with the reason in Upscaler's description (`dlss_status`) and the log. `SAVAGEWIND_NO_DLSS=1` leaves the bridge alone. NGX's own messages: the first 200 in the log (all with `SAVAGEWIND_DLSS_LOG=1`).
- **`dlss_available`** runs many times a frame (with DLSS on, at every camera the game makes: `output_scale.c` `rt_jitter_on`), so the `nvngx_dlss.dll` lookup behind it is cached for half a second.

**Packaging** (`Makefile` dist): the Inter fonts and their OFL, `launcher-background.png` and the icons (`assets/`), lsfg-vk's licence and README, `wsword_dlss.dll` and the DLSS SDK's licence (`NVIDIA-DLSS-LICENSE.txt`) when CI built them. No `nvngx*` file ships. `../docs/PC.md` ships as `WSword-PC-guide.md`, and Savage Wind's README as `SavageWind-README.md`.

**FFmpeg** (`tools/build_deps_windows.sh`): WSword's version, the same FFmpeg. It is configured for a neutral prefix, so no path of the build machine ends up in the DLLs.
