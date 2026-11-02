# WSword for PC (Windows 10/11)

WSword's PC build: the same recompiled *Breath of the Wild* as the Android app, with the Android work carried over -
the Glass Slate start screen and Settings, Video > Mode, even frame pacing, the steadied dynamic resolution, frame
generation with your own Lossless.dll, eleven menu languages - and what a PC needs on top: mouse and keyboard, a
window or fullscreen, high-refresh and ultrawide monitors.

> [!NOTE]
> **No game files and no Lossless.dll are included.** You need your own dump of *Breath of the Wild* 1.0.0, 1.6.0,
> 1.8.2 or 1.9.0 (`exefs` and `romfs`, extracted), and for frame generation your own copy of Lossless Scaling.

## What you need

- Windows 10 or 11, 64-bit, on a CPU with AVX2 (Intel 4th generation / Haswell or newer, AMD Ryzen or newer).
- A GPU with a **Vulkan 1.3** driver: NVIDIA GTX 900 series or newer, AMD RX 400 series or newer, Intel Arc or Iris Xe -
  with a recent driver from NVIDIA's, AMD's or Intel's site.
- About 30 GB free for each game version you set up (its files and the game code built from them).

## Installing and the first start

1. Unzip `WSword-<version>-windows.zip` anywhere you can write to (not `Program Files`; a folder synced by OneDrive
   works but is slower). Keep everything in the `WSword` folder together.
2. Run `WSword.exe`. On the start screen choose **Set up game files** and pick the folder with your `exefs` and
   `romfs`. WSword copies the files, builds the game's data image, then **builds the game's code on your PC** with the
   compiler in its `zig` folder - a few minutes, once per game version (it uses every CPU core).
3. **PLAY**. The first time it asks for the aspect ratio (Unlocked fills an ultrawide) and shows how to open the menu.

Saves, settings and the game files stay in WSword's data folder; Settings > Files > Saves opens it in Explorer.

## Controls

| | Keyboard and mouse | Controller |
| --- | --- | --- |
| Move | W A S D | Left stick |
| Camera | Mouse (Settings > Controls > Mouse camera: speed, invert) or numpad 8 4 5 6 | Right stick |
| Attack (Y) | Left mouse button, or I | Y |
| Bow (ZR) | Right mouse button - hold to aim, let go to shoot - or 3 | ZR |
| Lock on / shield (ZL) | Middle mouse button, or 1 | ZL |
| L / R | Mouse side buttons, or Q / E | L / R |
| A / B / X | K / J / L | A / B / X |
| + / - | Enter / Backspace | + / - |
| WSword's menu | Esc or F1 | Home / Xbox / PS button, or Back + Start |
| Fullscreen | F11 or Alt+Enter | |

The keys can be changed in Settings > Controls. While you play the game has the mouse; Esc, F1 or holding Alt lets it
go. Any controller SDL knows works (Xbox, DualSense / DualShock 4, Switch Pro, 8BitDo...), with its gyroscope as the
motion controls where it has one.

## Video settings on a PC

- **Mode**: Battery (30 fps, light - a laptop on battery), **Performance** (the default: 60 fps where your PC keeps up,
  30 where it doesn't, at your window's resolution), Turbo (40 fps for a 120 Hz monitor), Turbo 60 (always 60).
- **Display mode**: Windowed (resize it to any shape - with Aspect ratio Unlocked an ultrawide or a tall window shows
  more), Borderless fullscreen, or Exclusive fullscreen with its own **Refresh rate**: Highest, 60 Hz, the desktop's,
  or **120 Hz** - a whole number of refreshes for every frame at 30, 40 and 60 fps, the right choice on a 144 or 165 Hz
  monitor without G-SYNC / FreeSync.
- **Even frame pacing**: every frame on screen for the same time. AMD's driver reports when frames reach the screen, so
  there each frame is aimed at a refresh of its own (as on Android; Automatic turns it on). NVIDIA's and Intel's don't:
  there **On** shows each frame again for every refresh it stays up, on a vsync'd swapchain - even where the monitor's
  rate is a whole number of frames (60, 120, 240 Hz), at the cost of a refresh or two of delay. With G-SYNC / FreeSync
  leave it off: the monitor follows the game by itself.
- **Game's resolution changes**: Steadied (the default) stops one slow frame from swinging the game's resolution.
- **Frame rate**: Auto (30 / 60), 30, 40, 60 fps, or **Unlocked** - as fast as your PC goes (up to 300), the menus,
  cutscenes and movies still at the game's 30. Unlocked suits a G-SYNC / FreeSync or high refresh monitor; on a 60 Hz
  one without it, 60 fps is smoother.
- **Render thread** (on): the game's drawing is turned into Vulkan work on a thread of its own while the game goes on
  with its next frame - much faster. Off only to check whether it causes a problem. **Sync lag** (on, needs the render
  thread): the game starts its next frame while the GPU finishes the last one; Link's legs can sink into the ground
  standing still - turn it off if that bothers you.

## Frame generation (Lossless Scaling)

1. Find `Lossless.dll` in Lossless Scaling's folder (Steam: Lossless Scaling > Manage > Browse local files).
2. On WSword's start screen: **Settings > Files > Lossless Scaling > Choose Lossless.dll**, and pick that file. WSword
   checks it holds the frame generation shaders and keeps a copy (`lsfg\Lossless.dll` in its data folder). It never
   looks for the file by itself.
3. In the game: **Settings > Video > Frame generation** 2x, 3x or 4x. The line under it says what it does on your
   monitor and its GPU time; **Flow scale** (Automatic by default) and **Performance mode** lower that time.

It works best where the monitor's rate is a whole number of the game's frames: 2x at 30 fps on 60 Hz; 4x at 30, 3x at
40 or 2x at 60 on 120 Hz. A choice past what the monitor shows is lowered (4x at 60 fps on 120 Hz makes 2x). On
NVIDIA and Intel the in-between frames go up one per refresh on a vsync'd swapchain (and again where they stay up
longer), with G-SYNC / FreeSync they can come unevenly; AMD's driver aims each of them at its refresh. It's paused while
WSword's menu is open; the menus and the FPS overlay are drawn on every frame, never generated.

Frame generation is lsfg-vk's backend (GPL-3.0, `third_party/lsfg-vk`) running Lossless Scaling's shaders from your
DLL. WSword isn't made by or affiliated with Lossless Scaling's developers.

## Updating

Unzip the new release over the old `WSword` folder (replace the files). Your saves, settings and set-up game files are
in the data folder and stay; game code already built is kept unless the new release says it needs building again.

## If something goes wrong

- **The log**: `WSword.log` next to `WSword.exe` (Settings > Files > Graphics > Show the log in Explorer); after a
  crash the run before is kept beside it. Send it with a report.
- **"Vulkan 1.3 driver" errors**: update the GPU's driver from its maker's site. On a laptop with two GPUs WSword picks
  the dedicated one.
- **The game code build fails**: it needs a few GB free on the drive and an antivirus that lets `zig\zig.exe` and
  `botwgen\botwgen.exe` run; the reason is on the setup page and in the log ("compiling ... failed:" with the
  compiler's own message). Set up game files again goes on where it stopped. (0.2.8-pc.1's compiler lacked a header
  the game's code needs - "the game code didn't compile": fixed in 0.2.8-pc.2.)
- **Slow, or the game stops a few seconds into play** (0.2.8-pc.2 and before): the PC ran LAS Recomp's desktop paths,
  which never ran BOTW - fixed in 0.2.8-pc.3, which runs the Android release's game fixes and speed-ups on the PC too
  (the threads' hand-over that BOTW's job system needs, the waits the game could miss, the render thread, the function
  cache, the game's 20 fps step taken out). Unzip it over the old folder - the game code stays as built. If the game
  still stops, wait 10 seconds before closing it: the log gets a `[hang]` report of what each thread was doing.
- **Frame generation says "Stopped"**: the reason follows - usually a Lossless.dll that isn't one, or a driver that
  can't build its shaders. Choose the DLL again, or update the driver.
