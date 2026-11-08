// (WSword) wsword_nr.dll: OpenDLSS-NR (https://github.com/aloshdenny/open-dlss, MIT) on WSword's frame - Settings >
// Video > Neural rendering (beta). The network (its src/: the model, the kernels, the graph) and the demo's pass
// (demo/nr_pass.cpp, patched by patch_open_dlss.py: WSword's motion, no second tone mapping) run on WSword's own
// device and queue (vk::Context adopts them), recorded into WSword's command buffer on its recorder thread
// (runtime/nvn/vulkan/nr.c). The player supplies the model directory; none ships with WSword.
// Built with MSVC in CI (.github/workflows/build-windows.yml) against the open-dlss checkout, Vulkan-Headers and volk.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

#include "nr_pass.h"
#include "wsword_nr.h"

#define EXPORT extern "C" __declspec(dllexport)

namespace {
struct {
    void (*log)(const char *line) = nullptr;
    std::unique_ptr<NrPass> pass;
    uint32_t w = 0, h = 0;
    bool reset = true;
} s;

void say(char *err, int n, const char *what, const char *detail) {
    if (err && n > 0) _snprintf_s(err, (size_t)n, _TRUNCATE, "%s%s%s", what, detail ? ": " : "", detail ? detail : "");
}

void nrlog(const char *fmt, ...) {
    if (!s.log) return;
    char line[1024];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf_s(line, sizeof line, _TRUNCATE, fmt, ap);
    va_end(ap);
    s.log(line);
}
}  // namespace

// The device extensions and features the network needs (vk::DeviceRequirements), for WSword's vkCreateDevice: the
// extension names (n of them) and, chained from *chain, its feature structures (its pNext chain without
// VkPhysicalDeviceFeatures2 and the Vulkan 1.1/1.2/1.3 ones - nr.c merges those into WSword's own)
EXPORT int wsnr_requirements(const char *const **names, uint32_t *n) {
    static vk::DeviceRequirements req;
    *names = req.extensions.data();
    *n = (uint32_t)req.extensions.size();
    return 1;
}

EXPORT int wsnr_create(const WsNrInit *in, char *err, int n) {
    s.log = in->log;
    try {
        GpuDevice d;
        d.instance = in->instance, d.physicalDevice = in->physical, d.device = in->device;
        d.queueFamily = in->queue_family, d.rendererQueueIndex = in->queue_index, d.nrQueueIndex = in->queue_index;
        s.pass = std::make_unique<NrPass>(d, in->width, in->height, in->model_dir, in->kernel_dir, in->shader_dir);
        s.w = in->width, s.h = in->height, s.reset = true;
        nrlog("[nr] ready on %s at %ux%u\n", s.pass->deviceName().c_str(), s.w, s.h);
        return 1;
    } catch (const std::exception &e) {
        s.pass.reset();
        say(err, n, "OpenDLSS-NR didn't start", e.what());
        return 0;
    }
}

// The size the network runs at (the device idle: rebuilds the graph, its images and command buffers, ~50 ms)
EXPORT int wsnr_resize(uint32_t w, uint32_t h, char *err, int n) {
    if (!s.pass) return 0;
    try {
        s.pass->resize(w, h);
        s.w = w, s.h = h, s.reset = true;
        nrlog("[nr] %ux%u\n", w, h);
        return 1;
    } catch (const std::exception &e) {
        say(err, n, "OpenDLSS-NR: resize", e.what());
        return 0;
    }
}

// This frame's network, recorded into cb (outside any render pass): color - the frame (sampled), motion - rgba16f,
// the network's size, current -> previous in uv units, z 1 where the previous frame shows that point; out - rgba8,
// the network's size, its result (sRGB-encoded). Each is given in its layout; they are left as WsNrFrame says.
EXPORT int wsnr_record(VkCommandBuffer cb, const WsNrFrame *f) {
    if (!s.pass || f->color.width != s.w || f->color.height != s.h) return 0;
    try {
        if (s.pass->chainTimedOut()) {
            nrlog("[nr] a chained wait timed out: that frame was wrong; rebuilt with barriers\n");
            s.pass->fallBackToBarriers();
            s.reset = true;
        }
        if (f->reset || s.reset) s.pass->resetHistory();
        s.reset = false;
        NrControls c;
        c.enabled = true;
        c.style = f->style;
        c.intensity = f->intensity;
        c.temporal = true;
        c.paperWhite = 1.0f;  // (BOTW's image: display-referred, 1 is white)
        if (f->style == 1) c.styleExposure = -0.1f, c.styleContrast = -0.25f, c.styleSaturation = -0.1f;
        if (f->style == 2) c.styleSaturation = -0.15f;
        static const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        NrPass::Frame fr = s.pass->beginFrame(c, identity);
        GpuImage color{(uint64_t)f->color.image, (uint32_t)f->color.format, (uint32_t)f->color.layout, f->color.width, f->color.height};
        GpuImage motion{(uint64_t)f->motion.image, (uint32_t)f->motion.format, (uint32_t)f->motion.layout, f->motion.width, f->motion.height};
        GpuImage out{(uint64_t)f->out.image, (uint32_t)f->out.format, (uint32_t)f->out.layout, f->out.width, f->out.height};
        s.pass->recordStamp(cb, fr, NrPass::kFrameStart);
        s.pass->record(cb, fr, color, motion, out);
        s.pass->recordStamp(cb, fr, NrPass::kPresentEnd);
        s.pass->endFrame();
        return 1;
    } catch (const std::exception &e) {
        nrlog("[nr] %s\n", e.what());
        return 0;
    }
}

// The network's GPU time (ms) of a recent frame, and the whole pass's
EXPORT void wsnr_timing(float *network_ms, float *total_ms) {
    if (!s.pass) { *network_ms = *total_ms = 0; return; }
    const NrTimings &t = s.pass->timings();
    *network_ms = (float)t.networkMs;
    *total_ms = (float)t.frameMs;
}

EXPORT void wsnr_destroy(void) { s.pass.reset(); }
