**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update brings mods back.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Back from 0.2.8: mods

Mods for the Switch version, laid out as for Ryujinx, Yuzu or Eden, work as they are: each is a folder with a `romfs` folder inside, made for your game version.

1. Settings > **Mods** (on the start screen, or F1 in the game) > **Import a mod folder**, then pick the mod's own folder (extracted from its ZIP / RAR / 7z: the one with `romfs` inside). WSword copies it into the `mods` folder in its data folder. Or put mod folders there yourself: **Open the mods folder**.
2. Every mod there is on. The Mods page turns each one on or off, with the mod's own note (a `.txt` in its folder) as its description.
3. A change applies when the game starts again.

- **Order:** where two mods have the same file, the one first in the list (by name) wins.
- **DLC files:** a mod's `romfs\aoc\0010`, or a mod folder named `01007EF00011F001` (as BCML and Relics of the Past ship them). They need the DLC set up in Settings > Files; the Mods page warns if it isn't.
- Mods change files only. Code patches (exefs, `.pchtxt`, `.ips`) don't apply.
- Your mods from 0.2.8-pc.x: move their folders into the `mods` folder (or Import them), and the ones you had turned off, turn off again.

### Updating

Unzip `WSword-0.3.5-pc.1-windows.zip` **over your WSword folder** and replace the files.
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings, `nvngx_dlss.dll` and `streamline\nvngx_dlssg.dll` stay as they are. Replace `wsword_dlss.dll` too if you're coming from 0.3.3-pc.1 or older.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK (under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`) and to NVIDIA's Streamline SDK.
- The `streamline` folder holds NVIDIA's Streamline: its signed DLLs, and its licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.5-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.5-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
