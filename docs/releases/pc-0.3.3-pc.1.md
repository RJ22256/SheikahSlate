**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update makes NVIDIA DLSS faster, especially on RTX 20 and 30 cards.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Faster DLSS

- **A lighter model on RTX 20 and 30 cards.** Recent `nvngx_dlss.dll` files use NVIDIA's newest models for Performance and Ultra Performance modes. They're made for RTX 40 and newer; on an RTX 20 or 30 they took 5-14 ms a frame on their own, more than rendering smaller saved. WSword now uses the Transformer model on those cards in every mode.
- **No more waiting for DLSS each frame.** DLSS now runs on WSword's render thread, so the game's thread no longer stops every frame for the render thread to catch up.
- **A file check that ran many times a frame** is now done twice a second.
- **If the game puts the scene on screen twice in one frame**, DLSS now runs once instead of twice.

### New: DLSS model

Settings > Video > **DLSS model**, under Upscaler mode (RTX cards only):
- **Automatic:** NVIDIA's own choice for each mode on RTX 40 and newer; the Transformer on RTX 20 and 30.
- **CNN (fastest):** the older model, about 1 ms a frame at 1440p on an RTX 3060. A little softer, with more ghosting.
- **Transformer:** sharper and steadier, about 2.4 ms there.
- **Transformer 2:** NVIDIA's newest, for RTX 40 and newer.

The performance overlay (**F10**, the full one) now shows DLSS's model and the GPU time it takes each frame. Upscaler's description shows them too.

DLSS speeds the game up when the GPU is what holds it back. If GPU % in the overlay is well under 100 without DLSS, the CPU is the limit, and DLSS can't add frames there.

### Updating

Unzip `WSword-0.3.3-pc.1-windows.zip` **over your WSword folder** and replace the files.
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings and `nvngx_dlss.dll` stay as they are. Replace `wsword_dlss.dll` too: the one from 0.3.2-pc.1 doesn't work with this version.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK, under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`. NVIDIA, RTX and DLSS are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.3-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.3-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
