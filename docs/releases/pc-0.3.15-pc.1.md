**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update brings the **Glass Slate 2** look to WSword Configuration, Settings, the in-game menu and the first setup, and **F6** for Ray-Tracing.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Glass Slate 2

- **Home (WSword Configuration):** a rail down the left - Home, Saves, Settings, Mods, Game files, with your game files and GPU at its foot and Quit. Over the picture: *Continue your journey*, when you last saved, **PLAY** and **Choose a save**, chips for what's on (Ray-Tracing, the upscaler, frame generation, ReShade, mods), and your three newest saves (click one for Saves). The modes (Battery, Performance, Turbo, Unlocked) sit at the top right.
- **Settings:** Ray-Tracing is the first thing on the Video page, as a card with its switch and its F6 key. The page list shows what's on: **RT** on Video when Ray-Tracing is on, how many mods are on beside Mods.
- **In the game (F1):** quick toggles beside the page title - Ray-Tracing, frame generation (off, or back to the one you used), ReShade (from the next start) and the FPS counter / full overlay.
- **First setup:** four steps across the top (choose your files, check, build the game code, play), and what's been found so far - the folder you chose, the DLC.
- Headings use **Chakra Petch** (OFL).

### F6: Ray-Tracing on and off

Press **F6** in the game to turn Ray-Tracing on or off; a short notice says which.

Made without testing in the game: if something looks wrong, send a screenshot and `WSword.log`.

### Updating

Unzip `WSword-0.3.15-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
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
