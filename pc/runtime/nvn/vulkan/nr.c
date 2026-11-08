// (WSword) Neural rendering, beta: OpenDLSS-NR (https://github.com/aloshdenny/open-dlss, MIT) through wsword_nr.dll
// (tools/nr_bridge). Settings > Video > Neural rendering. The network re-renders each frame the game composited,
// before the HUD (exec.c nr_apply): detail generated from noise, tone and structure adjusted; the frame's size in and
// out (not an upscaler, no frames added). It runs on WSword's device - its features made with it (device.c, when
// the setting is on and everything is there) - and is recorded into WSword's command buffer.
//
// What it needs, all next to WSword.exe: wsword_nr.dll and nr\ (its kernels and passes; both in the release), and
// the player's model directory, nr-model\ (manifest.json and its stage files; SAVAGEWIND_NR_MODEL=<dir> another).
// None ships with WSword, and WSword doesn't say where to get one. Windows, NVIDIA RTX 40 (Ada) or newer: FP8
// cooperative matrices (VK_KHR_cooperative_matrix, VK_NV_cooperative_matrix2, VK_EXT_shader_float8) and
// VK_NV_cuda_kernel_launch.
#include "nr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL3/SDL_timer.h>

#include "../../paths.h"
#include "../../settings.h"
#include "vk.h"

#ifdef _WIN32
#include <windows.h>

#include "record.h"
#include "../../../tools/nr_bridge/wsword_nr.h"

static struct {
    bool tried, loaded, off_at_start, device_ok, ready;
    HMODULE h;
    PFN_wsnr_requirements req;
    PFN_wsnr_create create;
    PFN_wsnr_resize resize;
    PFN_wsnr_record record;
    PFN_wsnr_timing timing;
    PFN_wsnr_destroy destroy;
    char why[400];
    char model[1100], kernels[1100], pass[1100];
    uint32_t w, h_;  // the size it runs at (the recorder thread's)
    _Atomic int failed;
} n;

static bool exists(const char *p) {
    struct stat st;
    return stat(p, &st) == 0;
}

static void nr_log(const char *line) { LOG("%s", line); }

static void load(void) {
    if (n.tried) return;
    n.tried = true;
    if (!g_settings.neural) { n.off_at_start = true; return; }
    const char *dir = rt_exe_dir();
    char p[1200];
    snprintf(p, sizeof p, "%s\\wsword_nr.dll", dir);
    if (!exists(p)) { snprintf(n.why, sizeof n.why, "wsword_nr.dll is missing next to WSword.exe."); return; }
    snprintf(n.kernels, sizeof n.kernels, "%s\\nr\\kernels", dir);
    snprintf(n.pass, sizeof n.pass, "%s\\nr\\pass", dir);
    snprintf(p, sizeof p, "%s\\nr_composite.comp.spv", n.pass);
    if (!exists(p)) { snprintf(n.why, sizeof n.why, "The nr folder next to WSword.exe is missing or incomplete."); return; }
    const char *m = getenv("SAVAGEWIND_NR_MODEL");
    if (m && *m) snprintf(n.model, sizeof n.model, "%s", m);
    else snprintf(n.model, sizeof n.model, "%s\\nr-model", dir);
    snprintf(p, sizeof p, "%s\\manifest.json", n.model);
    if (!exists(p)) {
        snprintf(n.why, sizeof n.why, "No model: put your OpenDLSS-NR model folder (with its manifest.json) next to "
                 "WSword.exe as nr-model.");
        return;
    }
    wchar_t w[1200];
    snprintf(p, sizeof p, "%s\\wsword_nr.dll", dir);
    if (!MultiByteToWideChar(CP_UTF8, 0, p, -1, w, 1200)) w[0] = 0;
    n.h = LoadLibraryW(w);
    if (!n.h) { snprintf(n.why, sizeof n.why, "wsword_nr.dll didn't load (error %lu).", GetLastError()); return; }
    n.req = (PFN_wsnr_requirements)(void *)GetProcAddress(n.h, "wsnr_requirements");
    n.create = (PFN_wsnr_create)(void *)GetProcAddress(n.h, "wsnr_create");
    n.resize = (PFN_wsnr_resize)(void *)GetProcAddress(n.h, "wsnr_resize");
    n.record = (PFN_wsnr_record)(void *)GetProcAddress(n.h, "wsnr_record");
    n.timing = (PFN_wsnr_timing)(void *)GetProcAddress(n.h, "wsnr_timing");
    n.destroy = (PFN_wsnr_destroy)(void *)GetProcAddress(n.h, "wsnr_destroy");
    if (!n.req || !n.create || !n.resize || !n.record || !n.timing || !n.destroy) {
        snprintf(n.why, sizeof n.why, "wsword_nr.dll is from another WSword.");
        FreeLibrary(n.h), n.h = NULL;
        return;
    }
    n.loaded = true;
    LOG("[nr] wsword_nr.dll loaded; model %s\n", n.model);
}

const char *const *nr_device_extensions(uint32_t *count) {
    *count = 0;
    load();
    if (!n.loaded) return NULL;
    const char *const *names = NULL;
    if (!n.req(&names, count)) *count = 0;
    return names;
}

void nr_device_unsupported(const char *what) {
    snprintf(n.why, sizeof n.why, "This GPU or driver can't run it (no %s): NVIDIA RTX 40 or newer, a recent driver.", what);
    LOG("[nr] not available: no %s\n", what);
}

void nr_device_made(VkInstance inst, VkPhysicalDevice phys, VkDevice dev, uint32_t qfam, uint32_t qindex) {
    if (!n.loaded) return;
    n.device_ok = true;
    WsNrInit in = {.instance = inst, .physical = phys, .device = dev, .queue_family = qfam, .queue_index = qindex,
                   .width = 256, .height = 144, .model_dir = n.model, .kernel_dir = n.kernels, .shader_dir = n.pass,
                   .log = nr_log};
    uint64_t t0 = SDL_GetTicksNS();
    char err[400] = "";
    n.ready = n.create(&in, err, sizeof err) != 0;
    n.w = in.width, n.h_ = in.height;
    if (n.ready) LOG("[nr] OpenDLSS-NR started (%.0f ms)\n", (SDL_GetTicksNS() - t0) / 1e6);
    else snprintf(n.why, sizeof n.why, "%s", err), LOG("[nr] %s\n", err);
}

bool nr_on(void) { return n.ready && !n.failed && g_settings.neural; }

typedef struct {
    VkCommandBuffer cb;
    WsNrFrame f;
} RunArgs;

// (the recorder thread) the size first (the device idle), then the network into cb
static void run(const void *p) {
    const RunArgs *a = p;
    if (n.failed) return;
    if (a->f.color.width != n.w || a->f.color.height != n.h_) {
        char err[400] = "";
        vkQueueWaitIdle(g_vk.queue);  // (the recorder thread's queue; not the device: Lossless's present thread has the other)
        if (!n.resize(a->f.color.width, a->f.color.height, err, sizeof err)) {
            snprintf(n.why, sizeof n.why, "%s", err);
            LOG("[nr] %s: off\n", err);
            n.failed = 1;
            return;
        }
        n.w = a->f.color.width, n.h_ = a->f.color.height;
    }
    if (!n.record(a->cb, &a->f)) {
        static int k;
        if (k++ < 5) LOG("[nr] this frame's network wasn't recorded\n");
    }
}

void nr_record(VkCommandBuffer cb, VkImage color, VkFormat cf, VkImage motion, VkImage out, VkFormat of, uint32_t w, uint32_t h,
               bool reset) {
    RunArgs a = {.cb = cb};
    a.f.color = (WsNrImage){color, cf, VK_IMAGE_LAYOUT_GENERAL, w, h};
    a.f.motion = (WsNrImage){motion, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_GENERAL, w, h};
    a.f.out = (WsNrImage){out, of, VK_IMAGE_LAYOUT_GENERAL, w, h};
    a.f.style = g_settings.neural == 2 ? 1 : g_settings.neural == 3 ? 2 : 0;
    int s = g_settings.neural_strength ? g_settings.neural_strength : 100;
    a.f.intensity = s / 100.0f;
    a.f.reset = reset;
    rec_call(run, &a, sizeof a);
}

void nr_status(char *out, size_t cap) {
    load();
    if (!g_settings.neural) { snprintf(out, cap, "Off."); return; }
    if (nr_on()) {
        float net = 0, all = 0;
        n.timing(&net, &all);
        snprintf(out, cap, "On, at %ux%u: %.1f ms of GPU time a frame (the network %.1f ms).", n.w, n.h_, all, net);
        return;
    }
    if (n.off_at_start || (n.loaded && !n.device_ok && !n.why[0])) { snprintf(out, cap, "Restart WSword to start it."); return; }
    snprintf(out, cap, "%s", n.why[0] ? n.why : "Not started.");
}

#else  // (Windows only)
const char *const *nr_device_extensions(uint32_t *count) { *count = 0; return NULL; }
void nr_device_unsupported(const char *what) { (void)what; }
void nr_device_made(VkInstance inst, VkPhysicalDevice phys, VkDevice dev, uint32_t qfam, uint32_t qindex) {
    (void)inst, (void)phys, (void)dev, (void)qfam, (void)qindex;
}
bool nr_on(void) { return false; }
void nr_record(VkCommandBuffer cb, VkImage color, VkFormat cf, VkImage motion, VkImage out, VkFormat of, uint32_t w, uint32_t h,
               bool reset) {
    (void)cb, (void)color, (void)cf, (void)motion, (void)out, (void)of, (void)w, (void)h, (void)reset;
}
void nr_status(char *out, size_t cap) { snprintf(out, cap, "Windows only."); }
#endif
