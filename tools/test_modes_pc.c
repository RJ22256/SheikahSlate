// Desktop test of the PC build's Video > Mode (runtime/settings.c, without __ANDROID__): the PC's defaults are
// Performance (Auto 30 / 60 at the window's resolution); each of the four recognised once applied; Custom once an option
// changes; Exclusive fullscreen's refresh rate (120 Hz among them) and the PC's own keys - mouse camera, fullscreen
// mode, app icon - kept in the settings file and read back; neither changes the mode.
//   clang -Iruntime -o build/test_modes_pc tools/test_modes_pc.c runtime/settings.c -lm && build/test_modes_pc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "input_bind.h"
#include "settings.h"

// (the rest of the runtime settings.c links against)
int g_game_version = 182;
bool bind_settings_load(const char *key, const char *val) { (void)key, (void)val; return false; }
void bind_settings_save(FILE *f) { (void)f; }

static int failures;
static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    failures += !ok;
}

static int ini_has(const char *what) {
    FILE *f = fopen("sheikahslate.ini", "r");
    if (!f) return 0;
    char buf[8192];
    size_t n = fread(buf, 1, sizeof buf - 1, f);
    buf[n] = 0;
    fclose(f);
    return strstr(buf, what) != NULL;
}

int main(void) {
    char dir[] = "/tmp/test_modes_pc_XXXXXX";
    if (!mkdtemp(dir) || chdir(dir) != 0) return 1;
    unlink("sheikahslate.ini");
    fflush(stdout);
    pid_t first = fork();  // (settings_load reads the file once a process: each part in a child of its own)
    if (first) {
        int st;
        waitpid(first, &st, 0);
        if (!WIFEXITED(st)) return 1;
        failures += WEXITSTATUS(st);
        goto read_back;
    }
    settings_load();  // (no file: the defaults)
    expect(settings_current_mode() == MODE_PERFORMANCE, "PC defaults are Performance");
    expect(g_settings.fps_mode == 0 && g_settings.res_mode == RES_MATCH && g_settings.upscaler == UPSCALE_FSR1,
           "Performance: Auto 30 / 60, the window's resolution, FSR 1");
    for (int m = MODE_BATTERY; m <= MODE_TURBO60; m++) {
        settings_apply_mode(m);
        char what[96];
        snprintf(what, sizeof what, "mode %d recognised once applied", m);
        expect(settings_current_mode() == m, what);
    }
    settings_apply_mode(MODE_TURBO);
    expect(g_settings.fps_mode == 3, "Turbo: 40 fps");
    settings_apply_mode(MODE_TURBO60);
    expect(g_settings.fps_mode == 2, "Turbo 60: 60 fps");
    settings_apply_mode(MODE_BATTERY);
    expect(g_settings.fps_mode == 1 && g_settings.res_mode == 1 && g_settings.upscaler == UPSCALE_SGSR1 && g_settings.drs_mode == 2,
           "Battery: 30 fps, 900p, SGSR, the lowest resolution held");
    settings_apply_mode(MODE_PERFORMANCE);
    g_settings.display_refresh = 3;  // (Exclusive fullscreen at 120 Hz)
    expect(settings_current_mode() == MODE_PERFORMANCE, "the refresh rate isn't part of a mode on the PC");
    g_settings.antialias = AA_SMAA;
    expect(settings_current_mode() == MODE_CUSTOM, "Custom once an option changes");
    g_settings.antialias = AA_OFF;
    g_settings.fullscreen = true, g_settings.fullscreen_exclusive = true;
    g_settings.mouse_sens = 1.5f, g_settings.mouse_invert_y = true, g_settings.app_icon = 3;
    g_settings.framegen = 1;
    settings_save();
    expect(ini_has("display_refresh = 120"), "120 Hz saved");
    expect(ini_has("fullscreen_exclusive = 1") && ini_has("mouse_camera = 1.50") && ini_has("mouse_invert_y = 1") &&
           ini_has("app_icon = 3"), "the PC's keys saved");
    fflush(stdout);
    _exit(failures);
read_back:;
    pid_t p = fork();
    if (!p) {
        memset(&g_settings, 0, sizeof g_settings);
        settings_load();
        int ok = g_settings.display_refresh == 3 && g_settings.fullscreen_exclusive && g_settings.mouse_invert_y &&
                 g_settings.mouse_sens > 1.49f && g_settings.mouse_sens < 1.51f && g_settings.app_icon == 3 &&
                 g_settings.framegen == 1 && g_settings.fullscreen;
        _exit(ok ? 0 : 1);
    }
    int st;
    waitpid(p, &st, 0);
    expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, "the PC's keys read back");
    unlink("sheikahslate.ini");
    if (chdir("/tmp") == 0) rmdir(dir);
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("PC modes: all checks passed\n");
    return 0;
}
