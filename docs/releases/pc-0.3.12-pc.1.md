**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update adds neural rendering (beta) and DLSS frame generation diagnostics.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### New: Neural rendering (beta)

Settings > Video > **Neural rendering** runs [OpenDLSS-NR](https://github.com/aloshdenny/open-dlss) (MIT), an open reimplementation of a generative neural rendering network, on each frame before the HUD: it generates detail and adjusts tone and structure. **On** is the network's own look, **Natural** and **Cinematic** its styles, and **Strength** blends it with the game's frame.

- It needs an **NVIDIA RTX 40 or newer** with a recent driver, and **your own model folder** next to `WSword.exe`, named `nr-model` (`manifest.json` and its stage files). **No model comes with WSword**, and you're responsible for the licence of any model you use.
- It doesn't upscale or add frames, and it's heavy: about 12 ms a frame at 1440p on an RTX 4070 SUPER.
- Turn it on, then restart WSword. The Video page shows whether it's running and its GPU time.
- It's a first beta, built without being tried on a model: if it doesn't start or looks wrong, send `WSword.log`.

### NVIDIA DLSS frame generation

0.3.11-pc.1 fixed its first crash at start, but it can still close WSword while the window is being made. This update makes that crash show in `WSword.log` (NVIDIA's code had been hiding WSword's crash report). It's still being worked on: keep Frame generation off NVIDIA DLSS for now, or use Lossless or AMD FSR 3.

### Updating

Unzip `WSword-0.3.12-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included (and the new `wsword_nr.dll` and `nr` folder).
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings, mods, `nvngx_dlss.dll` and `streamline\nvngx_dlssg.dll` stay as they are.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK (under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`), to NVIDIA's Streamline SDK, and to ReShade (its add-on API headers, BSD-3-Clause: `ReShade-API-LICENSE.md`).
- The `streamline` folder holds NVIDIA's Streamline: its signed DLLs, and its licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.
- `wsword_nr.dll` and the `nr` folder are WSword's bridge to OpenDLSS-NR by aloshdenny (MIT: `OpenDLSS-LICENSE.txt`, `OpenDLSS-NOTICE.txt`). It isn't affiliated with or endorsed by NVIDIA.
- ReShade is crosire's ([reshade.me](https://reshade.me)); none of it ships with WSword. WSword isn't made by or affiliated with ReShade.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.12-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.12-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
