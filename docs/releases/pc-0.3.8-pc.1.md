**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update adds ray-traced shadows (experimental).

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### New: ray-traced shadows (experimental)

On graphics cards with hardware ray tracing (NVIDIA RTX, AMD Radeon RX 6000 and newer, Intel Arc): Settings > Video > **Ray-traced shadows**. It turns on and off in the game.

- **What it does:** the sun's shadows are traced from the same objects the game draws into its shadow maps. They're sharp where things touch and soften with distance, without the maps' blur, blockiness and shimmer.
- **Leaves:** tree leaves and other cut-out foliage keep the game's own shadows. Grass close to the ground lets the rays through.
- **Range:** as far as the game's own shadows reach. Past that, and where the game draws no sun shadows, nothing changes.
- **Cost:** a few milliseconds of GPU time. The first minute with it on, the game compiles a variant of each shadow-casting shader: expect some stutter while it does.
- **Experimental:** it's the first version. If shadows look wrong (missing, misplaced, striped, flickering), press **Ctrl+F9** in that spot and send `WSword.log` and the `build\rt-report` folder, with a screenshot.

Thanks to the ray tracing reports sent from 0.3.7-pc.1.

### Updating

Unzip `WSword-0.3.8-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings, mods, `nvngx_dlss.dll` and `streamline\nvngx_dlssg.dll` stay as they are.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK (under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`), to NVIDIA's Streamline SDK, and to ReShade (its add-on API headers, BSD-3-Clause: `ReShade-API-LICENSE.md`).
- The `streamline` folder holds NVIDIA's Streamline: its signed DLLs, and its licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.
- ReShade is crosire's ([reshade.me](https://reshade.me)); none of it ships with WSword. WSword isn't made by or affiliated with ReShade.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.8-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.8-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
