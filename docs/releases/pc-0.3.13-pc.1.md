**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update makes frame generation work with ReShade, brings back the DLC for dumps that have it merged in, and keeps NVIDIA DLSS frame generation from locking you out.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Frame generation with ReShade

Before, ReShade drew its effects on every frame shown, generated ones too, so with heavy effects frame generation gave little or nothing. Now, with frame generation on (AMD FSR 3, Lossless or NVIDIA DLSS), WSword draws ReShade's effects **once per game frame, on the game's picture before its HUD**: the generated frames are made from it and have the effects at no extra cost, and the HUD stays crisp, untouched by the effects. Without frame generation, ReShade works as before.

### The DLC, for dumps that have it merged in

Some dumping guides copy the DLC's files over the game's. WSword played those without the DLC (no Downloadable Content button on the title menu). Now it finds the DLC in the game's files and uses it, as if it were set up apart. A DLC dumped on its own still goes in through **WSword Configuration > Game files > DLC**.

### NVIDIA DLSS frame generation

It can still close WSword while it starts on some PCs. Now the next start turns it off by itself, so WSword always starts again, and `WSword.log` gets the crash report NVIDIA's code used to hide. Send `WSword.prev.log` if it happens.

### Updating

Unzip `WSword-0.3.13-pc.1-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included.
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

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.13-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.13-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
