// (WSword) wsword_nr.dll's interface (wsword_nr.cpp; runtime/nvn/vulkan/nr.c loads it): OpenDLSS-NR, neural rendering
// on WSword's frame (beta). C, for both sides.
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physical;
    VkDevice device;              // made with wsnr_requirements' extensions and the features nr.c lists
    uint32_t queue_family, queue_index;  // WSword's queue: the network's own setup work goes there too (synchronous)
    uint32_t width, height;       // the size to start at (wsnr_resize changes it)
    const char *model_dir;        // the player's model directory (manifest.json and its stage files)
    const char *kernel_dir;       // nr/kernels (the network's SPIR-V; its PTX in nr/ptx beside it)
    const char *shader_dir;       // nr/pass (the pass's preprocess and composite)
    void (*log)(const char *line);
} WsNrInit;

typedef struct {
    VkImage image;
    VkFormat format;
    VkImageLayout layout;
    uint32_t width, height;
} WsNrImage;

typedef struct {
    WsNrImage color;   // the frame, sampled (left in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    WsNrImage motion;  // rgba16f (left in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    WsNrImage out;     // rgba8 (left in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    int style;         // 0 the network's own, 1 natural, 2 cinematic
    float intensity;   // 0..1
    int reset;         // no history this frame (a cut)
} WsNrFrame;

typedef int (*PFN_wsnr_requirements)(const char *const **names, uint32_t *n);
typedef int (*PFN_wsnr_create)(const WsNrInit *in, char *err, int n);
typedef int (*PFN_wsnr_resize)(uint32_t w, uint32_t h, char *err, int n);
typedef int (*PFN_wsnr_record)(VkCommandBuffer cb, const WsNrFrame *f);
typedef void (*PFN_wsnr_timing)(float *network_ms, float *total_ms);
typedef void (*PFN_wsnr_destroy)(void);

#ifdef __cplusplus
}
#endif
