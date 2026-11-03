**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update brings WSword's Settings back over the game, and adds NVIDIA DLSS.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Settings in the game again

- **F1** opens WSword's Settings over the game, in the Glass Slate look, as in 0.2.8. On a controller: **Back + Start**, or the guide button.
- Its pages: Video (Mode, frame rate, resolution, upscaler, frame generation and the rest), Audio, Controls, Language and Quit.
- F1 again, BACK, B or Esc closes it.
- The game's own Options (pause menu > System > Options) still has the settings too. Its Video page is back to Savage Wind's 19 rows, with **Frame generation** (Off, AMD FSR 3, Lossless 2x / 3x / 4x) as one choice. Mode and Lossless's own options are in the F1 Settings.

### NVIDIA DLSS (new)

DLSS upscaling on **NVIDIA RTX** graphics cards, with your own copy of `nvngx_dlss.dll` (not included - it comes with NVIDIA's DLSS SDK, and most games with DLSS have one in their folder):
1. **WSword Configuration** > Settings > Files > NVIDIA DLSS > **Choose nvngx_dlss.dll**. WSword keeps a copy next to `WSword.exe` (putting it there yourself works too).
2. Settings > Video > **Upscaler**: **NVIDIA DLSS**, then its mode on the row below (Ultra Quality to Ultra Performance).

- The game renders smaller and DLSS builds the full-resolution image from the frames before. It anti-aliases too, so the picture is steadier.
- If DLSS isn't offered, or stops, Upscaler's description says why (no RTX card, no `nvngx_dlss.dll`, an old driver), and so does `WSword.log`.
- DLSS helps when the GPU is what holds the frame rate back. When the CPU is, it won't add frames.

### Updating

Unzip `WSword-0.3.2-pc.1-windows.zip` **over your WSword folder** and replace the files.
- **From 0.3.0-pc.1 or 0.3.1-pc.1:** your game files, game code, saves and settings stay as they are.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK, under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`. NVIDIA, RTX and DLSS are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.2-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.2-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
