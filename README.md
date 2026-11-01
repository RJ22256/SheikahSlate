# Sheikah Slate

**English** | [Português](README.pt-BR.md)

*Breath of the Wild, recompiled, on the slate in your hands.*

Sheikah Slate is a port of [BOTWRecompAndroid](https://github.com/mamcjr/BOTWRecompAndroid) (0.2.8-beta) made
specifically for the **Samsung Galaxy Tab S10 Ultra** (SM-X920 / SM-X926B / SM-X920N / SM-X926N). Like upstream, it is
a static recompilation of *The Legend of Zelda: Breath of the Wild* (Nintendo Switch, versions 1.0.0, 1.6.0, 1.8.2 and
1.9.0): not an emulator. The game's ARM64 code becomes C compiled for the device (or, in upstream's experimental Native
Mode, runs as it is), and its NVN graphics are translated to Vulkan. Everything upstream's 0.2.8 has is here - saves
import and export (a .zip, or an emulator's save folder), the DLC, game files used in place, Sync lag, 60 fps
cutscenes, eleven menu languages - with the tablet's own work on top.

It installs as its own app (`com.sheikahslate.tab`), so it can sit next to upstream's.

> [!WARNING]
> **Beta.** The tablet work is written and checked from the source and tuned from logs sent from a Tab S10 Ultra
> (29-30 fps in the world at 0.1.9-slate.8); this build itself hasn't run on one yet. Please report what you see:
> [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md) has a test checklist.

> [!NOTE]
> **No game files are included.** You need your own legally obtained dump of *Breath of the Wild* 1.0.0, 1.6.0,
> 1.8.2 or 1.9.0 (`exefs` and `romfs`, extracted). Instructions on how to obtain it will not be provided.

## What's different from upstream

| | Upstream (made on a 20:9 Adreno phone) | Sheikah Slate on the Tab S10 Ultra |
| --- | --- | --- |
| Orientation | `sensorLandscape`, which Android 16 ignores on large screens for apps targeting API 36 | Declared a game (`appCategory="game"`): the landscape lock holds on One UI 8+ |
| 16:10 screen, 1.8.2 "Unlocked" | Final image reshaped to 16:10 while the main camera stayed 16:9: the world stretched ~11% tall | 16:9 kept, fitted to the width, 92 px bars above and below (the camera notch sits in one) |
| Camera notch | Not handled (on Android 14, the window stopped short of it) | Window drawn under it on every version; on-screen controls move out from under it, also with the tablet turned around |
| On-screen controls | Sized as % of screen height: on a 14.6" panel, 3 cm buttons 5 cm from the corners | Sized in millimetres on big screens (~14 mm buttons, near the edges); phones unchanged |
| Refresh rate | Left to One UI | Video > Display refresh rate: Highest (120 Hz), 60 Hz or System |
| Graphics settings | Made on an Adreno phone | Video > Mode, from the tablet's own logs: **Battery** (30 fps at 60 Hz, lowest resolution held), **Performance** (the default: 30 fps paced to the 120 Hz screen, 900p scene steadied, FSR 1 to 1080p), **Turbo** (40 fps, 3 refreshes of 120 Hz a frame), **Turbo 60** (60 fps, the resolution lowered as far as needed) |
| Frame rate 40 / 60 | (0.2.8-slate.9/10: the frame pacer stayed off after the start screen, so every game ran at 30) | The frame pacer on; the game's resolution follows the GPU's real time, down only as far as 40 or 60 needs |
| Start screen | Title and menu over the artwork | The Glass Slate screen: a glass panel (PLAY, Saves, Mods, Settings, Quit), live chips, a card with the last save and the four modes a tap away |
| Look | The bundled background and icon | Settings > Look: your own start screen background, and the app's real icon - the player's picture by default, five more to choose |
| Frame pacing | A frame goes up when the GPU finishes it: 3, 4 or 5 refreshes at 30 fps | Video > Even frame pacing: each frame aimed at a refresh of its own, 4 apart (VK_GOOGLE_display_timing) |
| Decals on Mali | The pass ends before each decal that reads the G-buffer (a store and reload of every target on a tiler) | Read from the GPU's tile memory (VK_EXT_shader_tile_image): no pass breaks |
| Render passes on Mali | Each of ~300 passes a frame waits for the whole GPU | Video > Overlap render passes: geometry overlaps the pass before |
| Game's resolution changes | Every frame's GPU time: a hitch swung it several steps | Video > Game's resolution changes: Steadied (the median of 9 frames), Hold sharp, Hold low |
| Game text on Mali | 64-bit shader address math by default: letters missing or wrong on the Tab | 32-bit shader math automatic on Arm GPUs |
| Texture streaming | BC textures decoded on one thread | Big levels decoded on 3 worker threads too (same output, tested) |
| Performance capture | GPU time per pass | Also the CPU per thread with callers, memory by kind, the heaviest shaders, why passes break (Video > Capture a performance profile) |
| Driver error text | "Install a Turnip driver" (Adreno only) | Says Mali runs the system driver only |
| Name, icon | BOTWRecompAndroid, Triforce | Sheikah Slate, an original slate-and-mountains icon |

Details, reasoning and the files involved: [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md).

## Installing

1. Build the APK (below) and install it on the tablet. You need about 30 GB free per game version (or use the game
   files in place: Game files > Use them in place).
2. Open **Sheikah Slate**, go to **Set up game files** and choose the folder with your `exefs` and `romfs`. The app
   recompiles the game on the tablet (a long wait, the first time and after an update that changes the game code -
   0.2.8-slate.9 was one; 0.2.8-slate.10 to .13 aren't). It keeps building with the screen off.
3. Saves: start screen > Settings > **Saves** - Import saves (a .zip, one exported here or any holding 0, 1, 2... and
   option.sav), Import from folder (an emulator's save folder), Export saves.
4. The Tab's Mali GPU runs Samsung's own Vulkan driver; Turnip driver packages are for Adreno GPUs and won't load.
   Keep the tablet's software up to date.

## Building (developers)

With the Android SDK, NDK 30.0.16248370 and CMake 3.30.4:

```bash
git clone --recursive https://github.com/bylaws/libadrenotools third_party/libadrenotools  # (a submodule: source zips leave it out)
android/deps/build_deps.sh                      # SDL3, shaderc, Vulkan headers, volk, VMA (pinned, from GitHub)
android/deps/build_ffmpeg.sh                    # optional: the game's movies and the start screen's background
android/deps/build_gen_deps.sh                  # the on-device game code builder (Python, zig, capstone, unicorn)
cd android/sheikahslate && ./gradlew assembleRelease   # (or assembleDebug)
```

Release builds are signed with `~/.sheikahslate/keystore.properties` when it exists, else with the debug key.
Desktop-side tests of the tablet logic: `clang -Iruntime -o build/test_tablet_layout tools/test_tablet_layout.c -lm &&
build/test_tablet_layout`. Upstream's developer flow (game code built on a PC, pushed over adb) still works:
`android/build_gamelib.sh`, `android/push_game.sh`, `android/run_boot.sh` now target `com.sheikahslate.tab`.

## Credits

Sheikah Slate is upstream's work with a tablet port on top; every credit below is theirs to keep.

- [mamcjr](https://github.com/mamcjr)'s [BOTWRecompAndroid](https://github.com/mamcjr/BOTWRecompAndroid), the project
  this is ported from, and [LASRecompAndroid](https://github.com/mamcjr/LASRecompAndroid) before it.
- [peiikai](https://github.com/peiikai) for [LAS Recomp](https://github.com/peiikai/LASRecomp), the recompiler and
  runtime (MIT, see [LICENSE](LICENSE)).
- [Revivedx](https://github.com/Revivedx) for [SavageWindRecomp](https://github.com/Revivedx/SavageWindRecomp) (MIT);
  [Nycolazs](https://github.com/Nycolazs) for LASRecomp-Android-Plus (MIT); the
  [zeldaret/botw](https://github.com/zeldaret/botw) decompilation and [open-ead/sead](https://github.com/open-ead/sead).
- Upscaling and anti-aliasing: AMD FidelityFX FSR 1 (MIT), Qualcomm Snapdragon GSR (BSD-3-Clause), SMAA (MIT).
- Libraries: SDL (zlib), Dear ImGui (MIT), libadrenotools (BSD-2-Clause), volk (MIT), Vulkan Memory Allocator (MIT),
  shaderc (Apache 2.0), FFmpeg (LGPL), LZ4 (BSD-2-Clause), Capstone (BSD-3-Clause), Unicorn (GPLv2), Python (PSF) and
  Zig (MIT). Fonts: Baloo 2 (OFL), DejaVu Sans. The Android robot is reproduced or modified from work created and
  shared by Google (CC BY 3.0).
- Upstream's full README, with its own notes: [docs/upstream/README.md](docs/upstream/README.md).
- Like upstream, this port was made with AI (Claude, by Anthropic).

The port's changes are released under the same MIT license as the project ([LICENSE](LICENSE)).

*The Legend of Zelda*, *Breath of the Wild* and *Sheikah* are trademarks of Nintendo. *Galaxy* and *Galaxy Tab* are
trademarks of Samsung. This project is not affiliated with or endorsed by Nintendo or Samsung.
