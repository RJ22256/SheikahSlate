**WSword for the Samsung Galaxy Tab S10 Ultra** (Sheikah Slate 0.2.8-slate.18, beta). The Android build of the Breath of the Wild recompilation, tuned on the tablet from its own logs.

No game files are included. Bring your own dump of the game (1.8.2 or 1.9.0 for 60 fps) and, for frame generation, your own `Lossless.dll` from Lossless Scaling.

### Installing

Install the APK over the app you have. It has the same id and key, so your game files, saves and settings stay, and the game code already built on the tablet stays valid. On a first install, the start screen walks you through setting up the game files.

### What's new

- **Two modes** (Video > Mode, and the start screen's card):
  - **Performance** (the default): 60 fps, each frame up for exactly 2 refreshes of the 120 Hz screen. The game's resolution is lowered only as far as 60 needs, with the Snapdragon GSR 1 upscaler to a 1080p image.
  - **Quality**: 30 fps, each frame up for exactly 4 refreshes. The game's native 900p is held, never lowered, and AMD FSR 1 scales it to the screen's own 2960 x 1848.

  On the first start, Performance is applied once over your earlier settings.
- **Even frames at 60 fps.** Frame pacing broke down at 60: the timing grid was rebuilt on nearly every frame, so frames went up for 1, 2 or 3 refreshes even at a steady 60. Fixed, and covered by a simulator test.
- **Frame generation that fits the GPU.** At 60 fps the Mali's GPU is already full (15.5-16.9 ms of the 16.7 a frame), so frame generation used to pull the game down to 36-52 fps.
  - **In Performance**, the game runs at 120 Hz divided by the multiplier: 2x at 60, **3x at 40** (the best fit), 4x at 30. Every made frame lands on its own refresh.
  - **In Quality**, use 2x (60 shown) or 4x (120). The image is 1080p while frame generation is on.
  - **Lighter frame generation on the Mali:** the Automatic flow scale is 25%, performance mode is on by default, and the faster FP16 shaders are used even when Lossless.dll is added after the app has started. Before, 100% flow took 20-62 ms of GPU time per real frame; 25% takes about 4.5 ms.
- **Fewer frames over budget at 60.** The game's resolution starts coming down a little earlier (86% of the frame instead of 92%).

### If something's off

Turn on Settings > Prelaunch > Driver compatibility > Save a log file, play a few minutes in each mode, and send `sheikahslate-log.txt`. The details are in [docs/TAB-S10-ULTRA.md](https://github.com/RJ22256/SheikahSlate/blob/v0.2.8-slate.18/docs/TAB-S10-ULTRA.md), Update 18.
