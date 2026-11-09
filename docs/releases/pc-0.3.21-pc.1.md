**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen; it runs on that and newer Intel CPUs like it). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Export for PS5

WSword Configuration > Game files has a new **PlayStation 5** section: **Export for PS5** builds the game code for WSword on a jailbroken PS5 (the PPSA99777 title from WSwordPS5), from the game files you already set up here. It takes about as long as the setup's build and leaves the PC's own game code alone.

When it's done, copy these to `/data/homebrew/PPSA99777/` on the console over FTP, beside `eboot.bin`:
- `PS5/swgame.so` and `PS5/swgame.so.id` from your WSword folder
- your WSword folder's `extracted` folder (the game files, as they are)

Use the WSwordPS5 release made for this version (v0.3.0-m4 and later): the console only loads game code built with its own version's settings.

### Updating

Unzip `WSword-0.3.21-pc.1-i7-12700KF-windows.zip` **over your WSword folder** and replace the files. Everything else stays as it is, and the PC's game code isn't built again.

### Licences

As in 0.3.17-pc.1, and `gamelib-src/ps5-include`: FreeBSD's C library headers (BSD-style licences, each file's own notice), what the PS5 export compiles the game code against.
