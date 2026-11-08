**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen; it runs on that and newer Intel CPUs like it). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Sharper ray-traced shadows

- **Edges stay sharp.** Every pixel of the ray-traced shadows used to be blurred with its neighbours; now only the soft edge of a shadow (where its rays disagree) is smoothed, and everything fully lit or fully in shadow keeps its own value - a shadow's edge is as crisp as the game's.
- **Narrower soft edges.** The rays spread over a smaller sun, so shadows soften with distance only half as much.
- **No more flicker while things load in.** When something new appears whose shader is still being built, its shadow can't be traced yet and the game's own is used alongside. That used to flip on and off every other frame; it now stays on for two seconds after, so the shadows don't flicker.

### Draw distance: Extreme+

Video > Draw distance has a fourth level, **Extreme+**: trees, rocks, buildings and enemies 1.4 times the game's own distance (High: 1.3), far buildings and the distant forests 1.5 times, and grass to its furthest. That is the most the game takes: further out, it starts leaving buildings out (Rito Village's bridge vanished at 1.5) and crashes while it loads the world. It costs more CPU time.

### Shrines

F9 now makes the `build` folder itself: the shrine capture didn't save its images because your folder had none. A new F9 inside a shrine saves everything needed for shrine floor reflections.

Made without testing in the game.

### Updating

Unzip `WSword-0.3.20-pc.1-i7-12700KF-windows.zip` **over your WSword folder** and replace the files. Everything else stays as it is, and the game code isn't built again.

### Licences

As in 0.3.17-pc.1: `LICENSE.txt`, `SavageWind-README.md`, `Inter-OFL.txt`, `ChakraPetch-OFL.txt`, `lsfg-vk-LICENSE.md`, `NVIDIA-DLSS-LICENSE.txt`, `ReShade-API-LICENSE.md` and the `streamline` folder's licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks; WSword isn't made by or affiliated with NVIDIA, Lossless Scaling's developers or ReShade.
