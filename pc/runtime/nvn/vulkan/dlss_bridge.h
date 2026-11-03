// (WSword) NVIDIA DLSS through a bridge DLL of its own: wsword_dlss.dll (tools/dlss_bridge/wsword_dlss.cpp), built
// with MSVC against NVIDIA's DLSS SDK (its NGX library is MSVC's: this MinGW-built runtime can't link it), loaded by
// vulkan/dlss.c. Plain C across it: Vulkan handles, numbers, strings. The DLSS model itself, nvngx_dlss.dll, is the
// player's own - next to WSword.exe (or in its dlss folder), where NGX looks for it; it doesn't ship with WSword.
#pragma once
#include <stdint.h>
#include <wchar.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WSDLSS_ABI 2  // (wsdlss_abi: the bridge and the runtime agree on these structures)

typedef struct {
    VkInstance instance;
    VkPhysicalDevice phys;
    VkDevice dev;
    PFN_vkGetInstanceProcAddr gipa;
    PFN_vkGetDeviceProcAddr gdpa;
    const wchar_t *data_path;       // NGX's own files (its logs)
    const wchar_t *dll_paths[4];    // more folders to look for nvngx_dlss.dll in (NULL: none; the exe's folder always)
    const char *engine_version;     // WSword's version
    void (*log)(const char *line);  // NGX's messages, into WSword.log (NULL: none)
    void (*device_idle)(void);      // before a feature is let go: nothing of the queue in flight (vk_fg_wait_idle)
} WsDlssInit;

typedef struct {
    VkImage image;
    VkImageView view;
    VkFormat format;
    uint32_t width, height;
} WsDlssImage;

enum { WSDLSS_Q_ULTRA_QUALITY, WSDLSS_Q_QUALITY, WSDLSS_Q_BALANCED, WSDLSS_Q_PERFORMANCE, WSDLSS_Q_ULTRA_PERFORMANCE,
       WSDLSS_Q_DLAA };
// The DLSS model (NGX's render presets, nvsdk_ngx_defs.h NVSDK_NGX_DLSS_Hint_Render_Preset): 0 NVIDIA's default for the
// mode (with a 310.5+ nvngx_dlss.dll: K for DLAA / Quality / Balanced, M for Performance, L for Ultra Performance), E the
// CNN (the fastest), K the transformer, L / M the second transformer (heavier, made for RTX 40 and newer)
enum { WSDLSS_PRESET_DEFAULT = 0, WSDLSS_PRESET_E = 5, WSDLSS_PRESET_K = 11, WSDLSS_PRESET_L = 12, WSDLSS_PRESET_M = 13 };

typedef struct {
    VkCommandBuffer cb;              // recording; DLSS's work goes into it (every image in VK_IMAGE_LAYOUT_GENERAL)
    WsDlssImage color, depth, motion;  // at the render size: the scene (LDR), its depth (R32F, not inverted), the
                                       // motion vectors (RG16F, render pixels to where each was, with mv_scale)
    WsDlssImage output;              // at the output size, a storage image
    float jitter[2];                 // this frame's jitter, render pixels (x right, y down)
    float mv_scale[2];               // the motion vectors' scale to render pixels
    float dt_ms;
    int reset;                       // no usable history (a cut, the first frame)
    int quality;                     // WSDLSS_Q_*: the feature's mode (the render size is the caller's)
    int preset;                      // WSDLSS_PRESET_*: the model the feature is made with
} WsDlssEval;

typedef int (*PFN_wsdlss_abi)(void);
// The Vulkan instance / device extensions DLSS needs (NGX's own answer), as names into out[max]; the count, -1 if NGX
// couldn't say. The instance's before the instance is made; the device's before the device is.
typedef int (*PFN_wsdlss_instance_extensions)(const wchar_t *data_path, char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max);
typedef int (*PFN_wsdlss_device_extensions)(VkInstance instance, VkPhysicalDevice phys, const wchar_t *data_path,
                                            char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max);
// NGX on the device, and DLSS's availability on this GPU and driver. 0, or -1 with the reason in err.
typedef int (*PFN_wsdlss_init)(const WsDlssInit *in, char *err, int n);
// DLSS into e->cb: the feature made first when there is none (its creation recorded into e->cb too). 0, or -1 with the
// reason in err. A feature made for other sizes, mode or model is an error: the caller lets it go first
// (wsdlss_release_feature). Called on the thread that records e->cb, which may not be the one that called init.
typedef int (*PFN_wsdlss_evaluate)(const WsDlssEval *e, char *err, int n);
// The feature let go (device_idle first), so the next evaluate makes it again
typedef void (*PFN_wsdlss_release_feature)(void);
// The feature let go and NGX shut down (the device idle first: the caller's)
typedef void (*PFN_wsdlss_shutdown)(void);

#ifdef __cplusplus
}
#endif
