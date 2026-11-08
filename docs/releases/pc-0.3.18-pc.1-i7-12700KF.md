**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen, "Alder Lake"; it runs on that and newer Intel CPUs like it, not on others). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT), the same as 0.3.17-pc.1 otherwise. For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### What's different

- **The game code built for speed.** On your PC the start screen builds the game code once more, with full optimisation (`-O2` with vectorisation, where the normal build uses a lighter level for a quicker first build) and for your CPU's own instructions (`alderlake`). Expect that first build to take about 2-3 times as long as before. WSword itself is built for your CPU too.
- **The game on your fast cores.** The i7-12700KF has 8 performance cores and 4 efficiency cores. WSword already gave the game one thread per core; it now gives it one per **performance core** only, so its busy threads (the game, its job workers, the renderer) never land on a slow efficiency core. `WSword.log` says `one logical processor per fast core` when it's on. (`SAVAGEWIND_FAST_CORES=0` turns it off.)
- The start screen shows **i7-12700KF** next to the version.

Made without testing in the game. If the game misbehaves in a way the normal 0.3.17-pc.1 doesn't, go back to that one (it builds its own game code again) and send `WSword.log`.

### Updating

Unzip `WSword-0.3.18-pc.1-i7-12700KF-windows.zip` **over your WSword folder** and replace the files, `wsword_dlss.dll` included. Your game files, saves, settings, mods, `nvngx_dlss.dll` and `streamline\nvngx_dlssg.dll` stay as they are; the start screen builds the game code again once.

Ctrl+F8 (a performance capture) is in this build too: one taken here shows what's left to speed up.

### Licences

As in 0.3.17-pc.1: `LICENSE.txt`, `SavageWind-README.md`, `Inter-OFL.txt`, `ChakraPetch-OFL.txt`, `lsfg-vk-LICENSE.md`, `NVIDIA-DLSS-LICENSE.txt`, `ReShade-API-LICENSE.md` and the `streamline` folder's licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks; WSword isn't made by or affiliated with NVIDIA, Lossless Scaling's developers or ReShade.
