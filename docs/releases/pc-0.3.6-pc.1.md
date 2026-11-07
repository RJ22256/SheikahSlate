**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update adds ReShade, loaded by WSword itself.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### New: ReShade inside WSword

WSword now loads your own ReShade itself, with no installer. ReShade isn't included with WSword.

1. Download ReShade from [reshade.me](https://reshade.me). Get the version **with full add-on support** if you use ReShade add-ons.
2. **WSword Configuration** > Settings > Files > **ReShade** > **Choose ReShade**. Pick ReShade's setup program as you downloaded it (WSword takes `ReShade64.dll` out of it), or a `ReShade64.dll`.
3. Settings > Video > **ReShade**: On, then **restart WSword**. In the game, **Home** opens ReShade's menu.

- **Its files:** ReShade's settings, presets, shaders (`reshade\reshade-shaders\Shaders`) and add-ons (`.addon64`) go in the `reshade` folder next to `WSword.exe`. **Open the ReShade folder** in Settings > Files takes you there.
- **The game's depth:** Settings > Video > **ReShade depth** is **The game's (WSword)** by default. WSword gives ReShade's effects (ambient occlusion, depth of field, fog...) the game's own 3D depth, lined up with the picture. It's set up for ReShade's default depth settings, so leave those as they are. It replaces ReShade's Generic Depth add-on, which saves CPU time. **ReShade's own detection** brings Generic Depth back.
- **Installed ReShade with its installer before?** WSword uses the `ReShade.ini` the installer left next to `WSword.exe`, so your presets stay. Uninstall it from WSword with the installer, so it isn't loaded twice.
- ReShade's effects cover the whole window, WSword's menus included.
- **If it doesn't run:** Settings > Video > ReShade's description says why, and so do `WSword.log` (its `[reshade]` lines) and ReShade's own `ReShade.log` in the `reshade` folder.

### Updating

Unzip `WSword-0.3.6-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included: the game's depth for ReShade comes from it.
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

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.6-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.6-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
