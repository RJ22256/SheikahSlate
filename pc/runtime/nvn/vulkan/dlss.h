// (WSword) NVIDIA DLSS Super Resolution for the 3D scene at its composite (exec.c dlss_scene, Video > Upscaler DLSS),
// as the FSR 3 upscaler (fsr3.h) is: this frame's scene, depth and motion vectors at the render size in, the scene at
// its size in the window out. Through wsword_dlss.dll (dlss_bridge.h), Windows only; the player's own nvngx_dlss.dll
// next to WSword.exe. Render thread, under the GPU lock.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

#include "dlss_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

// The bridge loaded (at startup: before the Vulkan instance, whose extensions it names). Windows only.
bool dlss_bridge_loaded(void);
// The extensions DLSS needs that the instance / device should enable: names into out[max], the count (0: none)
int dlss_instance_extensions(char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max);
int dlss_device_extensions(VkInstance instance, VkPhysicalDevice phys, char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max);
// The device was made with them (device.c), and an NVIDIA RTX GPU
void dlss_device_ready(bool extensions_on);
// Video > Upscaler can offer DLSS: the bridge, an RTX GPU with the extensions, nvngx_dlss.dll there, no failure yet
bool dlss_available(void);
// Where nvngx_dlss.dll is looked for first (next to WSword.exe), and whether one is there (any of the places)
const char *dlss_dll_path(void);
bool dlss_dll_present(void);
// Video > Upscaler's status line: why DLSS isn't offered, or what it does
void dlss_status(char *out, size_t n);
// DLSS into e->cb (NGX started the first time). false when it failed: the scene goes on without it (and DLSS is
// offered no more this run, with the reason in dlss_status).
bool dlss_dispatch(WsDlssEval *e);
void dlss_shutdown(void);

#ifdef __cplusplus
}
#endif
