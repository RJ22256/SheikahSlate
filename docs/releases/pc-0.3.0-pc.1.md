**WSword for PC (Windows 10/11, 64-bit)**, now built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** by Revivedx and the Savage Wind contributors (MIT).

Savage Wind is built on LAS Recomp, as WSword is, and it was made for PCs:
- the game's geometry stays in video memory instead of being read over PCIe on every draw;
- the CPU and GPU work at the same time;
- the driver calls run on a thread of their own;
- the game copies about 13 times less data to the GPU each frame.

Where WSword 0.2.8-pc.x ran at 15-20 fps with the CPU and GPU mostly idle, this plays at 60-120 fps.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2: the ExeFS, and the RomFS with the update merged in.

### Updating from 0.2.8-pc.x

Unzip `WSword-0.3.0-pc.1-windows.zip` **over your WSword folder** and replace the files, then run `WSword.exe`:

1. It finds the game files and saves you set up before (your `v190` or `v182` folder). No new copy of the game is made.
2. It builds the game code once, on its own. This takes a few minutes and uses every CPU core.
3. It removes the old `romfs.bin` (about 15 GB), which it no longer needs.
4. It starts the game.

Settings start fresh, in the game's own Options. If it asks for game files instead (1.0.0 / 1.6.0, or files set up in place), choose your `exefs` / `romfs` folder again.

### A new install

Unzip anywhere you can write to (not Program Files). Run `WSword.exe`, choose the folder with your extracted game and **Set up game files**.

### What's different

- **Settings** are in the game's own Options (pause menu > System > Options):
  - frame rate 30 / 60 / 90 / 120 / no limit;
  - resolution and AMD FSR, VSync, draw distance, colour grading, HDR10;
  - **AMD FSR 3 frame generation**;
  - keys, mouse and motion.
- **F10** shows the FPS overlay.
- **WSword Configuration.exe** has the game files, settings and Wii U / Cemu save import.
- The DLC is supported, from your own files.
- **Not in this version:**
  - WSword's Glass Slate start screen;
  - frame generation with Lossless.dll (FSR 3's is built in);
  - game versions 1.0.0 and 1.6.0.

The full guide is in the zip (`WSword-PC-guide.md`). Savage Wind's license is in `LICENSE.txt`, and the bundled components' licenses are in `SavageWind-README.md`. The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.0-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.0-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
