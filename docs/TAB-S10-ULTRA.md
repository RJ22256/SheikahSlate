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

## Update 3 (0.1.9-slate.3): the text, from the tablet's log

The first log from the Tab (driver r44p1, Vulkan 1.3.247) showed: no `VK_EXT_external_memory_host` (the game's memory
reaches the GPU through CPU copies), and memory types `0:0x7 1:0xb` - **the cached type is not coherent**. On Adreno the
cached type is also coherent, so upstream never needed a flush.

### 12. The global memory arena in coherent memory (`exec.c` gres_init)

In the CPU-copy path the ranges shaders read through global memory - the text's vertices among them - are copied into
the arena each draw, and what compute shaders store is read back from it. It asked for cached memory, which on this
driver is the non-coherent type 1, with no flush or invalidate anywhere: the CPU's writes reached the GPU only when its
cache happened to evict them, so letters went missing at random ("adven urous"). The arena now requires host-coherent
memory (type 0 here, the one the transient ring already used; unchanged where a cached coherent type exists). The log
names the arena's memory type. The readback buffer (`vk_readback`: textures the game reads back, screenshots, save
thumbnails) stays cached, and is invalidated after each GPU copy before the CPU reads it.

### 13. CPU-written textures hashed whole (`textures.c` upload_texture)

A texture the page tracking saw the CPU write is hashed in full instead of by a sample of its bytes: a few new glyphs
drawn into a font's glyph cache could miss every sampled byte, leaving blanks until the next periodic full hash.

### 14. Video > Capture a GPU profile (Update 5: Capture a performance profile, #18)

The engine's per-pass GPU timer (`LARECOMP_GPUPROF_AT`, adb only) from the menu: three frames, about 10 seconds after
pressing it, into the log as `[gpuprof]` rows - where this GPU's time goes, for the next optimizations.

## Update 4 (0.1.9-slate.4): speed back, and a check for the menu flash

The Update 3 log: the text right, but the world at 12-15 fps (Update 2: ~26) with the GPU busy only ~27 ms a frame -
the CPU had become the limit. Coherent memory on this driver is also **uncached**, and the CPU reads the arena back
(what shaders stored, compared and copied out every draw): those reads crawled.

### 15. The arena cached again, flushed and invalidated by hand (`exec.c` ga_dirty / ga_flush_dirty / ga_invalidate)

The arena takes the cached type again (type 1 here) and, where it is not coherent, keeps it right explicitly: every CPU
write into it (a resident range copied in, a range loaded for a draw, the miss log reset) widens one dirty range, flushed
once just before each submission; what the GPU wrote is invalidated before the CPU reads it (the stores read back for
the game, the shaders' miss log). One flush a submission instead of uncached writes and reads everywhere. The log's
arena line says "not coherent - flushed and invalidated", and the fps lines add `arena flushes N (X MB)`.
`LARECOMP_GA_COHERENT=1` (adb) brings back Update 3's coherent arena for comparison.

### 16. The frame check (`exec.c` vk_frame_check): what the menu flash is

The video of the "Load save data?" screen: one frame every ~3.5 s (107 frames apart) shows the whole game image in
rainbow contour bands, then it's back. Decoded: that frame shows the final image's **10-bit pixels (RGB10A2, the menu's
target) read as 8-bit RGBA** - re-encoding a normal frame that way reproduces the flash almost pixel for pixel (mean
error 48 against 94 for the frame unchanged; 8-bit read as 10-bit: 85). The on-screen controls and the FPS overlay,
drawn after the game's image, stay clean. The code paths that could do that were read and ruled out (pipeline keys,
render-target and texture views, the view / descriptor-set / texture caches, copy aliases, write-backs, texture
copies, the scene upscaler, slot rewrites), so this update logs it instead of guessing: a frame whose work differs
from 30+ identical frames before it is logged with its counts (passes, draws, submissions and mid-pass splits, new
pipelines and views, uploads, targets drawn or read through a view of another format, pipelines made for other
targets, and whether the presented image was drawn this frame and through which format) - plus each kind of format
mismatch once. In a menu, the flash frames stand out by frame number. `[frame-check]` in the log; at most 120 lines
a run; `LARECOMP_FRAME_CHECK=0` turns it off.

## Update 5 (0.1.9-slate.5): decals on the tile, a CPU profile from the menu

### 17. On-tile reads for decals (`exec.c` tile_read_ok / tile_glsl / tile_fs; `device.c` VK_EXT_shader_tile_image)

BOTW's decals and some effects sample the G-buffer targets they draw into. With Driver compatibility > Draw-target reads
in place: **On**, the renderer ended the pass before each such draw so its texture reads saw memory: in the Update 3
log that was ~43 pass ends a frame (`breaks` ~1075 every 2 s), the G-buffer pass split into up to 46 passes, each
storing and reloading six targets and depth. The Tab's driver has `VK_EXT_shader_tile_image`: a fragment shader reads
its own pixel of a colour target straight from tile memory, in rasterization order (every earlier draw's write
included), so those draws no longer end the pass. Such a draw gets a variant of its fragment shader: each `texture` /
`textureLod` / `texelFetch` of that slot becomes a helper that samples as before and, when the lookup lands on the
fragment's own pixel (within half a texel of its centre, level 0), returns the tile's value instead. Only 2D float
colour targets read in their own format, level 0 / layer 0, unswizzled, bound in their own format; anything else keeps
the pass end. A variant that doesn't compile falls back to the plain shader (logged as `[vk] tile reads: ...`). The fps
line's feedback part reads `(read N, breaks N, tile N)`. Off: copies as before; Automatic stays off on Mali.
`LARECOMP_NO_TILE_READS=1` / `LARECOMP_NO_TILE_IMAGE=1` (adb) turn it off. `tools/test_tile_glsl.py` (CI) rewrites a
shader shaped like the translator's (with its real prelude) and compiles the variants for Vulkan 1.3 with glslang -
glslang takes a tile image and an output on the same location only with the tile image declared first.

### 18. Video > Capture a performance profile

The GPU capture (#14) now also turns on the renderer's CPU profile (`LARECOMP_VK_PROF`'s categories: draw setup, keys,
descriptor sets, texture checks and uploads, vertex copies, submissions, waits; `[hitch]` lines for frames over 50 ms)
for the three fps lines after the first captured frame (~6 s), then off again - where the CPU's time goes, for the next
round of CPU work (the copies).

### Checked, not changed

- The `sampled format 91 (nvn 0x2a)` line (RGBA16 UNORM without linear filtering in Vulkan's table): Mali GPUs filter
  16-bit normalized formats (OpenGL ES's EXT_texture_norm16 requires it); converting it to half floats would lose
  precision on data textures, so it stays.
- The scene's size steps (2220x1249 down to 1184x666 and back): BOTW's own dynamic resolution (no `[dynres]` lines).
  With Internal resolution at Match display the scene is ~1.9x the Switch's pixels; Internal resolution **1x** and
  Output resolution **1080p** roughly halve the GPU's work, and SGSR still fills the screen.

## Update 6 (0.1.9-slate.6): fewer copies, overlapping passes, where the game's own time goes

The Update 5 log, in the world at Internal resolution 1x / Output 1080p: **21-25 fps** (Update 3: 12-15), no pass ends
left (`breaks 0`, ~400 on-tile reads a frame). Per frame: the GPU ~27 ms busy, the render thread ~18 ms - and the
render thread idle ~32 ms of each ~43 ms frame, waiting for the game's own thread to hand over the next one. The game's
logic is the limit now; this update trims the renderer's and the GPU's share and measures the game's.

### 19. Constant buffers copied once a frame (`exec.c` guest_copy)

The copies of guest memory draws bind (constant buffers, some vertex data) were shared within a submission only; a
frame is ~8 submissions on the Tab (passes split by time, dispatches), so the scene's and the camera's constants were
copied again in each. Now within the frame: a copy a later submission reuses keeps its ring chunk alive for that one
(`ring_keep`); copies are dropped at each present, by in-list writes, and where the GPU's results or render targets
are written back into guest memory (`gcopy_retire`, `wb_apply`: ranges queued from any thread, applied by the render
thread). The fps line: `upload X MB (copies Y MB, shared Z MB)` - shared: what a later submission took from an earlier
one's copy. `LARECOMP_GCOPY_PER_SUBMISSION=1`: as before.

### 20. Video > Overlap render passes (`exec.c` begin_pass / light_barrier)

Every pass began with a full barrier: all earlier work done before any of the pass's - its vertex shading and binning
included. A tiler runs a pass's geometry while the pass before still shades pixels; the full barrier made each of the
~300 passes a frame wait for the GPU to drain (80 small 64x64 passes ~2.9 ms, 30 8x8 ones ~1 ms). When only a pass came
before (no copy, dispatch or submission since), the barrier now holds back the new pass's pixel work only (attachment
loads, depth tests, fragment shaders, colour writes). A draw whose vertex shader reads a target drawn since the last full
barrier, or global memory stored to since, or stores itself, gets the full barrier (its pass restarted if it began
light). Automatic: on for Arm GPUs; Off: upstream's full barrier. The fps line: `light barriers N (restarts N)`.

### 21. The capture samples every thread (`platform/android.c` plat_thread_sample)

Video > Capture a performance profile now also samples the process's threads for ~4 s by their own CPU time (a 1 ms
SIGPROF timer on each thread's CPU clock, only while it runs): `[threadprof]` lines give each busy thread's share of a
core, how its time splits between the game's code (botwgame.so), the runtime (libmain.so), libc, the GPU driver and
the rest, and its hottest places - libmain.so offsets readable with the CI's `libmain-symbols-<commit>` artifact.
`tools/test_thread_sampling.py` (CI) runs it against busy and sleeping threads on Linux. The fps line also counts
`write faults` - the game's threads trapping on pages the copy mode tracks (a signal and an mprotect each).

## Update 7 (0.1.9-slate.7): a steadier resolution, the memory, the whole thread list

The Update 6 log: ~26 fps at first, easing to ~19 over half a minute as the GPU's time per frame grew (the scene
lighter meanwhile), ~16 during and after the capture; Game Booster's overlay: GPU 64%, CPU 47%, **RAM 84%** at 24 fps.
And the "unstable" feel: BOTW's own dynamic resolution swinging 1600x900 -> 1280x720 -> 1600x900 within a second every
few seconds, the longest frames (280-300 ms) on those swings.

### 22. Video > Game's resolution changes (`exec.c` drs_filter)

BOTW lowers its scene's resolution from what it reads as its frame's GPU time (a counter pair). A hitch read as GPU time
took it down several steps at once. **Steadied** (default): the median of the last 9 frames' times. **Hold sharp** /
**Hold low**: a steady 20 / 40 ms - always 1600x900 / always 1280x720. **As the game does**: as before.

### 23. Memory and the capture (`exec.c` vk_memory_report, `textures.c` vk_texture_memory)

On a tablet the GPU's memory is the system's RAM. The capture logs `[gpumem]`: in use and budget, render targets,
textures (and how much of them are BC textures decoded to full size - this GPU can't sample BC), the transient ring,
the vertex cache, the arena, the process's resident memory and what the system has left.

### 24. Fixes to Update 6's measurements

- The thread sampler took the first 64 threads only - the game has more, and the render thread came later: now 256.
- The runtime's symbols are also annotations on the CI run (`libmain symbols N`), readable where artifact downloads
  aren't, to name the sampler's `libmain.so+offset` places.
- The constant-buffer copies shared across a frame's submissions (#19) measured ~3 MB of ~510 MB every 2 s: per
  submission again by default (`LARECOMP_GCOPY_PER_FRAME=1`: per frame).

## Update 8 (0.1.9-slate.8): even frame pacing, and what the next GPU savings need

The Update 7 log: 30 fps nearly throughout (29-30.3 in 2-second readings), the resolution at its top (1440x810)
except half-second dips to 1152x648 every 5-10 s - the game then waiting on the GPU (113-186 ms per 2 s, against 3-30
when steady). The CPU had room (the busiest threads 64% of a core, clusters at 1.0-1.6 GHz); the GPU took ~25-28 ms of
the 33 ms frame at full resolution. Nothing here trades image quality for speed (no 40 fps mode at a lower resolution,
no 16-bit upscaler maths, no lossy texture conversion).

### 25. Video > Even frame pacing (`device.c`, `display_pacing.c`)

At 30 fps on the 120 Hz panel a frame should stay up for 4 refreshes. It went up at the first refresh after the GPU
finished it, and that time varies by several milliseconds, so frames stayed up for 3, 4 or 5 refreshes (25-42 ms) at a
steady 30 - in a simulation with frames ready 15-30 ms after their grid time (`tools/test_display_pacing.c`) only ~56%
of frames got 4. Now each present carries an id and a time it asks not to go up before (`VK_GOOGLE_display_timing`,
Android's own swapchain): a slot of its own on a grid of refreshes learnt from when earlier frames really went up, one
refresh of slack. The game's frame grid (`framepace.c`) runs at 4 of the panel's measured refreshes rather than 1/30 s
of our clock. In the simulation: 100% of frames at 4 refreshes (99% with a slow GPU frame in 40), ~10 ms more latency.
**Automatic** (on where the system reports present times), **On**, **Off** (as before). The fps line now says how long
frames stayed up: `on screen 4x 98% (3x 1.0%, 5x 1.0%), paced: ... late N, early N, regrids N` - early frames mean the
system ignores the asked-for times. `LARECOMP_EVEN_PACING=0/1`, `LARECOMP_NO_DISPLAY_TIMING=1`.

### 26. What the next GPU savings need

- **Pass breaks** (the stats line's `breaks`, ~1 a frame in some places): the first 40 different reasons a draw ended
  its pass to read a target it draws, `[vk] pass break N: fs ... slot ... - why` (`exec.c` tile_read_why, break_note).
- **The heaviest small passes' shaders**: one draw into a 200x112 target took 3.3 ms. The capture now logs the
  fragment shader of its three heaviest passes of 1-4 draws (0.5 ms or more), with what they sample, as `[glsl]` lines
  (`exec.c` gp_dump_shaders; six a run at most).
- **Textures by format** (`[gpumem] textures by format`): the decoded BC textures by kind (BC1 is decoded to 8x its
  size), the ASTC ones, the rest - to see what a lossless or near-lossless conversion would save.

### 27. The profiler names more, and who called

- Each sample keeps the link register and three frames of return addresses (frame pointers, read with
  `process_vm_readv` so a bad pointer can't fault): per thread, `libc called from:` puts its libc / vDSO time (locks,
  system calls, memcpy, clock reads) down to the runtime or game function that called.
- Places are named from each module's dynamic symbols with their sizes (dladdr named any address after an exported
  function by it - `libc.so!syscall` may have been other code), libc's string functions by the implementation chosen,
  and the game's code as `botwgame.so!f_<address>` (its own addresses: 0x7000000000 + offset in main).
- CI also commits the runtime's symbols to the branch `symbols` (`libmain-<commit>.txt`).

## Update 9 (0.2.8-slate.9): upstream 0.2.8 underneath, a graphics preset for the tablet

Upstream's 0.2.8-beta is now the base (a three-way merge from 0.1.9-beta): game version 1.9.0, **saves import and
export** (start screen > Settings > Saves: a .zip exported here or any holding 0, 1, 2... and option.sav; Import from
folder takes an emulator's save folder; the saves before are kept beside them), the DLC, game files used in place (no
16 GB copy), Native Mode (experimental), Sync lag, 60 fps cutscenes, eleven menu languages, the game code build going on
with the screen off, and upstream's own Mali work - the arena cached with hand flushes and invalidations (the same idea
as this port's Update 4: upstream's version kept), chained compute dispatches, render targets Mali can compress (AFBC),
copy-range learning and idle transient chunks handed back (upstream's fix for Mali-G720 crashes after minutes). The
port's per-frame copy sharing (Update 6, measured useless) is gone; everything else of Updates 1-8 is on top.

**The game code id changes** (0.2.8's generator: 65da5d95e597ba58): the first start builds the game code again, once
(about an hour on the tablet, screen off is fine) - or Video > Native game code (experimental; upstream hasn't tested
it with 1.8.2).

### 28. Video > Graphics preset: Tab S10 Ultra (`settings.c` tab_preset_into)

From the logs of Updates 4-8: 30 fps (the GPU took 25-28 ms at the top resolution, ~20 at the lowest: 60 is out of
reach, 30 held), Display refresh rate Highest with Even frame pacing (4 refreshes a frame), Internal resolution 1x with
the game's resolution changes Steadied (1440x810 most of the time, half-second dips in busy moments), Output resolution
Auto (1080 lines on the 1848-line panel), AMD FSR 1 (the sharpest; ~2 ms at 1080 lines, within the frame's room), no
extra anti-aliasing, overlapping passes and 32-bit shader math automatic, Sync lag on. The defaults on the tablet;
applied once over earlier settings on the first start of this version (`tab_preset = 9` in sheikahslate.ini); the menu
shows Custom once any of them is changed, and choosing Tab S10 Ultra puts them all back. Tested on the desktop
(`tools/test_tab_preset.c`).

### 29. The GPU's time in Android's performance hints (`exec.c` vk_pace_frame)

On Arm GPUs the frame's GPU time now goes into the performance hint session (API 35+), scaled to the frame's own
budget: at 30 fps a GPU frame within 90% of its 33 ms reads as on target, a longer one as over - so the governor raises
the GPU's clock where frames are about to miss (where the game lowered its resolution in the logs), not for every frame.
`LARECOMP_ADPF_GPU=0`: off; `=1`: the raw time (upstream's test).

## Update 10 (0.2.8-slate.10): 60 fps unblocked, Battery / Performance / Turbo, 40 fps

Same game code id (65da5d95e597ba58): no rebuild of the game code.

### 30. Frame rate 60 stayed at 30 (`exec.c` pace_end_counter, drs_rate_scale)

At Frame rate 60 the game's GPU counters were capped at 8 ms (upstream's choice for Adreno, where its 60 fps frames
fit): the game read every frame as quick, kept its resolution at the top, and on the Mali a frame took 25-28 ms - the
pacer's 60 fps grid restarted every frame and the game sat at ~30-36. On Arm GPUs the cap is gone; instead, at 40 or
60 fps, the GPU time the game's own dynamic resolution reads is scaled to the frame it has to fit (its budget is its
present interval's, 33 ms in its 30 fps way: x1.33 at 40, x2 at 60), after the Steadied filter - so it lowers its
resolution until a frame fits. At 60 the Tab still lands under 60 (~20 ms a frame at the game's lowest resolution in
the logs), unevenly; the menu says so. `LARECOMP_FAST_COUNTERS=1` puts the old cap back.

### 31. Video > Mode: Battery, Performance, Turbo (`settings.c` mode_into, `menu.mm`)

The Graphics preset is replaced by three modes, at the top of the Video tab:

| | Battery | Performance (default) | Turbo |
| --- | --- | --- | --- |
| Frame rate | 30 | 30 | 40 |
| Display refresh rate | 60 Hz (2 refreshes a frame) | Highest, 120 Hz (4 a frame) | Highest, 120 Hz (3 a frame) |
| Game's resolution changes | Hold low (its lowest resolution) | Steadied | Steadied, aimed at 25 ms |
| Upscaler | Snapdragon GSR 1 (one pass) | AMD FSR 1 (the sharpest) | Snapdragon GSR 1 |

All three: Internal resolution 1x, Output resolution Auto (1080 lines), no extra anti-aliasing, Even frame pacing,
overlapping passes and 32-bit shader math automatic, Sync lag on, cutscenes at 30. Performance is Update 9's preset
(already applied once over earlier settings: it isn't applied again). Changing any of these options shows Custom;
choosing a mode sets them all. Tested on the desktop (`tools/test_modes.c`).

### 32. Frame rate 40 (`framepace.c`, `settings.c`)

Frame rate gets 40 fps on 1.8.2 and 1.9.0 (`frame_rate = 40` in sheikahslate.ini). The pacer's grid is 25 ms - on
the 120 Hz panel exactly 3 refreshes, locked to the panel's real period with Even frame pacing - and the game's clock
steps 0.75 of a console frame each frame, staying in the game's 30 fps way (its interval switches below 0.7). In the
pacing simulation (`tools/test_display_pacing.c`): 100% of frames up for 3 refreshes paced, 63% without. On a 60 Hz
screen 40 isn't a whole number of refreshes: it isn't paced there (Battery stays at 30).

## Update 11 (0.2.8-slate.11): the frame pacer stayed off; Turbo 60; your own background and icon

Same game code id (65da5d95e597ba58): no rebuild of the game code.

### 33. Frame rate 40 / 60 did nothing: the frame pacer was off all along (`framepace.c` framepace_on)

The Galaxy S25's log (2026-11-01, 0.2.8-slate.10, Frame rate 60): 30.0 fps in every 2-second line, each frame up for
4 refreshes of 120 Hz, and "not paced" though Even frame pacing was on. `framepace_on()` - is the runtime pacing the
game and setting its clock - kept its first answer, and since Update 8 the first question came from the start
screen's presents (Even frame pacing asks it), before any game was loaded: version 0, so no. Every game after ran with
the game's own 30 fps pacing (its present interval of 2 vsyncs), whatever Frame rate said - on the tablet too. It now
reads the game's version each time.

### 34. The resolution follows the GPU at 40 and 60 fps (`exec.c` drs_rate_follow)

Update 10 scaled the GPU time the game's own dynamic resolution reads by a fixed budget / frame. Now the factor follows
the GPU's real time a frame, twice a second: x1.08 while it averages over 92% of the frame, /1.08 under 75%, between 1
and 3, starting at 1 - the resolution comes down only as far as the frame rate needs, wherever the game's budget
really is. The 8 ms counter cap at 60 is gone with the frame pacer on any GPU (it only fed the game's resolution: on a
GPU where frames fit, the factor stays at 1 and so does the top resolution). In a simulation against a model of the
game's resolution (budget 33 or 16.7 ms, a GPU taking 10-28 ms a frame at the top): 60 held wherever the game's
lowest resolution allows it, and the top kept where frames fit; starting at budget / frame took it lower than needed
(68% of the pixels where 77% held 60). `[dynres] the game's resolution for 60 fps: ...` in the log.

### 35. Video > Mode: Turbo 60

Turbo at 60 fps: 2 refreshes of 120 Hz a frame, the resolution lowered as far as 60 needs, Snapdragon GSR 1. On the
Tab's Mali (~20 ms a frame at the game's lowest resolution in the logs) it may not hold 60; on the Galaxy S25's Adreno
830 there is room. `tools/test_modes.c` covers it.

### 36. Settings > Look: your own background and home screen icon (`launcher.mm`, `SheikahSlateActivity.java`)

The start screen's Settings, beside Saves and DLC:
- **Choose background**: a picture from the photo picker, decoded at the screen's size and saved as
  `files/look/background.png`; the start screen fills itself with it, cut to its shape. **Original background** puts
  the artwork back. (The old texture is freed: `imgui_glue.cpp` vk_imgui_texture_free.)
- **Choose icon picture**: Android doesn't let an app swap its own launcher icon for a picture, so this pins a home
  screen icon with the picture that opens the app (the system asks once: Add), and changes that icon's picture after.
  The picture is cut square and fitted inside the adaptive icon's visible 72 of 108 dp, a blurred copy of it around.
  The original icon can then be removed from the home screen (the app list keeps it).

## Update 12 (0.2.8-slate.12): the app's real icon, chosen in the app

### 37. Settings > Look > App icon (`AndroidManifest.xml`, `GameActivity.java`, `SheikahSlateActivity.setAppIcon`)

Update 11's icon was a home screen shortcut with a picture beside the app's own; now the app's real launcher icon - on
the home screen and in the app list - changes. Android lets an app switch its launcher entry among the ones in its
manifest, not give it a picture at run time, so the icons are built in: the original and four more original designs
(`tools/make_app_icons.py`: Midnight, Dawn, Ancient, Forest - no Nintendo artwork or emblems), each an
activity-alias of the game's activity, one enabled at a time. The activity itself is now `GameActivity` (the code is
still SheikahSlateActivity's, the class it extends, and the JNI names with it); the original icon's alias keeps the
old name `com.sheikahslate.SheikahSlateActivity`, so a home screen icon from before stays through the update.

Choosing an icon (a picture each, the one in use framed in gold; then "Use it") enables its alias first, disables the
others, and starts the app again from it straight away - before the system closes what the old entry opened, and with
no second copy of the game's activity left beside it. The home screen may drop the old icon: the new one is in the app
list. The long-press shortcuts follow every icon (they name `GameActivity`), and Update 11's picture icon, if one was
added, now opens `GameActivity` too. The Choose icon picture button is gone. CI fails the build without the launcher
entry, the aliases or their icons. A picture of your own as the real icon has to be built into the APK.

## Update 13 (0.2.8-slate.13): the Glass Slate start screen, the player's own icon

### 38. The Glass Slate start screen (`launcher.mm` glass_main)

Concept A of the UI concepts, built in on Android: a glass panel down the left over the background - the app's icon in
use and its name, the game's and the app's version, PLAY, Saves (opens Settings > Prelaunch, where they are), Mods
(Settings > Mods), Settings, Set up game files while it's needed, Quit; under them the game files' state, the GPU and
its driver (a custom one by its folder's name) and how to reach the menu in game. At the top right: the display's
refresh rate, Even frame pacing, the battery and its temperature. At the lower right a card: the last save's time
(the newest `game_data.sav` of this version), Resume (Start without one), and Video > Mode's four modes a tap away -
the one in use outlined, Custom when the options match none, Turbo and Turbo 60 off for 1.0.0 / 1.6.0. Laid out in
units of an 800-line screen, kept clear of a camera cutout (the window's safe area), every item reachable with a
controller (PLAY focused first). Typeface: Inter Bold and Medium (OFL, `third_party/fonts/Inter`; the concept's
display face couldn't be fetched here). The first PLAY's questions and the game's start are as before; the desktop
start screen is unchanged.

### 39. The app's icon is the player's picture

The player's picture (an overgrown sword with flowers) is the app's icon - `assets/artwork/app-icons/app-icon-sword.png`,
cut to its drawing and centred on a white rounded square - as the default launcher entry, so the update changes the
icon on the home screen without a step. The original slate design is one of the others in Settings > Look > App icon
("Slate", alias `IconSlate`), now six: Sword, Slate, Midnight, Dawn, Ancient, Forest.

## Update 14 (0.2.8-slate.14): the app is WSword; Settings in the Glass Slate look

### 40. The name: WSword

The app shows itself as **WSword**: its launcher entries' labels (every icon), the start screen, Settings, messages and
notifications, in every menu language. What doesn't show stays, so the update installs over the app with everything
kept: the application id `com.sheikahslate.tab`, the signing key, the classes and JNI names, `sheikahslate.ini`, the
data folders. The game's own item, the Sheikah Slate (Controls > Motion controls' text), keeps its name.

### 41. Settings in the Glass Slate look (`menu.mm` tabs_body, `option_rows.mm`)

The Settings window (the start screen's and the in-game menu's) as the start screen: deep navy glass with a teal edge
and rounded corners. A rail down the left holds the app's name over "Settings", the pages as tiles - an icon and the
name, the open one filled and outlined in cyan - and BACK at its foot; the open page is beside it under its own title,
RESTORE DEFAULTS at the title's right on Video and Audio. Rows are parted by thin navy rules, the selected one a navy
tile outlined in cyan, a changed value in light cyan; section titles in light cyan small capitals; the description in a
glass card beside the rows. The gold cut-corner frames, diamonds and the eye are gone. Type: Inter Medium / Bold at
Baloo 2's letter height there (x0.72), the menu language's other letters merged at 90% of it. Up / Down move along the
rail, Right into the page, L / R switch pages as before.

## Update 15 (0.2.8-slate.15): frame generation with your own Lossless.dll

### 42. Video > Frame generation (`device.c` fg_present, `framegen.cpp`, `third_party/lsfg-vk`)

Lossless Scaling's frame generation (LSFG), with the player's own `Lossless.dll`. None is included with WSword.

**Adding the DLL.** Start screen > Settings > **Lossless Scaling** > Choose Lossless.dll:
- The file is copied to `files/lsfg/`.
- It is kept only if it holds every frame generation shader this version uses: the full-precision set, the FP16 set, or both (`fg_check_dll`).
- *Remove Lossless.dll* deletes the copy.

**Running it.** The engine is the backend of lsfg-vk, by PancakeTAS and contributors:
- It reads the shaders' SPIR-V straight out of the DLL's resources.
- It runs them as compute passes on WSword's own device and queue: no second device and no shared memory.
- It is the last GPL-3.0 version (PR #544, `f4f7444`, on `8b0da26`). Later versions are CC BY-NC-ND, which forbids modified copies. `third_party/lsfg-vk/README.WSword.md` lists WSword's changes.

**Each real frame, while it is on:**

1. The game's final image is copied, byte for byte, into the backend's source image for this frame (`source[n % 2]`).
2. That submission signals the backend's timeline semaphore.
3. The backend's passes make 1–3 frames between the frame before and this one.
4. Each of those frames is presented first:
   - drawn as the game's image is (same placement, crop and post passes);
   - with the overlay drawn again on top (`menu_render_again`), so the FPS counter and touch controls are never generated;
   - waiting on the GPU for its own semaphore value.
5. Then the real frame is presented.

**Presents.**
- Every present is aimed at fps × (1 + the generated frames) per second through `VK_GOOGLE_display_timing`, whether or not Even frame pacing is on.
- The swapchain is FIFO, with one more image per generated frame, so the presents in a row don't wait for each other.
- The ring of frames in flight grows by the same number. Without frame generation everything is as before (MAILBOX, 3 frames in flight).

**Choices on a 120 Hz screen:**

| Setting | Frame rate | Shown |
|---|---|---|
| 2x | 30 or 60 fps | 60 or 120 |
| 3x | 40 fps | 120 |
| 4x | 30 fps | 120 |

**Other settings.**
- *Flow scale* (25–100%, default 75%): the resolution motion is found at.
- *Performance mode*: Lossless Scaling's lighter passes.
- FP16 shaders are used where the GPU has `shaderFloat16`. That feature, and the storage-image formats the shaders write, are enabled only for a player who has the setting on or has added a DLL.

**Costs and limits.**
- **Delay:** about half a real frame at 2×, because the frames in between need the next real one.
- **GPU time:** the game's resolution may come down a little, because the GPU time it measures includes frame generation.
- **Menu:** frame generation is paused while the in-game menu is open.
- **Failures:** if it can't start (no DLL, a format the GPU can't copy, a driver that can't build the shaders), it stops with the reason on the Video page and the game carries on. It tries again once the setting changes.

**Untested GPU:** the Mali-G720 is untried with this engine. Adreno with Turnip is what lsfg-vk's Android users run.

**Log lines:** `[framegen]`. `LARECOMP_FRAMEGEN=0..3` overrides the setting; `LARECOMP_FG_FP32=1` forces full precision.

## Update 16 (0.2.8-slate.16): frame generation's frames evenly on screen

### 43. A frame's group of presents paced as one (`display_pacing.c`, `device.c` fg_present)

**What went wrong on the Galaxy S25 (slate.15).** Frame generation ran (`[framegen] started: Lossless.dll's FP16 shaders`), but it didn't look any smoother. At 2x and 30 fps the stats line read:

> on screen 4x 42% (1x 26.6%, 3x 26.6%), … not shown …

So each generated frame was either:
- up for **one** refresh, with its real frame then up for three; or
- never shown at all, overtaken by its real frame at the same refresh.

**Why.** Even pacing judged every present on its own:
- **The real frame kept the grid in place.** The generated frame needs the most GPU work before its slot, so it was the one that came late. But the real frame, a slot after it, was on time, and that on-time record held the grid where it was.
- **Lateness was taken for an offset.** The generated frame's one-refresh lateness, the same every frame, was learnt as a reporting offset ("offset 8.3 ms").
- **The group looked too far ahead.** Presents that come together looked more than a frame ahead of their slots, so the grid was dropped and started again: "regrids 121, ahead 119".

**The fix.** `fg_present` tells the pacer each present's place in its frame's group (`burst_pos`, `burst_n`). Then:
- **Judged by the first present.** The grid follows only the group's first present, the one with the least time to be ready. The group is never read as an offset; two late first presents in a row start a new grid from it.
- **"Ahead" measured per group.** A frame is too far ahead only when it's more than a real frame's time ahead, plus its place in the group.
- **The menus are unchanged.** Pacing without frame generation behaves exactly as before.

**Simulation** (`tools/test_display_pacing.c`, new cases): with each generated frame taking 3 ms of GPU time, the slate.15 behaviour reproduces the S25's mix. With the fix:

| Case | Up for | Overtaken |
|---|---|---|
| 30 fps × 2 | 2 refreshes each, 100% | none |
| 30 × 4, 40 × 3, 60 × 2 | 1 refresh each, 100% | none |

**Also in this update:**
- **The overlay** shows the frames presented per second with frame generation, beside the game's own rate:
  - compact: `FPS 30 > 60`
  - full: "60 with frame generation 2x"
- **The `[framegen]` line** is logged every 10 s even while frame generation is paused.
- **A content check in the log.** After the 240th generated frame shown, then every 3600, `[framegen] check` compares the middle of a generated frame with the real frames before and after it. A frame that is black, or a copy of one of the two, is named as such.

## Update 17 (0.2.8-slate.17): frame generation at its least GPU time

### 44. Nothing made that isn't shown; the motion search at the resolution it needs (`device.c` fg_present, `display_pacing.c`)

**Never more frames than the screen shows** (`dp_fg_frames`). The multiplier in use is the one chosen, lowered:
- **to the display's refresh rate.** Frames past it were made and never went up. For example, 60 fps at 4x on 120 Hz made three frames per real one and showed one.
- **to a whole number of refreshes per frame, where one fits.** 30 fps at 3x is 90 frames a second on 120 Hz, which is uneven, so it becomes 2x.

| On 120 Hz | Becomes | Why |
|---|---|---|
| 30 fps × 3 | 2x | 90 a second is uneven |
| 40 fps × 4 | 3x | 160 is past 120 |
| 60 fps × 4 | 2x | 240 is past 120 |
| 60 fps on a 60 Hz screen | off | nothing to add |

**Auto (30 / 60) frame rate.** The context is made for the game's lowest rate. When the game is at 60, only some of its passes run: a 4x context makes 2x's frame with its middle pass (`dp_fg_subset`, the backend's `scheduleFrames(context, which)`). So there is no second context, and no pause while one is made, when the rate moves.

**Flow scale: Automatic** (the new default). This follows Lossless Scaling's own advice by the image's height, about 1080 lines of motion:

| Image height | Flow scale |
|---|---|
| 1080 lines | 100% |
| 1440 lines | 75% |
| 1848 lines (the Tab's) | 60%, roughly a third of 100%'s motion-search work |

Slate.15 always wrote its default, 75, so that value is read as Automatic. Any other value is kept.

**Less waiting on the GPU in WSword's own steps:**
- **After the copy:** no barrier. The submission's signal already makes the copy visible to the passes.
- **Between the presents:** barriers only when the present runs passes of its own (Video > Anti-aliasing).

**Its cost, measured.** Timestamps record when the game's frame, the copy and frame generation's passes finish, read 4 frames later:
- **Overlay:** "60 with frame generation 2x (GPU 2.1 ms)".
- **Video page:** the status line has it.
- **Log:** every 10 s, `[framegen] … GPU x ms a real frame (the copy y)`.

**Tests:** `tools/test_display_pacing.c` has the multiplier table and the subsets. `tools/test_modes.c` has the flow scale's Automatic and the slate.15 key.

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
8. **Modes.** Video > Mode: Battery, Performance, Turbo and Turbo 60 each set the options below them (Custom after
   changing one). Turbo: the overlay reads ~40 fps; Turbo 60: above 40, `[dynres] the game's resolution for 60 fps`
   lines as it settles. The fps lines no longer say "not paced" with Even frame pacing on.
9. **Look.** Settings > Look > Choose background: the start screen shows the picture at once; Original background puts
   the artwork back. App icon: tap a picture, Use it - the app starts again, and the app list shows the new icon (put
   it back on the home screen if it went).
10. **Frame generation.**
    1. Start screen > Settings > Lossless Scaling > Choose Lossless.dll. Expect "Lossless.dll added".
    2. In game, Video > Frame generation 2x at Frame rate 30.
    3. Check the log:
       - `[framegen] started: Lossless.dll's FP16 shaders ...`
       - `[framegen] 2x at ...`
       - every 10 s, `[framegen] 2x: N frames made, N shown`
    4. Check what you see: motion smoother than plain 30 fps; the FPS counter steady, not smeared.
    5. Open the menu: the status says Paused. Close it: frame generation picks up again.
    6. With the overlay on: "30 FPS" and "60 with frame generation 2x" (compact: `FPS 30 > 60`). The stats line reads
       "on screen 2x" near 100% (4x at 30 fps x 4: "1x"). `[framegen] check` gives two differences of about half "those
       two" for a moving picture.
    7. If the Video page says Stopped, send the log.

Switches for testing (adb, as upstream's `android/run_boot.sh`):

```bash
adb shell am start -n com.sheikahslate.tab/com.sheikahslate.GameActivity --es env LARECOMP_VK_HOST_IMPORT=0
adb logcat -v time SheikahSlate:V '*:S'
```

## What wasn't changed, and why

- **Performance defaults** (30 fps, 1x internal resolution, Snapdragon GSR 1, screen-resolution output) stay as
  upstream chose them: they would need measurements on the Tab to change for the better.
- **Pre-rotation** of the swapchain: the compositor still turns the image if the panel's native orientation differs;
  untestable without the device.
- **Split screen and pop-up view** stay possible; a window narrower than 16:9 is letterboxed like the full screen.
- **The update check** stays off on Android, as upstream had it; its URLs point at a placeholder `OWNER/SheikahSlate`.
