# wsword_dlss.dll

WSword's bridge to NVIDIA DLSS (Super Resolution) for its Vulkan renderer. The runtime (`runtime/nvn/vulkan/dlss.c`) loads it; the C interface is `runtime/nvn/vulkan/dlss_bridge.h`.

- **Why a DLL of its own:** NVIDIA's NGX library comes as an MSVC static library (`nvsdk_ngx_s.lib`), which the MinGW-built `WSword.exe` can't link. So this file is built with MSVC and linked against it.
- **Built by** `.github/workflows/build-windows.yml`:
  - NVIDIA's DLSS SDK is fetched from <https://github.com/NVIDIA/DLSS>: its `include/`, `lib/Windows_x86_64/x64/nvsdk_ngx_s.lib` and `LICENSE.txt`.
  - It is compiled with `cl /MT` (the static CRT, as the library is).
  - `make dist` puts it in the zip with the SDK's licence (`NVIDIA-DLSS-LICENSE.txt`).
- **Not included:** the DLSS model, `nvngx_dlss.dll`. It is the player's own: Settings > Files > NVIDIA DLSS copies it next to `WSword.exe`, where NGX looks for it.

By hand, from a "x64 Native Tools" prompt in `pc/`:

```bat
cl /nologo /O2 /MT /EHsc /LD /std:c++17 /I <DLSS>\include /I <Vulkan-Headers>\include tools\dlss_bridge\wsword_dlss.cpp ^
   /Fe:build\wsword_dlss.dll /Fo:build\ /link <DLSS>\lib\Windows_x86_64\x64\nvsdk_ngx_s.lib advapi32.lib user32.lib ^
   shell32.lib ole32.lib version.lib setupapi.lib cfgmgr32.lib gdi32.lib
```
