// (WSword) NVIDIA DLSS through wsword_dlss.dll (dlss.h, dlss_bridge.h)
#include "dlss.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../../paths.h"
#include "vk.h"

bool renderer_gpu_rtx(void);  // (device.c)
void vk_fg_wait_idle(void);   // (device.c: the presents, the recorder and the device idle)

#ifdef _WIN32
#include <windows.h>

static struct {
    bool tried, loaded, dev_ext, inited, failed;
    HMODULE h;
    PFN_wsdlss_instance_extensions inst_ext;
    PFN_wsdlss_device_extensions dev_exts;
    PFN_wsdlss_init init;
    PFN_wsdlss_evaluate evaluate;
    PFN_wsdlss_shutdown shutdown;
    int eval_fails;
    char err[400];
} d;

static const wchar_t *wide(const char *s, wchar_t *out, int n) {
    if (!MultiByteToWideChar(CP_UTF8, 0, s, -1, out, n)) out[0] = 0;
    return out;
}

static void load(void) {
    if (d.tried) return;
    d.tried = true;
    char p[1100];
    wchar_t w[1100];
    snprintf(p, sizeof p, "%s\\wsword_dlss.dll", rt_exe_dir());
    d.h = LoadLibraryW(wide(p, w, 1100));
    if (!d.h) { snprintf(d.err, sizeof d.err, "wsword_dlss.dll is missing next to WSword.exe"); return; }
    PFN_wsdlss_abi abi = (PFN_wsdlss_abi)(void *)GetProcAddress(d.h, "wsdlss_abi");
    d.inst_ext = (PFN_wsdlss_instance_extensions)(void *)GetProcAddress(d.h, "wsdlss_instance_extensions");
    d.dev_exts = (PFN_wsdlss_device_extensions)(void *)GetProcAddress(d.h, "wsdlss_device_extensions");
    d.init = (PFN_wsdlss_init)(void *)GetProcAddress(d.h, "wsdlss_init");
    d.evaluate = (PFN_wsdlss_evaluate)(void *)GetProcAddress(d.h, "wsdlss_evaluate");
    d.shutdown = (PFN_wsdlss_shutdown)(void *)GetProcAddress(d.h, "wsdlss_shutdown");
    if (!abi || abi() != WSDLSS_ABI || !d.inst_ext || !d.dev_exts || !d.init || !d.evaluate || !d.shutdown) {
        snprintf(d.err, sizeof d.err, "wsword_dlss.dll is from another WSword");
        FreeLibrary(d.h);
        d.h = NULL;
        return;
    }
    d.loaded = true;
    LOG("[dlss] wsword_dlss.dll loaded\n");
}

static const wchar_t *data_path(void) {
    static wchar_t w[1100];
    if (!w[0]) wide(rt_exe_dir(), w, 1100);
    return w;
}

// (SAVAGEWIND_NO_DLSS=1: the bridge left alone - no DLSS, nothing of NGX at startup)
bool dlss_bridge_loaded(void) {
    if (getenv("SAVAGEWIND_NO_DLSS")) return false;
    load();
    return d.loaded;
}

int dlss_instance_extensions(char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max) {
    if (!dlss_bridge_loaded()) return 0;
    int n = d.inst_ext(data_path(), out, max);
    if (n < 0) LOG("[dlss] NGX named no instance extensions\n");
    return n < 0 ? 0 : n;
}

int dlss_device_extensions(VkInstance instance, VkPhysicalDevice phys, char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max) {
    if (!dlss_bridge_loaded()) return 0;
    int n = d.dev_exts(instance, phys, data_path(), out, max);
    if (n < 0) LOG("[dlss] NGX named no device extensions\n");
    return n < 0 ? 0 : n;
}

void dlss_device_ready(bool extensions_on) { d.dev_ext = extensions_on; }

const char *dlss_dll_path(void) {
    static char p[1100];
    if (!p[0]) snprintf(p, sizeof p, "%s\\nvngx_dlss.dll", rt_exe_dir());
    return p;
}

bool dlss_dll_present(void) {
    char p[1100];
    struct stat st;
    snprintf(p, sizeof p, "%s\\dlss\\nvngx_dlss.dll", rt_exe_dir());
    return stat(dlss_dll_path(), &st) == 0 || stat(p, &st) == 0;
}

bool dlss_available(void) {
    return renderer_gpu_rtx() && dlss_bridge_loaded() && d.dev_ext && !d.failed && dlss_dll_present();
}

void dlss_status(char *out, size_t n) {
    if (!renderer_gpu_rtx()) snprintf(out, n, "DLSS needs an NVIDIA RTX graphics card.");
    else if (!dlss_bridge_loaded()) snprintf(out, n, "DLSS: %s.", d.err);
    else if (!d.dev_ext) snprintf(out, n, "DLSS: this driver lacks what DLSS needs - update the NVIDIA driver.");
    else if (d.failed) snprintf(out, n, "DLSS stopped: %s.", d.err);
    else if (!dlss_dll_present())
        snprintf(out, n, "DLSS: add your nvngx_dlss.dll - start screen > Settings > Files > NVIDIA DLSS, or put it next to "
                 "WSword.exe.");
    else snprintf(out, n, "DLSS is ready.%s", d.inited ? " Running." : "");
}

static void ngx_line(const char *line) { LOG("%s", line); }

bool dlss_dispatch(WsDlssEval *e) {
    if (!dlss_available()) return false;
    if (!d.inited) {
        static wchar_t paths[2][1100];
        char p[1100];
        snprintf(p, sizeof p, "%s\\dlss", rt_exe_dir());
        wide(p, paths[0], 1100);
        wide(rt_data_root(), paths[1], 1100);
        extern const char *rt_version(void);
        WsDlssInit in = {.instance = g_vk.inst, .phys = g_vk.phys, .dev = g_vk.dev, .gipa = vkGetInstanceProcAddr,
                         .gdpa = vkGetDeviceProcAddr, .data_path = data_path(), .dll_paths = {paths[0], paths[1]},
                         .engine_version = rt_version(), .log = ngx_line, .device_idle = vk_fg_wait_idle};
        if (d.init(&in, d.err, sizeof d.err)) {
            d.failed = true;
            LOG("[dlss] not started: %s\n", d.err);
            return false;
        }
        d.inited = true;
        LOG("[dlss] started on %s\n", g_vk.props.deviceName);
    }
    char err[400];
    if (d.evaluate(e, err, sizeof err)) {
        if (d.eval_fails++ < 8) LOG("[dlss] %s (%ux%u -> %ux%u)\n", err, e->color.width, e->color.height, e->output.width,
                                    e->output.height);
        if (d.eval_fails >= 3) {  // (a failure that stays: DLSS off for this run, its reason on the Video page)
            snprintf(d.err, sizeof d.err, "%s", err);
            d.failed = true;
        }
        return false;
    }
    d.eval_fails = 0;
    return true;
}

void dlss_shutdown(void) {
    if (d.inited) d.shutdown();
    d.inited = false;
}

#else  // (no DLSS: Linux, macOS)
bool dlss_bridge_loaded(void) { return false; }
int dlss_instance_extensions(char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max) { (void)out, (void)max; return 0; }
int dlss_device_extensions(VkInstance i, VkPhysicalDevice p, char (*out)[VK_MAX_EXTENSION_NAME_SIZE], int max) {
    (void)i, (void)p, (void)out, (void)max;
    return 0;
}
void dlss_device_ready(bool on) { (void)on; }
bool dlss_available(void) { return false; }
const char *dlss_dll_path(void) { return ""; }
bool dlss_dll_present(void) { return false; }
void dlss_status(char *out, size_t n) { snprintf(out, n, "DLSS is for Windows and NVIDIA RTX graphics cards."); }
bool dlss_dispatch(WsDlssEval *e) { (void)e; return false; }
void dlss_shutdown(void) {}
#endif
