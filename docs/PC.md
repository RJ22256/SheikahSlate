# WSword for PC (Windows 10/11)

From 0.3.0, WSword's PC build is built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp)** 0.4.1-alpha.4
by Revivedx and the Savage Wind contributors (MIT). Like WSword, Savage Wind is built on LAS Recomp, and it was made for
PCs:
- the game's geometry stays in video memory;
- the CPU and GPU work at the same time;
- the driver calls run on a thread of their own;
- the game copies about 13 times less data to the GPU each frame.

Where 0.2.8-pc.x ran at 15-20 fps with the CPU and GPU mostly idle, it plays at 60-120 fps.

The source is in `pc/` in this repository: Savage Wind as released, with WSword's changes listed in `pc/WSWORD.md`.
Its license is in the zip (`LICENSE.txt`), and its own README (`SavageWind-README.md`) lists the components inside
and their licenses.

> [!NOTE]
> **No game files are included.** You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2:
> the ExeFS and the RomFS with the update merged in, extracted from your own Switch.

## What you need

- Windows 10 or 11, 64-bit. A CPU with AVX2 runs fastest; one with only SSE4.2 gets a slower compatible build.
- A GPU with a **Vulkan 1.3** driver, from NVIDIA's, AMD's or Intel's site.
- 16 GB of RAM.
- About 20 GB free.

## Updating

**From 0.3.0-pc.1 or later:** unzip `WSword-0.3.4-pc.1-windows.zip` over your `WSword` folder and replace the files. Your game files, game code, saves and settings stay as they are.

**From 0.2.8-pc.x:** unzip it over your `WSword` folder and replace the files. Then run `WSword.exe`:

1. **It finds your game files and saves.** It uses the game version you set up before (its `v190` or `v182` folder, the one `game_version.txt` names). No new copy of the game is made.
2. **It builds the game code once,** on its own. This takes a few minutes and uses every CPU core. The old one was built for WSword's previous engine.
3. **It frees space.** The old `romfs.bin` next to `romfs/` is removed (~15 GB): the game now reads `romfs/` directly.
4. **It starts the game.** Settings start fresh, in the game's own Options.

Kept from 0.2.8: your `lsfg\Lossless.dll` (Lossless frame generation finds it there) and a background you chose (`look\background.png`).

Left over from 0.2.8 and no longer used (you can delete them):
- next to `WSword.exe`: the `botwgen` folder, `THIRD-PARTY.txt` and the Baloo2 font files;
- in the version folder: `sheikahslate.ini` and `extracted\botwgame.dll`.

If it asks for game files instead, choose the folder with your `exefs` and `romfs` again. This happens with 1.0.0 or 1.6.0 (Savage Wind plays 1.8.2 and 1.9.0), or if the old setup used them in place.

## A new install

1. Unzip it anywhere you can write to (not `Program Files`).
2. Run `WSword.exe`, choose the folder with your extracted game (`exefs` and `romfs`) and **Set up game files**. It
   copies them and builds the game code (about 6 minutes on a 12-thread CPU).
3. From then on `WSword.exe` goes straight into the game.

**WSword Configuration.exe** opens WSword's Glass Slate start screen:
- a panel on the left: PLAY, Saves, Settings, Game files (set up again, build the game code again, delete, DLC, shaders) and Quit, with the game files' state and your GPU under them;
- chips at the top right: the monitor's refresh rate, frame generation, the battery on a laptop;
- a card at the lower right: when you last saved, Resume, and the **modes** - Battery (30 fps, AMD FSR 1), Performance (60), Turbo (120) and Unlocked (no limit). Video > Mode in Settings and in the game's Options has them too.

**Settings** there are in the Glass Slate look: the pages down the left (Files, Video, Audio, Controls, Language, Mods, Saves), each option with its explanation on the right. **Files** has:
- the game files and the game font;
- **Look**: the start screen's background (a picture of your own, or the original) and WSword's icon (Sword, Slate, Midnight, Dawn, Ancient, Forest), on its window and in the taskbar;
- **Lossless Scaling**: choose your own `Lossless.dll` for Lossless frame generation;
- **NVIDIA DLSS**: choose your own `nvngx_dlss.dll` for DLSS upscaling (NVIDIA RTX graphics cards);
- **NVIDIA DLSS frame generation**: your own `nvngx_dlssg.dll`;
- **ReShade**: your own ReShade, which WSword loads itself (below).

Saves are in `save\` in the data folder: next to `WSword.exe`, or in its `v190` / `v182` folder when you came from 0.2.8. The log is `WSword.log` next to `WSword.exe`.

## In the game

- **F1** opens WSword's **Settings** over the game, in the Glass Slate look (on a controller: **Back + Start**, or the guide button). F1 again, or Back, closes it. Its pages: Video, Audio, Controls, Language, Mods and Quit.
  - Video: Mode, frame rate (30, 60, 90, 120 or no limit, the game at the right speed), resolution, the upscaler (AMD FSR or NVIDIA DLSS), VSync, draw distance, colour grading.
  - Frame generation: AMD FSR 3, or Lossless 2x / 3x / 4x (below).
  - Controls: keys, mouse, motion.
- The game's own Options (pause menu > System > Options, or Options on the title screen) has most of them too.
- **F10** shows the performance overlay: FPS, then the full one.
- **Esc** pauses or goes back; **F2** frees the mouse, a click takes it back.

| Action | Key | Action | Key |
|---|---|---|---|
| Move | W A S D | Camera | Mouse (or I J K L) |
| Walk (hold) | C | Crouch | Left Ctrl |
| Jump (X) | Space | Sprint / back (B) | Left Shift |
| Interact / confirm (A) | E | Attack (Y) | Left click |
| Shield / lock on (ZL) | Right click | Bow (ZR) | F |
| Rune (L) | Q | Throw (R) | R |
| Look (right stick press) | Middle click | Weapons / shields / runes / whistle | 1 / 2 / 3 / 4 |
| Inventory (Plus) | Tab | Map (Minus) | M |
| Tilt (hold, motion puzzles) | T + mouse | | |

Every key can be changed in Options. Any controller SDL knows works; one with a gyroscope also aims by motion.

## Lossless frame generation

Lossless Scaling's frame generation, with your own copy of its `Lossless.dll`. It isn't included with WSword: it's in Lossless Scaling's folder (Steam: Lossless Scaling > Manage > Browse local files).

1. In **WSword Configuration**: Settings > Files > Lossless Scaling > **Choose Lossless.dll**. WSword checks it and keeps a copy in `lsfg\Lossless.dll`.
2. Turn it on in Settings > Video > **Frame generation**, or in the game's Options > Video: **Lossless 2x**, **3x** or **4x** - one, two or three frames made between each two the game draws.
3. The **frame rate** counts the frames on screen, as with FSR 3: at 120, 2x draws 60, 3x 40, 4x 30. Pick your monitor's refresh rate and the multiplier your PC keeps up with.

- **Flow scale** (Settings > Video): the resolution motion is found at. Automatic follows Lossless Scaling's advice (100% at 1080p, 75% at 1440p, 50% at 4K). Lower is faster.
- **Performance mode**: lighter passes, a little less exact.
- The menus, the map and the title screen show the game's own frames (it draws at the full rate there).
- On NVIDIA (GTX 16 / RTX 20 and newer), the frames are presented by a thread of their own, so the game goes on with its next frame meanwhile. On other GPUs they are presented from the game's own thread.
- The frame generation code is lsfg-vk by PancakeTAS and contributors (GPL-3.0, `lsfg-vk-LICENSE.md` in the zip). WSword isn't made by or affiliated with Lossless Scaling's developers.

## NVIDIA DLSS

DLSS upscaling (Super Resolution) on **NVIDIA RTX** graphics cards (RTX 20 and newer), with your own copy of `nvngx_dlss.dll`. It isn't included with WSword: it comes with NVIDIA's DLSS SDK, and most games with DLSS have one in their folder.

1. In **WSword Configuration**: Settings > Files > NVIDIA DLSS > **Choose nvngx_dlss.dll**. WSword keeps a copy next to `WSword.exe` (putting it there yourself works too, or in a `dlss` folder there).
2. Settings > Video > **Upscaler** (or the game's Options > Video): **NVIDIA DLSS**, then its mode on the row below - Ultra Quality, Quality, Balanced, Performance or Ultra Performance. The further down, the smaller the game renders and the faster it is; DLSS fills in the detail.

- The game is drawn at the lower resolution and DLSS brings it up to the window's, using the frames before it. Its anti-aliasing replaces the game's own, so the picture is steadier too.
- **DLSS model** (Settings > Video, under Upscaler mode): which of NVIDIA's models runs.
  - **Automatic:** on RTX 40 and newer, NVIDIA's own choice for each mode; on RTX 20 and 30, the Transformer for every mode. NVIDIA's newest models, which recent `nvngx_dlss.dll` files use for Performance and Ultra Performance, cost those cards more than rendering smaller saves.
  - **CNN (fastest):** the older model, about 1 ms a frame at 1440p on an RTX 3060. A little softer, with more ghosting.
  - **Transformer:** sharper and steadier, about 2.4 ms there.
  - **Transformer 2:** NVIDIA's newest, made for RTX 40 and newer. On an RTX 3060 at 1440p it takes 5-7 ms.
- **What DLSS costs:** the performance overlay (F10, the full one) shows its model and GPU time a frame, as does Upscaler's description.
- Upscaler's description in Settings says why DLSS isn't offered, if it isn't (no RTX card, no `nvngx_dlss.dll`, an old driver) or why it stopped.
- DLSS speeds up a GPU that's the bottleneck. If **GPU %** in the overlay is well under 100 without DLSS, the CPU is what holds the frame rate back: DLSS can't add frames there, and the motion information it needs costs the CPU a little.
- `wsword_dlss.dll` in the zip is WSword's bridge to NVIDIA's DLSS SDK (`NVIDIA-DLSS-LICENSE.txt`). NVIDIA and DLSS are NVIDIA's trademarks; WSword isn't made by or affiliated with NVIDIA.

## NVIDIA DLSS frame generation

NVIDIA's frame generation on **NVIDIA RTX 40** graphics cards and newer: **2x** on RTX 40, and **3x** and **4x** on RTX 50. It uses your own copy of `nvngx_dlssg.dll`, which isn't included with WSword. It comes with NVIDIA's Streamline SDK, and most games with DLSS frame generation have one in their folder.

1. In **WSword Configuration**: Settings > Files > **NVIDIA DLSS frame generation** > **Choose nvngx_dlssg.dll**. WSword keeps a copy in the `streamline` folder next to `WSword.exe`.
2. Settings > Video > **Frame generation** (or the game's Options > Video): **NVIDIA DLSS 2x** (3x, 4x on RTX 50).
3. **Restart WSword.** NVIDIA's frame generation has to be in from the moment WSword starts.

- **The frame rate** counts the frames on screen, as with the other frame generation: at 120, 2x draws 60.
- **Windows:** Windows 10 version 2004 or newer, and **Hardware-accelerated GPU scheduling** on (Windows Settings > System > Display > Graphics > Change default graphics settings; then restart the PC).
- **VSync:** NVIDIA's frame generation has no VSync on Vulkan, so with it Video > VSync On and Adaptive work as Fast. With G-SYNC or FreeSync the display still follows the frames.
- **NVIDIA Reflex** comes with it, to keep the delay down.
- **HDR** is off while it's in (SDR).
- The menus, the map, the title screen and the F1 Settings show the game's own frames.
- The frame generation line in Settings > Video says what it does, or why it doesn't: no RTX 40, no `nvngx_dlssg.dll`, scheduling off, an old driver. So does `WSword.log` (its `[dlssg]` lines).
- To leave it out completely, choose another Frame generation and restart.
- The `streamline` folder in the zip holds NVIDIA's Streamline (its signed `sl.*.dll` files and licence). NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks; WSword isn't made by or affiliated with NVIDIA.

## ReShade

WSword loads your own ReShade itself, with no installer. ReShade isn't included with WSword.

1. Download ReShade from [reshade.me](https://reshade.me). Get the version **with full add-on support** if you use ReShade add-ons.
2. In **WSword Configuration**: Settings > Files > **ReShade** > **Choose ReShade**. Pick ReShade's setup program as you downloaded it (WSword takes `ReShade64.dll` out of it), or a `ReShade64.dll`. WSword keeps it in its `reshade` folder.
3. Settings > Video > **ReShade**: On. **Restart WSword.**
4. In the game, **Home** opens ReShade's menu.

- **Its files:** ReShade's settings (`ReShade.ini`), log, presets, shaders and add-ons are in the `reshade` folder (**Open the ReShade folder** in Settings > Files). Put shader packs in `reshade\reshade-shaders\Shaders` (and their textures in `reshade\reshade-shaders\Textures`), and add-ons (`.addon64`) in `reshade\`.
- **If you installed ReShade for WSword with ReShade's installer before:** WSword uses the `ReShade.ini` it left next to `WSword.exe`, so your presets and shaders stay where they are. You can uninstall it from WSword with the installer, so it isn't loaded twice.
- **Depth:** Settings > Video > **ReShade depth**.
  - **The game's (WSword)** (the default): WSword gives ReShade's effects the game's own 3D depth, lined up with the picture. It's set up for ReShade's default depth settings: leave `RESHADE_DEPTH_INPUT_IS_REVERSED`, `IS_UPSIDE_DOWN`, `IS_LOGARITHMIC` and `RESHADE_DEPTH_LINEARIZATION_FAR_PLANE` as they are. The linear depth is the distance in metres / 1000, so everything past 1 km counts as far away. ReShade's Generic Depth add-on is turned off then (it costs CPU time), and its Add-ons page lists "WSword game depth" instead.
  - **ReShade's own detection:** ReShade's Generic Depth add-on looks for the depth itself.
  - Where the game draws no 3D scene (the title screen, some menus), the depth is empty (far away).
- ReShade's effects cover the whole window, WSword's own menus and the performance overlay too.
- `WSword.log` says what happened (its `[reshade]` lines), and ReShade's own `ReShade.log` is in the `reshade` folder.
- To leave it out, turn Settings > Video > ReShade off and restart.
- ReShade is crosire's ([reshade.me](https://reshade.me)). WSword isn't made by or affiliated with ReShade. The bridge's ReShade part uses ReShade's add-on API headers (BSD-3-Clause, `ReShade-API-LICENSE.md`).

## Mods

Mods for the Switch version, laid out as for Ryujinx, Yuzu or Eden, work as they are: each is a folder with a `romfs` folder inside. They must be made for your game version (1.9.0 or 1.8.2).

1. Settings > **Mods** (on the start screen, or F1 in the game) > **Import a mod folder**, and pick the mod's own folder (extracted from its ZIP / RAR / 7z, the one with `romfs` inside). WSword copies it into the `mods` folder in its data folder. Or put mod folders there yourself: **Open the mods folder**.
2. Every mod there is on; the Mods page turns each one on or off. Its description is its own note (a `.txt` in its folder, such as README.txt).
3. A change applies when the game starts again (in the game: Quit > Return to the start screen, then Play).

- **Order:** where two mods have the same file, the one first in the list (by name) wins.
- **DLC files:** a mod's `romfs\aoc\0010`, or a mod folder named `01007EF00011F001` (as BCML and Relics of the Past ship them), whose `romfs` is the DLC's own. They need the DLC set up (Settings > Files); the Mods page says so if it isn't.
- Mods change files only: mods that patch the game's code (exefs, `.pchtxt` / `.ips`) don't apply.
- `WSword.log` lists the mods in use (its `[mods]` lines).

## What's different from 0.2.8-pc.x

- **Much faster:** this is Savage Wind's engine.
- **Back from 0.2.8 (0.3.1-pc.1):** the Glass Slate start screen and Settings, Video > Mode, the background and icon choices, and frame generation with your own Lossless.dll. AMD FSR 3 frame generation is there too.
- **Back from 0.2.8 (0.3.2-pc.1):** Settings over the game on F1.
- **New in 0.3.2-pc.1:** NVIDIA DLSS upscaling, with your own `nvngx_dlss.dll`; 0.3.3-pc.1 made it faster and added Video > DLSS model.
- **New in 0.3.4-pc.1:** NVIDIA DLSS frame generation, with your own `nvngx_dlssg.dll`.
- **Back from 0.2.8 (0.3.5-pc.1):** mods, on Settings > Mods.
- **New in 0.3.6-pc.1:** your own ReShade, loaded by WSword (Settings > Video > ReShade), with the game's depth for its effects.
- **Game versions:** 1.8.2 and 1.9.0. 1.0.0 and 1.6.0 aren't supported.
- **New:**
  - Savage Wind's settings in the game's own Options;
  - the DLC;
  - draw distance, colour grading and HDR10;
  - Wii U / Cemu save import (Settings > Saves).

## If something goes wrong

- **The log:** `WSword.log` next to `WSword.exe`. The run before is kept as `WSword.prev.log`. Send it with a report.
- **"Vulkan 1.3" errors:** update the GPU's driver from its maker's site.
- **The game code build fails:** it needs a few GB free, and an antivirus that lets `zig\zig.exe` and `swgen\swgen.exe` run. The reason is on the game files page and in the log.
