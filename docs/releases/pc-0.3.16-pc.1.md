**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update adds **Path tracing**: light bouncing between surfaces, traced with your graphics card's ray tracing.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Path tracing

Settings > Video > Ray-Tracing > **Path tracing** (also in the in-game F1 menu's quick toggles). It needs Ray-Tracing on.

- From every surface, rays go out over the half of the sky it faces. Where one meets another surface, that surface's light comes back in its colour: sunlit ground lightens the shade next to it, a coloured wall tints what's close to it, corners of lit places fill in.
- It's the light of what the game already lit - the sun, the sky, its own lights, the ray-traced shadows - bounced once more.
- **Limits:** it isn't full path tracing as in games built for it. WSword's rays can only see the shapes the game draws for its sun shadows - no textures or materials - so the light it brings back is taken from what's on screen. A surface out of view or hidden behind something adds nothing. It works where Ray-Tracing does: outdoors, in daylight.
- It costs a few milliseconds more of GPU time. Settings > Video shows **PT** beside Video while it's on.

Made without testing in the game. If it's too strong, too weak or looks wrong, send a screenshot with it on and off.

### Updating

Unzip `WSword-0.3.16-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
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
