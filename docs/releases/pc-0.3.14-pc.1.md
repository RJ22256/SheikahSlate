**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update renames Ray-traced shadows to **Ray-Tracing**, adds ray-traced ambient occlusion, and makes the ray-traced shadows sharper.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Ray-Tracing

Settings > Video > **Ray-traced shadows** is now **Ray-Tracing** (your setting carries over), and it does more:

- **Ambient occlusion:** short rays from each surface find where the sky's light is closed off - corners, under rocks and overhangs, where things meet the ground, around Link - and shade it. It's applied to the game's picture before the HUD, so ReShade and frame generation see it. Grass and leaves don't take part in it.
- **Sharper shadows:** they were softer and blurrier than the game's own. Their softness now matches the sun's real size, and each pixel traces twice as many rays, so the edges are crisp near what casts them and the grain is gone.

Both are experimental and were made without testing in the game: if something looks wrong, press **Ctrl+F9** there and send `WSword.log` and the `build\rt-report` folder.

### Updating

Unzip `WSword-0.3.14-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
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

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.14-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.14-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
