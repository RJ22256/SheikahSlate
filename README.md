# Sheikah Slate

**English** | [Português](README.pt-BR.md)

*Breath of the Wild, recompiled, on the slate in your hands.*

Sheikah Slate is a port of [BOTWRecompAndroid](https://github.com/mamcjr/BOTWRecompAndroid) (0.1.9-beta) made
specifically for the **Samsung Galaxy Tab S10 Ultra** (SM-X920 / SM-X926B / SM-X920N / SM-X926N). Like upstream, it is
a static recompilation of *The Legend of Zelda: Breath of the Wild* (Nintendo Switch, versions 1.0.0, 1.6.0 and 1.8.2):
not an emulator. The game's ARM64 code becomes C compiled for the device, and its NVN graphics are translated to Vulkan.

It installs as its own app (`com.sheikahslate.tab`), so it can sit next to upstream's.

> [!WARNING]
> **Untested on the hardware it targets.** The changes below were written and checked from the source (compile checks
> and unit tests on a desktop), not on a Tab S10 Ultra. Upstream was tested on one phone with an Adreno GPU; the Tab's
> Immortalis-G720 is a Mali GPU and takes the engine's slower, compatible paths. Expect bugs, and please report
> what you see: [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md) has a test checklist.

> [!NOTE]
> **No game files are included.** You need your own legally obtained dump of *Breath of the Wild* 1.0.0, 1.6.0 or
> 1.8.2 (`exefs` and `romfs`, extracted). Instructions on how to obtain it will not be provided.

## What's different from upstream

| | Upstream (made on a 20:9 Adreno phone) | Sheikah Slate on the Tab S10 Ultra |
| --- | --- | --- |
| Orientation | `sensorLandscape`, which Android 16 ignores on large screens for apps targeting API 36 | Declared a game (`appCategory="game"`): the landscape lock holds on One UI 8+ |
| 16:10 screen, 1.8.2 "Unlocked" | Final image reshaped to 16:10 while the main camera stayed 16:9: the world stretched ~11% tall | 16:9 kept, fitted to the width, 92 px bars above and below (the camera notch sits in one) |
| Camera notch | Not handled (on Android 14, the window stopped short of it) | Window drawn under it on every version; on-screen controls move out from under it, also with the tablet turned around |
| On-screen controls | Sized as % of screen height: on a 14.6" panel, 3 cm buttons 5 cm from the corners | Sized in millimetres on big screens (~14 mm buttons, near the edges); phones unchanged |
| Refresh rate | Left to One UI | Video > Display refresh rate: Highest (120 Hz), 60 Hz or System |
| Mali GPU memory path | The guest heap import, never exercised on Android; a refusal left shaders reading zeros for the whole run | Probed at start; a refusal (now or in an earlier run) takes the CPU-copy path from the beginning |
| Game text on Mali | 64-bit shader address math (Turnip-tested): letters missing or wrong on the Tab | 32-bit shader math automatic on Arm GPUs (upstream's own advice for Mali) |
| Texture streaming | BC textures decoded on one thread, the one recording the frame | Big levels decoded on 3 worker threads too (same output, tested) |
| Decal in-place reads | Turnip only, even when set to On | On works on any GPU (experiment on Mali; Automatic stays off) |
| Driver error text | "Install a Turnip driver" (Adreno only) | Says Mali runs the system driver only |
| Name, icon | BOTWRecompAndroid, Triforce | Sheikah Slate, an original slate-and-mountains icon |

Details, reasoning and the files involved: [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md).

## Installing

1. Build the APK (below) and install it on the tablet. You need about 30 GB free per game version.
2. Open **Sheikah Slate**, go to **Set up game files** and choose the folder with your `exefs` and `romfs`. The app
   copies what it needs and recompiles the game on the tablet (a long wait, the first time).
3. The Tab's Mali GPU runs Samsung's own Vulkan driver; Turnip driver packages are for Adreno GPUs and won't load.
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
