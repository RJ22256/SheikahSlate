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

## Updating from 0.2.8-pc.x

Unzip `WSword-0.3.0-pc.1-windows.zip` over your `WSword` folder and replace the files. Then run `WSword.exe`:

1. **It finds your game files and saves.** It uses the game version you set up before (its `v190` or `v182` folder, the one `game_version.txt` names). No new copy of the game is made.
2. **It builds the game code once,** on its own. This takes a few minutes and uses every CPU core. The old one was built for WSword's previous engine.
3. **It frees space.** The old `romfs.bin` next to `romfs/` is removed (~15 GB): the game now reads `romfs/` directly.
4. **It starts the game.** Settings start fresh, in the game's own Options.

Left over from 0.2.8 and no longer used (you can delete them):
- next to `WSword.exe`: the `botwgen` and `lsfg` folders, `THIRD-PARTY.txt`, `launcher-background.png`, the `app-icon-*.png` pictures, and the Baloo2 / Inter font files;
- in the version folder: `sheikahslate.ini` and `extracted\botwgame.dll`.

If it asks for game files instead, choose the folder with your `exefs` and `romfs` again. This happens with 1.0.0 or 1.6.0 (Savage Wind plays 1.8.2 and 1.9.0), or if the old setup used them in place.

## A new install

1. Unzip it anywhere you can write to (not `Program Files`).
2. Run `WSword.exe`, choose the folder with your extracted game (`exefs` and `romfs`) and **Set up game files**. It
   copies them and builds the game code (about 6 minutes on a 12-thread CPU).
3. From then on `WSword.exe` goes straight into the game.

**WSword Configuration.exe** opens the configuration screen: Play, Settings, the game files (set up again, build the game code again, delete), and Quit.

Saves are in `save\` in the data folder: next to `WSword.exe`, or in its `v190` / `v182` folder when you came from 0.2.8. The log is `WSword.log` next to `WSword.exe`.

## In the game

- **Settings** are in the game's own Options: pause menu > System > Options, or Options on the title screen.
  - Video: frame rate (30, 60, 90, 120 or no limit, the game at the right speed), resolution and AMD FSR, VSync, draw distance, colour grading.
  - AMD FSR 3 frame generation.
  - Controls: keys, mouse, motion.
- **F10** shows the performance overlay: FPS, then the full one.
- **Esc** frees the mouse; a click takes it back.

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

## What's different from 0.2.8-pc.x

- **Much faster:** this is Savage Wind's engine.
- **Gone:**
  - the Glass Slate start screen and Settings;
  - Video > Mode;
  - frame generation with Lossless.dll. AMD FSR 3 frame generation is built in instead.
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
