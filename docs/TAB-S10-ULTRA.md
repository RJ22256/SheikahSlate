# Sheikah Slate on the Galaxy Tab S10 Ultra

What this port changes from BOTWRecompAndroid 0.1.9-beta, why, and how to check it on the tablet.

## The target

| | Galaxy Tab S10 Ultra |
| --- | --- |
| Models | SM-X920 (Wi-Fi), SM-X926B (5G), SM-X920N / SM-X926N (Korea) |
| SoC | MediaTek Dimensity 9300+: 1x Cortex-X4 3.4 GHz, 3x Cortex-X4 2.85 GHz, 4x Cortex-A720 2.0 GHz |
| GPU | Arm Immortalis-G720 MC12 (Mali family: Samsung's system Vulkan driver only; no BC textures) |
| RAM | 12 GB (16 GB on the 1 TB model) |
| Display | 14.6" Dynamic AMOLED 2X, 2960 x 1848 (16:10), 239 ppi, up to 120 Hz, a camera notch on the top long edge |
| Software | Launched with One UI 6.1 (Android 14); One UI 8 (Android 16) and later since late 2025 |

Upstream was made and tested on a POCO F7 Ultra: Snapdragon 8 Elite, Adreno 830 with Mesa's Turnip driver, a 20:9
3200 x 1440 screen. Almost everything that differs between the two matters to this engine.

## The changes

### 1. Landscape stays locked on Android 16 (manifest)

Apps targeting API 36 have `screenOrientation`, `setRequestedOrientation()` and `resizeableActivity` ignored on
displays at least 600 dp wide, which the Tab is - **except games, identified by `android:appCategory`**
([Android: app orientation, aspect ratio and resizability](https://developer.android.com/develop/adaptive-apps/guides/app-orientation-aspect-ratio-resizability)).
Upstream targets API 36 with no category, so on One UI 8+ the tablet held upright would put the game in portrait.
`AndroidManifest.xml` now declares `android:appCategory="game"` (also the category Android's game features read; a
sideloaded app may still need adding to One UI's Game Launcher by hand) and, as a second line of defence for API 36
only, the
`android.window.PROPERTY_COMPAT_ALLOW_RESTRICTED_RESIZABILITY` opt-out (API 37 removes it).

### 2. No stretched picture on a 16:10 screen (`runtime/nvn/output_scale.c`)

With Video > Aspect ratio "Unlocked" on game 1.8.2, upstream reshapes the final image to the window's shape and widens
the game's cameras to match (`runtime/hle/widescreen.c`). The hooks only ever *widen*: `rt_persp_update` returns early
when the window is not wider than 16:9. On a 16:10 window the targets became 16:10 (scale 1.542 x 1.711) while the main
camera stayed 16:9, so the world rendered ~11% too tall. Now the output reshapes only for windows wider than 16:9;
narrower ones keep 16:9 fitted to the width: 2960 x 1665 with 91.5 px bars above and below - the same as "Original
16:9" and as 1.0.0 / 1.6.0 always were. On the Tab the camera notch falls inside the top bar. The start screen's
one-time aspect question and the menu's description say so on such a screen.

Proved by `tools/test_tablet_layout.c`: the 16:10 checks fail on upstream's `output_scale.c` and pass on this one.

A true 16:10 fill (Vert+: a taller field of view with the HUD kept inside the screen) would need the camera hooks to
change the projection's field of view too, which can't be done safely without the device to look at.

### 3. The camera notch (`SlateDisplay.java`, `runtime/ui/touch_controls.mm`)

- The window is drawn under the notch on every Android version (`LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS`). Android 15+
  already does this for apps targeting API 35+; on One UI 6 / Android 14 upstream's `SHORT_EDGES` mode left a strip,
  because the Tab's notch is on a *long* edge.
- Once a second (with the battery report), the activity hands the notch's rectangles to the runtime when they change -
  the notch moves to the bottom edge when the tablet is turned around (`sensorLandscape`). The on-screen controls'
  default places move out of it: the menu button (top centre) or Minus / Plus (bottom centre). Places the player chose
  in the Controls tab are left alone.

### 4. On-screen controls a thumb can reach (`runtime/ui/touch_units.h`, `touch_controls.mm`)

Upstream sizes the layout in units of 1% of the screen height: 0.7 mm on its phone, 2 mm on the Tab - face buttons
3 cm across and the left stick's resting place 5 cm from the corner. The unit is now at most 0.95 mm on the screen
(the activity passes the physical density as `LARECOMP_SCREEN_DPI`: 239 ppi on the Tab, the system's figure
elsewhere when plausible). Phones keep exactly the layout they had (their 1% is already smaller); on the Tab the face
buttons are ~14 mm and sit ~19 mm from the right edge. The Controls tab's size slider still scales everything.

### 5. Display refresh rate (Video > Display refresh rate)

`Highest` (the default) asks for the display mode with the highest refresh rate at the current resolution: 120 Hz on
the Tab. The game still runs at its own frame rate, but the swapchain presents in mailbox mode, so a frame shows at the
next refresh: on a 120 Hz grid a 30 fps frame that misses its slot shows for 25 ms rather than 50, and in "Auto" a
frame that takes 17-25 ms lands at 25 ms (40 fps) rather than 33. `60 Hz` saves some battery. `System` lets One UI
decide (Motion smoothness, Game Booster). Applied through `WindowManager.LayoutParams.preferredDisplayModeId`, at once.

### 6. The GPU memory path on Mali (`runtime/nvn/vulkan/device.c`, `exec.c`)

Shader global memory (the game's lights, text and more) reaches the guest heap one of three ways: placed GPU memory
(Turnip only), a host-memory import of the heap (`VK_EXT_external_memory_host`), or CPU copies plus a GPU arena.
Neither Turnip nor Qualcomm's driver has the import extension, so upstream never ran that path on Android. Mali drivers
can list it - and when the full-heap import failed, upstream logged it and left global memory reading **zeros** for the
rest of the run, with every shader already translated for the import. Now:

- right after the device is made, a 64 MB slice of the heap's own range is imported and freed (`host_import_probe`);
  if the driver refuses, the run takes the CPU-copy path from the start, as a driver without the extension does;
- if the full import still fails later, `cache/vk_host_import_failed` records the GPU and driver version, and the next
  start goes straight to CPU copies (a driver update tries the import again);
- `LARECOMP_VK_HOST_IMPORT=1` forces the import (no probe), `=0` forbids it.

BC textures, which the Mali GPU lacks, are decoded on the CPU at upload (upstream's path, tested on a Mali-G615 with
LAS). Upstream's faster Turnip paths (placed heap, in-place target reads, LRZ) don't apply to a Mali GPU.

### 7. Text and messages

The graphics driver error dialog and the fatal "driver too old" message no longer send a Tab user to Turnip drivers
(Adreno only): on Mali only the system driver runs, so they point at the tablet's software update.
`[slate]` lines in the log name the device profile, the GPU and the paths taken - paste them into bug reports.

### 8. The rename

| | Upstream | Sheikah Slate |
| --- | --- | --- |
| App name | BOTWRecompAndroid | Sheikah Slate |
| Application id | `com.botwrecomp.android` | `com.sheikahslate.tab` |
| Java package / activity | `com.botwrecompandroid` / `BOTWRecompAndroidActivity` | `com.sheikahslate` / `SheikahSlateActivity` (JNI names to match) |
| Gradle project | `android/botwrecompandroid` | `android/sheikahslate` |
| Data folder | `Android/data/com.botwrecomp.android/files` | `Android/data/com.sheikahslate.tab/files` |
| Settings / log | `botwrecompandroid.ini`, `botwrecomp-log.txt` | `sheikahslate.ini` (an upstream ini copied in is picked up), `sheikahslate-log.txt` |
| Logcat tag | `BOTWRecompAndroid` | `SheikahSlate` |
| Icon | Triforce | `assets/artwork/sheikah-slate-icon-v1.png` (original; `tools/make_icon.py`) |

Game files already set up by upstream's 0.1.9-beta app can be moved over with adb or a computer: copy
`Android/data/com.botwrecomp.android/files/` to `Android/data/com.sheikahslate.tab/files/`. The recompiled game code
carries over too - its id hashes the generator and the headers it is compiled against, none of which this port
changes. (If the start screen asks to build it again anyway, let it.)

## Update 2 (0.1.9-slate.2): text, decals, texture streaming

Installs over 0.1.9-slate.1 (same id, same key, versionCode 2), keeping the game files, saves and settings. The game
code built on the tablet stays valid: its id (`df5f066ed772a211`) is pinned in `android/gamelib_id.expected`, and the
CI fails any build that would change it.

### 9. The game's text on Mali (`device.c` vk_shader_int32, Video > 32-bit shader math)

On the Tab the game's text came out with letters missing, others replaced by pieces of the font sheet ("ABCDE",
"iiklm") or another language's. Its vertex shader finds each glyph's quad and place in the font through 64-bit address
math, and the translated shaders take the 64-bit path wherever the driver lists 64-bit integers - a path tested only
on Turnip. Upstream's own option already said, for Mali stock drivers, "try it if lighting, grass or text are missing
or wrong". It is now Automatic / On / Off, Automatic being **on for Arm GPUs** (off elsewhere, as before). A settings
file from before (`shader_int32 = 0`, upstream's default) loads as Automatic; it is saved as `shader_math`. The first
start after the update translates the shaders again (some stutter while they compile).

Also: the guest heap import now only uses a host-coherent memory type (the first type a driver lists may be cached
and not coherent, so the CPU's writes - vertices, text - wouldn't reach the GPU without a flush); the start-up probe
turns the import off when no coherent type takes the heap's pages.

### 10. Decals read in place, as an experiment (Settings > Prelaunch > Driver compatibility)

Upstream's biggest speed-up on its phone (31 -> 40 fps): the terrain decals read the G-buffer targets they draw on
instead of a copy made before every one of them. It was locked to Turnip even when set to On. Now **On** applies to any
driver; **Automatic stays off** outside Adreno 8xx. On a tile-based Mali GPU it should hold - the pass ends wherever a
copy used to be made, so a draw reads what earlier passes stored - but it can't be checked from here: if the ground's
decals, roads or shadows look wrong with it On, set it back. The by-the-rules version needs
`VK_EXT_attachment_feedback_loop_layout`; the log's `[slate] driver features` line says whether this driver has it.

### 11. Texture streaming on more cores (`runtime/nvn/vulkan/bc_threads.h`)

This GPU has no BC textures, so each one is decoded on the CPU when it streams in - on the thread recording the frame,
a 2048 x 2048 level being a million blocks. Levels of 4096 blocks or more are now decoded in bands by three worker
threads and the caller (`LARECOMP_BC_THREADS=<n>` to change, 0 for the old way). `tools/test_bc_threads.py` checks the
output is byte for byte the same for every BC format and size (also under ThreadSanitizer): 2.0x faster on a 2-core
build machine, more on the tablet's 8 cores.

Not done, and why: converting the textures to formats the GPU reads natively (ETC2 / ASTC) needs an encoder in the
app and a look at the result on the tablet - a lossy conversion done blind could make every texture worse; caching the
decoded textures on disk would take tens of GB (they decode 4-8x larger). Output resolution's default stays Screen:
1080p (Video > Output resolution) is worth trying, but making it the default wants a measurement first.

## Test checklist (on the tablet)

On the start screen, Settings > Prelaunch > Driver compatibility > **Save a log file**: choose a folder (e.g. Download/SheikahSlate) and keep
`sheikahslate-log.txt` for reports. Video > Show FPS gives the performance overlay.

1. **Start-up.** The app opens in landscape with the tablet held either way up, and stays landscape when held upright.
   The log has `[slate] profile tab-s10-ultra, 239.0 ppi, GPU ...` and `[vk] guest heap import: ...`.
2. **Notch.** On the start screen and in game, nothing important is under the notch; turn the tablet around: the
   touch controls' menu button (top) or Minus / Plus (bottom) move clear of it within a second. Log: `[touch] camera
   cutout ...`.
3. **Picture shape.** 1.8.2 with "Unlocked": round things look round (a Sheikah Tower's base, the moon), with thin black
   bars above and below. Same with "Original 16:9".
4. **Global memory.** Text appears (dialogue, the HUD's labels), the world is lit (not uniformly dark), shrines' glow
   shows. If not, send the log: the `[vk] guest heap import` and `[vk] no heap import` lines say which path ran; try
   `LARECOMP_VK_HOST_IMPORT=0` (below).
5. **Controls.** The face buttons and the left stick are comfortable with thumbs while holding the tablet's sides.
   The Controls tab's layout editor still moves and resizes them.
6. **Refresh rate.** Video > Display refresh rate: with Highest, Developer options > Show refresh rate reads 120 in
   game; 60 Hz switches it at once. Logcat: `display refresh: ...`.
7. **Heat and speed.** 15-20 minutes on the Great Plateau at Frame rate 30: note FPS, SoC temperature and whether it
   throttles. Internal resolution Dynamic, or 75% / 50%, if the GPU can't hold 30.

Switches for testing (adb, as upstream's `android/run_boot.sh`):

```bash
adb shell am start -n com.sheikahslate.tab/com.sheikahslate.SheikahSlateActivity --es env LARECOMP_VK_HOST_IMPORT=0
adb logcat -v time SheikahSlate:V '*:S'
```

## What wasn't changed, and why

- **Performance defaults** (30 fps, 1x internal resolution, Snapdragon GSR 1, screen-resolution output) stay as
  upstream chose them: they would need measurements on the Tab to change for the better.
- **Pre-rotation** of the swapchain: the compositor still turns the image if the panel's native orientation differs;
  untestable without the device.
- **Split screen and pop-up view** stay possible; a window narrower than 16:9 is letterboxed like the full screen.
- **The update check** stays off on Android, as upstream had it; its URLs point at a placeholder `OWNER/SheikahSlate`.
