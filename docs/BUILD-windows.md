# Building WSword for Windows

The PC build comes from the repository's `Makefile` (LAS Recomp's desktop build, MIT, made for WSword). The release
zip holds `WSword.exe`, its import library `WSword.dll.a`, the frozen game code generator (`botwgen/`), a slimmed zig
for building the game code on the player's PC (`zig/`), the headers that code is compiled against (`gamelib-src/`), the
fonts and art, and the DLLs they load. **No game code, no game files and no Lossless.dll** - the CI checks.

## On GitHub (what releases use)

`.github/workflows/build-windows.yml` runs on every push to `main` and `pc-*`: MSYS2 UCRT64 with clang on
`windows-2022`, then the desktop tests (frame pacing - the FIFO model of fill pacing too - and the Lossless.dll
check), then `make dist`. The zip is the run's `WSword-windows` artifact.

## On Windows

1. Install [MSYS2](https://www.msys2.org) and open the **UCRT64** shell.
2. Packages:
   ```sh
   pacman -S --needed make git curl tar xz diffutils zip unzip \
       mingw-w64-ucrt-x86_64-{clang,lld,binutils,sdl3,vulkan-devel,shaderc,spirv-tools,pkgconf,vulkan-memory-allocator,nasm}
   ```
3. A python.org Python 3.12 for the generator helper (PyInstaller freezes it from its own venv).
4. In the repository:
   ```sh
   make deps-windows                       # LGPL FFmpeg for the game's movies (build/deps-win, once)
   make gen-helper GEN_PY=/c/Python312/python.exe
   tools/fetch_zig.sh                      # zig 0.17.0, pinned
   make -j$(nproc) dist                    # build/dist/WSword-<version>-windows.zip
   ```
   `make` alone builds `build/win/WSword.exe` for development (a console window, the log on it).

## From Linux (compile and link checks)

With [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) (ucrt) and a prefix holding SDL3's mingw headers and import
library plus import libraries for `vulkan-1` and `shaderc_shared`:

```sh
make WINCROSS=<llvm-mingw>/bin/x86_64-w64-mingw32- DEPS=<prefix> NO_FFMPEG=1
```

## What's Windows-specific

- `runtime/platform/windows.c`: the log next to the exe, crash reports, threads, `plat_device_stats` for the overlay.
- `runtime/ui/slate_ui.h`: `WS_SLATE_UI` - the Glass Slate UI on Android and Windows.
- `runtime/ui/menu.mm`: Video > Display mode (`menu_apply_display_mode`), Controls > Mouse.
- `runtime/ui/launcher.mm`: Settings' pickers through SDL's Windows dialogs (Lossless.dll, background, DLC, mods).
- `runtime/hle/input.c`: the mouse camera.
- `runtime/nvn/vulkan/device.c`: VK_GOOGLE_display_timing loaded from the device where the driver has it, fill pacing
  (`fill_k`, `fill_swapchain_burst`) where it doesn't, frame generation's files in the data folder.
