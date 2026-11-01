// Frame generation with the player's own Lossless.dll (WSword): the C side of third_party/lsfg-vk, the last GPL-3.0
// version of lsfg-vk's backend, run on the renderer's own device and queue. The game's finished image is copied into
// one of two source images, the copy's submission signals the timeline semaphore, the backend's passes write the
// frames between the last two real ones, and each of those presents waits for its own value (device.c fg_present).
// Render thread only. No Lossless.dll ships with WSword: the player picks theirs (Settings > Lossless Scaling).
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FG_MAX_GENERATED 3  // (4x)

typedef struct {
    VkInstance instance;
    VkPhysicalDevice phys;
    VkDevice dev;
    uint32_t qfam;
    VkQueue queue;
    PFN_vkGetInstanceProcAddr gipa;
    bool fp16;               // shaderFloat16 is enabled on the device
    const char *cache_path;  // the backend's pipeline cache (NULL: none)
} FgDevice;

typedef struct {
    VkImage source[2];                      // real frame n goes into source[n % 2] (GENERAL layout throughout)
    VkImage dest[FG_MAX_GENERATED];         // generated frame i
    VkImageView dest_view[FG_MAX_GENERATED];
    int ndest;
    VkSemaphore sync;                       // timeline
    VkFormat format;
    uint32_t width, height;
} FgImages;

// Lossless.dll checked without a device: 0 when it holds the frame generation shaders (*fp16: the FP16 set too),
// else -1 with the reason in err.
int fg_check_dll(const char *path, bool *fp16, char *err, size_t n);
// The backend on the renderer's device, its shaders from the DLL (FP16 ones when allowed and the device has them,
// else full precision). 0, or -1 with the reason in err.
int fg_init(const FgDevice *d, const char *dll, bool allow_fp16, char *err, size_t n);
bool fg_ready(void);
bool fg_low_precision(void);
// A context for w x h frames, `generated` frames between two real ones; the open one is closed first. 0, or -1.
int fg_open(int generated, uint32_t w, uint32_t h, float flow_scale, bool perf, FgImages *out, char *err, size_t n);
bool fg_is_open(void);
void fg_close(void);  // (waits for the device to be idle)
// The value to signal once the next real frame is in its source image, and that frame's index (0: the first, with
// nothing before it to generate from).
uint64_t fg_next_signal(void);
uint64_t fg_frame_index(void);
// The passes for the frame just signalled: generated frame i is ready at fg_next_signal() (read before) + 1 + i.
// 0, or -1 with the reason in err.
int fg_schedule(char *err, size_t n);
// Some of the context's frames: passes which[0..count) (ascending), the k-th ready at fg_next_signal() (read before)
// + 1 + k. A context for 4x makes 2x's frame with {1}; count 0: only the shared pass (the next frame's history).
int fg_schedule_some(const int *which, int count, char *err, size_t n);
void fg_shutdown(void);

#ifdef __cplusplus
}
#endif
