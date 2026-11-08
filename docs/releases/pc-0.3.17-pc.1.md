**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update raises the frame rate's ceiling and removes Path tracing.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Frame rate: up to 1000 fps

With Video > Frame rate set to **Off**, the game stopped at 300 fps. It now goes up to **1000 fps**, and the game keeps its normal speed all the way up there - the time of day, physics and controls all follow the real time.

It doesn't go past 1000: the game's clocks lose precision with very short frames, so with no ceiling at all the time of day and the game's speed would start to drift. (For testing, `SAVAGEWIND_FPS_CEILING=<n>` sets another ceiling, 60 to 4000.)

### Path tracing removed

The Path tracing option from 0.3.16-pc.1 is gone. Ray-Tracing (shadows and ambient occlusion) is as it was.

### Updating

Unzip `WSword-0.3.17-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings, mods, `nvngx_dlss.dll` and `streamline\nvngx_dlssg.dll` stay as they are.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`. Chakra Petch (the headings' font, OFL): `ChakraPetch-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK (under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`), to NVIDIA's Streamline SDK, and to ReShade (its add-on API headers, BSD-3-Clause: `ReShade-API-LICENSE.md`).
- The `streamline` folder holds NVIDIA's Streamline: its signed DLLs, and its licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.
- ReShade is crosire's ([reshade.me](https://reshade.me)); none of it ships with WSword. WSword isn't made by or affiliated with ReShade.
