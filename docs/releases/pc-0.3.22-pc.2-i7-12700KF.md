**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen; it runs on that and newer Intel CPUs like it). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Ray-traced shadows: the PS5's fixes

- **Far objects:** past 50 m the ray-traced shadows hand over to the game's own (all of it by 90 m), so far hills and buildings no longer get lines across them.
- **Grass:** no more shimmering under the ray-traced shadows.
- **Faster:** half the rays each frame, with the last frames' result reused and turned each frame, so busy places hold their frame rate better.

### Taken back out from 0.3.22-pc.1

Wide HUD and menus (21:9), the binding rows named by action and the keyboard on the Controls screen are gone again: everything is as in 0.3.21-pc.1 apart from the ray-traced shadows.

### Updating

Unzip `WSword-0.3.22-pc.2-i7-12700KF-windows.zip` **over your WSword folder** and replace the files. Everything else stays as it is, and the PC's game code isn't built again.

The PS5 export still goes with WSwordPS5 v0.3.23-m4.

Made without testing in the game.
