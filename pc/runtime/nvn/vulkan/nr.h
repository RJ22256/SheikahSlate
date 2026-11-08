// (WSword) Neural rendering, beta: OpenDLSS-NR through wsword_nr.dll (nr.c)
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vk.h"

#ifdef __cplusplus
extern "C" {
#endif

// (device.c) The device extensions the network needs - NULL / 0 when the setting is off or something it needs isn't
// there; then, the device made with them (and the features nr.c's comment lists) or not (what was missing)
const char *const *nr_device_extensions(uint32_t *count);
void nr_device_unsupported(const char *what);
void nr_device_made(VkInstance inst, VkPhysicalDevice phys, VkDevice dev, uint32_t qfam, uint32_t qindex);
// Ready, and the setting on
bool nr_on(void);
// (exec.c) The network on this frame, recorded into cb (on the recorder thread, in order): color - the frame
// (sampled), motion - rgba16f, current -> previous in uv units, z 1 where there is a history; out - rgba8 (its
// result, sRGB-encoded); all w x h, in VK_IMAGE_LAYOUT_GENERAL. Left: color and out in SHADER_READ_ONLY_OPTIMAL,
// motion in TRANSFER_SRC_OPTIMAL. reset: no history (a cut).
void nr_record(VkCommandBuffer cb, VkImage color, VkFormat cf, VkImage motion, VkImage out, VkFormat of, uint32_t w, uint32_t h,
               bool reset);
// Settings > Video: its state in words
void nr_status(char *out, size_t cap);

#ifdef __cplusplus
}
#endif
