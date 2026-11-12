**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen; it runs on that and newer Intel CPUs like it). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Ray-traced shadows: the PS5's fixes

- **Far objects:** past 50 m the ray-traced shadows hand over to the game's own (all of it by 90 m), so far hills and buildings no longer get lines across them.
- **Grass:** no more shimmering under the ray-traced shadows.
- **Faster:** half the rays each frame, with the last frames' result reused and turned each frame, so busy places hold their frame rate better.

### 21:9 (and 32:9): Wide HUD and menus

Video > **Wide HUD and menus** (on; it needs Fill wide screens, and applies after a restart). On a screen wider than 16:9:
- the **HUD** goes to the screen's edges: hearts and stamina top left, the mini-map, temperature, noise and time bottom right, the rune and Korok counters, the save icon, the camera's frame;
- the **start screen** fills the screen (its paintings zoomed to the width) with the logo and the menu at the edges;
- the **map** shows more to the sides.

The pause menu's pages stay in the middle for now (they slide from page to page). Turn the setting off for the old 16:9 HUD.

### Keyboard controls

- **Settings > Controls**: the keyboard and controller rows say what each input does - Jump, Attack, Sprint / Cancel, Use rune, Move forward, Camera left... - instead of the Switch's buttons (still in each row's description).
- The game's **Controls screen** (System > Controls) shows a **keyboard** while you play on the keyboard: your keys, and the mouse, at the places its labels point to. With a controller it shows the controller, as before.

### Updating

Unzip `WSword-0.3.22-pc.1-i7-12700KF-windows.zip` **over your WSword folder** and replace the files. Everything else stays as it is, and the PC's game code isn't built again.

The PS5 export still goes with WSwordPS5 v0.3.23-m4.

Made without testing in the game.
