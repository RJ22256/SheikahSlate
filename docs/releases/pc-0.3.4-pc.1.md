**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). This update adds NVIDIA DLSS frame generation.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### New: NVIDIA DLSS frame generation

NVIDIA's frame generation on **RTX 40** graphics cards and newer: **2x** on RTX 40, **3x** and **4x** on RTX 50. It uses your own copy of `nvngx_dlssg.dll` (not included; it comes with NVIDIA's Streamline SDK, and most games with DLSS frame generation have one in their folder).

1. **WSword Configuration** > Settings > Files > **NVIDIA DLSS frame generation** > **Choose nvngx_dlssg.dll**. WSword keeps a copy in its `streamline` folder.
2. Settings > Video > **Frame generation** (or the game's Options > Video): **NVIDIA DLSS 2x** (3x and 4x on RTX 50).
3. **Restart WSword.** NVIDIA's frame generation has to be in from the moment WSword starts.

- **Frame rate:** as with the other frame generation, the frame rate counts the frames on screen. At 120, 2x draws 60.
- **Windows:** you need Windows 10 version 2004 or newer, with **Hardware-accelerated GPU scheduling** on (Windows Settings > System > Display > Graphics > Change default graphics settings, then restart the PC).
- **VSync:** NVIDIA's frame generation has no VSync on Vulkan, so while it's in, VSync On and Adaptive work as Fast. G-SYNC and FreeSync still follow the frames.
- **Reflex:** NVIDIA Reflex comes with it, to keep the delay down.
- **HDR:** HDR is off while it's in.
- **Menus:** the menus, the map, the title screen and the F1 Settings show the game's own frames.
- **If it doesn't run:** the Frame generation description in Settings > Video says why (no RTX 40, no `nvngx_dlssg.dll`, scheduling off, an old driver), and so does `WSword.log` (the `[dlssg]` lines).

### Updating

Unzip `WSword-0.3.4-pc.1-windows.zip` **over your WSword folder** and replace the files.
- **From 0.3.0-pc.1 or later:** your game files, game code, saves, settings and `nvngx_dlss.dll` stay as they are. Replace `wsword_dlss.dll` too: older ones don't work with this version.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.
- `wsword_dlss.dll` is WSword's bridge to NVIDIA's DLSS SDK (under the SDK's licence: `NVIDIA-DLSS-LICENSE.txt`) and to NVIDIA's Streamline SDK.
- The `streamline` folder holds NVIDIA's Streamline: its signed DLLs, and its licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks. WSword isn't made by or affiliated with NVIDIA.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.4-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.4-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
