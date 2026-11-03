# Building WSword for Windows

From 0.3.0, the PC build is built from **`pc/`**: Savage Wind Recomp 0.4.1-alpha.4 (MIT), with WSword's changes
([pc/WSWORD.md](../pc/WSWORD.md)). Its own build guide, [pc/docs/BUILD-windows.md](../pc/docs/BUILD-windows.md), applies
from `pc/`.

## The release zip

`WSword-<version>-windows.zip` holds:
- `WSword.exe`, its import library `WSword.dll.a`, and `WSword Configuration.exe`;
- the frozen game code generator (`swgen/`), and a slimmed zig for building the game code on the player's PC (`zig/`);
- the headers that code is compiled against (`gamelib-src/`);
- the fonts (Fira Sans, Inter, Noto Sans Thai), the start screen's art and icons, and the DLLs they load;
- `wsword_dlss.dll`, WSword's bridge to NVIDIA's DLSS SDK (`pc/tools/dlss_bridge`), and the SDK's licence `NVIDIA-DLSS-LICENSE.txt`;
- `LICENSE.txt` (Savage Wind's), `SavageWind-README.md` (the components' licenses), `lsfg-vk-LICENSE.md` / `lsfg-vk-README.md` (Lossless frame generation's backend, GPL-3.0) and `WSword-PC-guide.md` (this repository's `docs/PC.md`).

**No game code, no game files, no `Lossless.dll` and no `nvngx_dlss.dll`.** The CI checks this.

## On GitHub (what releases use)

`.github/workflows/build-windows.yml` runs on every push to `main` and `pc-*` (MSYS2 UCRT64 with clang, on `windows-2022`):
1. builds the LGPL FFmpeg with the root's `make deps-windows` (cached), copied into `pc/build/deps-win`;
2. fetches NVIDIA's DLSS SDK (https://github.com/NVIDIA/DLSS: its headers, `nvsdk_ngx_s.lib` and licence) into `pc/build/dlss-sdk` and builds `pc/build/wsword_dlss.dll` with MSVC (`cl`, from the runner's Visual Studio);
3. in `pc/`: `make gen-helper`, `tools/fetch_zig.sh` and `make dist`.

The zip is the run's `WSword-windows` artifact. Started by hand with a `release` tag, the run also publishes the zip as a GitHub release, with the notes from the `release_notes` file.

## On Windows

1. Install [MSYS2](https://www.msys2.org) and open the **UCRT64** shell.
2. Install the packages:
   ```sh
   pacman -S --needed make git curl tar xz diffutils zip unzip \
       mingw-w64-ucrt-x86_64-{clang,lld,binutils,sdl3,ffmpeg,vulkan-devel,shaderc,spirv-tools,pkgconf,vulkan-memory-allocator,nasm,zlib}
   ```
3. A python.org Python 3.12 for the generator helper (PyInstaller freezes it from its own venv).
4. In `pc/`:
   ```sh
   make deps-windows                       # LGPL FFmpeg for the game's movies (build/deps-win, once)
   make gen-helper GEN_PY=/c/Python312/python.exe
   tools/fetch_zig.sh                      # zig 0.17.0, pinned
   make -j$(nproc) dist                    # build/dist/WSword-<version>-windows.zip
   ```
   `make` alone builds `build/WSword.exe` for development: a console window, with the log on it.
5. DLSS (optional): build `wsword_dlss.dll` with MSVC as [pc/tools/dlss_bridge/README.md](../pc/tools/dlss_bridge/README.md) describes, into `pc/build/`, before `make dist`. Without it the zip has no DLSS.

## The Android build

The root `Makefile`'s Windows target (the LAS-based 0.2.8-pc.x) is no longer built. The Android app is built from the root's `runtime/` as before (`build-apk.yml`).
