**WSword for PC (Windows 10/11, 64-bit)**, still built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT) and running as fast as 0.3.0-pc.1. This update brings back what 0.3.0 left out from WSword 0.2.8.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Back from 0.2.8

- **The Glass Slate start screen** (WSword Configuration.exe):
  - a glass panel on the left: PLAY, Saves, Settings, Game files and Quit, with the game files' state and your GPU;
  - chips at the top right: the refresh rate, frame generation, the battery on a laptop;
  - a card at the lower right: when you last saved, Resume, and the modes.
- **The Glass Slate settings look:** the pages down a rail on the left (Files, Video, Audio, Controls, Language, Saves), each option a glass row with its explanation in a card on the right.
- **Modes:** Battery (30 fps, AMD FSR 1), Performance (60), Turbo (120) and Unlocked (no limit) - on the start screen's card, in Settings > Video > Mode and in the game's Options > Video.
- **Settings > Files > Look:** your own start screen background (PNG or JPEG), and WSword's icon - Sword, Slate, Midnight, Dawn, Ancient or Forest - on its window and in the taskbar.
- **Lossless Scaling frame generation**, with your own copy of `Lossless.dll` (not included):
  1. Settings > Files > Lossless Scaling > **Choose Lossless.dll**.
  2. Settings > Video (or the game's Options > Video) > **Frame generation**: Lossless 2x, 3x or 4x.
  - The frame rate counts the frames on screen, as with AMD FSR 3: at 120, 2x draws 60, 3x 40, 4x 30.
  - Flow scale (Automatic follows Lossless Scaling's advice) and Performance mode are in Settings > Video.
  - On NVIDIA GTX 16 / RTX 20 and newer, the generated frames are presented by a thread of their own, so the game goes straight on with its next frame.
  - The menus, the map and the title screen show the game's own frames.

AMD FSR 3 frame generation is still there: Frame generation > AMD FSR 3.

### Updating

Unzip `WSword-0.3.1-pc.1-windows.zip` **over your WSword folder** and replace the files.
- **From 0.3.0-pc.1:** your game files, game code, saves and settings stay as they are.
- **From 0.2.8-pc.x:** as 0.3.0-pc.1 describes - it finds your game files and saves, builds the game code once, and starts. Your `lsfg\Lossless.dll` and chosen background are used again.

The full guide is in the zip (`WSword-PC-guide.md`).

### Licences

- Savage Wind's: `LICENSE.txt`, and the components' in `SavageWind-README.md`.
- Inter (the menus' font, OFL): `Inter-OFL.txt`.
- Lossless frame generation's code is lsfg-vk by PancakeTAS and contributors (GPL-3.0): `lsfg-vk-LICENSE.md` and `lsfg-vk-README.md`. WSword isn't made by or affiliated with Lossless Scaling's developers.

The source is [`pc/`](https://github.com/RJ22256/SheikahSlate/tree/v0.3.1-pc.1/pc) in this repository, with WSword's changes in [`pc/WSWORD.md`](https://github.com/RJ22256/SheikahSlate/blob/v0.3.1-pc.1/pc/WSWORD.md).

If something goes wrong, send `WSword.log` (next to `WSword.exe`).
