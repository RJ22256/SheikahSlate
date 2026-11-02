// Desktop test of Video > Mode (runtime/settings.c): Performance the defaults on the Galaxy Tab S10 Ultra, applied once
// over an older settings file - 0.2.8-slate.17's too (and written back with tab_preset), not again over the player's own
// changes after, and not on another device; Performance (60 fps) and Quality (30 fps, the game's native resolution on the
// screen's) each recognised, Custom once an option is changed; with frame generation Performance's frame rate is the
// screen's 120 Hz over the multiplier and Quality's image 1080p; 40 fps kept in the settings file.
//   clang -D__ANDROID__ -Iruntime -I<SDL3 include> -I<stub dir with sys/system_properties.h> -o build/test_modes \
//         tools/test_modes.c runtime/settings.c && build/test_modes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <SDL3/SDL.h>

#include "input_bind.h"
#include "settings.h"

// (the rest of the runtime settings.c links against)
int g_game_version = 182;
bool bind_settings_load(const char *key, const char *val) { (void)key, (void)val; return false; }
void bind_settings_save(FILE *f) { (void)f; }
SDL_Locale **SDL_GetPreferredLocales(int *count) { if (count) *count = 0; return NULL; }
void SDL_free(void *p) { (void)p; }
int __system_property_get(const char *name, char *value) { (void)name; value[0] = 0; return 0; }

static int failures;
static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    failures += !ok;
}

static int run(const char *profile, const char *ini, char *after, size_t n) {
    // each case in a child: settings_load runs once a process
    FILE *f = fopen("modes.out", "w");
    fclose(f);
    if (ini) { f = fopen("sheikahslate.ini", "w"); fputs(ini, f); fclose(f); }
    else unlink("sheikahslate.ini");
    fflush(stdout);
    pid_t p = fork();
    if (!p) {
        if (profile) setenv("LARECOMP_DEVICE_PROFILE", profile, 1);
        else unsetenv("LARECOMP_DEVICE_PROFILE");
        settings_load();
        FILE *o = fopen("modes.out", "w");
        fprintf(o, "fps %d res %d up %d out %d aa %d drs %d sync %d refresh %d mode %d fgperf %d",
                g_settings.fps_mode, g_settings.res_mode, g_settings.upscaler, g_settings.out_lines, g_settings.antialias,
                g_settings.drs_mode, g_settings.sync_lag, g_settings.display_refresh, settings_current_mode(), g_settings.fg_perf);
        fclose(o);
        _exit(0);
    }
    int st;
    waitpid(p, &st, 0);
    f = fopen("modes.out", "r");
    size_t r = fread(after, 1, n - 1, f);
    after[r] = 0;
    fclose(f);
    return 0;
}

static int ini_has(const char *what) {
    FILE *f = fopen("sheikahslate.ini", "r");
    if (!f) return 0;
    char buf[8192];
    size_t r = fread(buf, 1, sizeof buf - 1, f);
    buf[r] = 0;
    fclose(f);
    return strstr(buf, what) != NULL;
}

int main(void) {
    char dir[] = "/tmp/modesXXXXXX";
    if (!mkdtemp(dir) || chdir(dir)) return 2;
    char got[512];

    run("tab-s10-ultra", NULL, got, sizeof got);
    printf("  no settings file on the tablet: %s\n", got);
    expect(strstr(got, "fps 2 res 1 up 3 out -1 aa 0 drs 0 sync 1 refresh 0 mode 0") != NULL,
           "Performance (60 fps, GSR 1, 1080p) is the tablet's default");
    expect(strstr(got, "fgperf 1") != NULL, "frame generation's performance mode on by default on the tablet");

    run("tab-s10-ultra", "frame_rate = auto\nresolution = dynamic\nupscaler = fsr1\nantialias = smaa\n", got, sizeof got);
    printf("  an older settings file on the tablet: %s\n", got);
    expect(strstr(got, "mode 0") != NULL, "Performance applied once over settings from before it");
    expect(ini_has("tab_preset = 10") && ini_has("upscaler = sgsr1") && ini_has("frame_rate = 60"), "and written back, marked");

    run("tab-s10-ultra", "tab_preset = 9\nframe_rate = 40\nupscaler = sgsr1\n", got, sizeof got);
    printf("  0.2.8-slate.17's settings (Turbo) on the tablet: %s\n", got);
    expect(strstr(got, "fps 2") != NULL && strstr(got, "mode 0") != NULL && ini_has("tab_preset = 10"),
           "slate.17's settings: Performance applied once (the four modes are two now)");

    run("tab-s10-ultra", "tab_preset = 10\nupscaler = fsr1\nantialias = smaa\n", got, sizeof got);
    printf("  the player's own changes after it: %s\n", got);
    expect(strstr(got, "up 2") != NULL && strstr(got, "aa 1") != NULL && strstr(got, "mode 2") != NULL,
           "not applied again over the player's changes (Custom)");

    run("phone", NULL, got, sizeof got);
    printf("  another device: %s\n", got);
    expect(strstr(got, "mode 2") != NULL, "not the default on another device");

    run("phone", "frame_rate = auto\n", got, sizeof got);
    expect(!ini_has("tab_preset"), "not applied on another device's settings file");

    // each mode applied (in a child, as the menu does after settings_load), recognised, and Custom once changed
    for (int m = 0; m < MODE_COUNT; m++) {
        unlink("sheikahslate.ini");
        fflush(stdout);
        pid_t p = fork();
        if (!p) {
            setenv("LARECOMP_DEVICE_PROFILE", "tab-s10-ultra", 1);
            settings_load();
            settings_apply_mode(m);
            int ok = settings_current_mode() == m;
            settings_save();
            g_settings.upscaler = g_settings.upscaler == UPSCALE_FSR1 ? UPSCALE_SGSR1 : UPSCALE_FSR1;
            ok = ok && settings_current_mode() == MODE_CUSTOM;
            _exit(ok ? 0 : 1);
        }
        int st;
        waitpid(p, &st, 0);
        char what[96];
        snprintf(what, sizeof what, "mode %d: applied, recognised, Custom once an option changes", m);
        expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, what);
        if (m == MODE_PERFORMANCE)
            expect(ini_has("frame_rate = 60") && ini_has("display_refresh = max") && ini_has("upscaler = sgsr1") &&
                   ini_has("output = auto") && ini_has("game_resolution = steady"),
                   "Performance: 60 fps on the highest refresh, GSR 1, 1080p, the resolution steadied - saved");
        if (m == MODE_QUALITY)
            expect(ini_has("frame_rate = 30") && ini_has("display_refresh = max") && ini_has("upscaler = fsr1") &&
                   ini_has("output = screen") && ini_has("game_resolution = sharp") && ini_has("resolution = 1x"),
                   "Quality: 30 fps on the highest refresh, the game's 900p held, FSR 1 onto the screen's own - saved");
    }

    // frame generation and the modes: Performance at 120 Hz over the multiplier, Quality's image 1080p while it's on
    {
        struct { int mode; const char *fg; int fps, out; } c[] = {
            {MODE_PERFORMANCE, "off", 2, OUT_AUTO}, {MODE_PERFORMANCE, "2x", 2, OUT_AUTO}, {MODE_PERFORMANCE, "3x", 3, OUT_AUTO},
            {MODE_PERFORMANCE, "4x", 1, OUT_AUTO}, {MODE_QUALITY, "off", 1, 0}, {MODE_QUALITY, "2x", 1, OUT_AUTO},
            {MODE_QUALITY, "4x", 1, OUT_AUTO}};
        for (unsigned i = 0; i < sizeof c / sizeof c[0]; i++) {
            FILE *f = fopen("sheikahslate.ini", "w");
            fprintf(f, "tab_preset = 10\nframe_generation = %s\n", c[i].fg);
            fclose(f);
            pid_t p = fork();
            if (!p) {
                setenv("LARECOMP_DEVICE_PROFILE", "tab-s10-ultra", 1);
                settings_load();
                settings_apply_mode(c[i].mode);
                bool ok = g_settings.fps_mode == c[i].fps && g_settings.out_lines == c[i].out && settings_current_mode() == c[i].mode;
                // (the menu's Frame generation: the mode applied again after it changes - the same mode, its options moved)
                g_settings.framegen = g_settings.framegen == 2 ? 0 : 2;
                settings_apply_mode(c[i].mode);
                int fps3x = c[i].mode == MODE_QUALITY ? 1 : 3, fps_off = c[i].mode == MODE_QUALITY ? 1 : 2;
                ok = ok && settings_current_mode() == c[i].mode && g_settings.fps_mode == (g_settings.framegen ? fps3x : fps_off);
                _exit(ok ? 0 : 1);
            }
            int st;
            waitpid(p, &st, 0);
            char what[128];
            snprintf(what, sizeof what, "%s with frame generation %s: %s fps, image %s", c[i].mode ? "Quality" : "Performance",
                     c[i].fg, c[i].fps == 1 ? "30" : c[i].fps == 3 ? "40" : "60", c[i].out ? "1080p" : "the screen's");
            expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, what);
        }
    }

    run("tab-s10-ultra", "tab_preset = 10\nframe_rate = 40\ndisplay_refresh = max\nresolution = 1x\nupscaler = sgsr1\n", got, sizeof got);
    printf("  40 fps in the settings file: %s\n", got);
    expect(strstr(got, "fps 3") != NULL, "frame_rate = 40 read back as 40 fps");

    // (WSword) frame generation: read back, kept by the modes (a mode isn't about it), clamped, saved
    for (int m = 0; m < MODE_COUNT; m++) {
        FILE *f = fopen("sheikahslate.ini", "w");
        fputs("tab_preset = 10\nframe_generation = 3x\nfg_flow = 60\nfg_performance = 1\n", f);
        fclose(f);
        fflush(stdout);
        pid_t p = fork();
        if (!p) {
            setenv("LARECOMP_DEVICE_PROFILE", "tab-s10-ultra", 1);
            settings_load();
            int ok = g_settings.framegen == 2 && g_settings.fg_flow == 60 && g_settings.fg_perf;
            settings_apply_mode(m);
            ok = ok && g_settings.framegen == 2 && g_settings.fg_flow == 60 && g_settings.fg_perf && settings_current_mode() == m;
            settings_save();
            _exit(ok ? 0 : 1);
        }
        int st;
        waitpid(p, &st, 0);
        char what[96];
        snprintf(what, sizeof what, "frame generation 3x read back and kept by mode %d", m);
        expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, what);
    }
    expect(ini_has("frame_generation = 3x") && ini_has("fg_flow = 60") && ini_has("fg_performance = 1"),
           "frame generation saved");
    {   // (slate.15's key: its default 75 read as Automatic, another value kept)
        int want[2] = {0, 100};
        const char *ini[2] = {"tab_preset = 9\nfg_flow_scale = 75\n", "tab_preset = 9\nfg_flow_scale = 100\n"};
        for (int k = 0; k < 2; k++) {
            FILE *f = fopen("sheikahslate.ini", "w");
            fputs(ini[k], f);
            fclose(f);
            pid_t p = fork();
            if (!p) {
                settings_load();
                settings_save();
                _exit(g_settings.fg_flow == want[k] ? 0 : 1);
            }
            int st;
            waitpid(p, &st, 0);
            expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, k ? "slate.15's flow scale 100 kept" : "slate.15's default flow scale 75: Automatic");
        }
        expect(ini_has("fg_flow = 100"), "and saved under the new key");
    }
    {
        FILE *f = fopen("sheikahslate.ini", "w");
        fputs("tab_preset = 9\nframe_generation = 9x\nfg_flow = 5\n", f);
        fclose(f);
        pid_t p = fork();
        if (!p) {
            settings_load();
            _exit(g_settings.framegen == 0 && g_settings.fg_flow == 25 ? 0 : 1);
        }
        int st;
        waitpid(p, &st, 0);
        expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, "an unknown frame generation is off, the flow scale clamped to 25%");
    }
    {
        unlink("sheikahslate.ini");
        pid_t p = fork();
        if (!p) {
            settings_load();
            _exit(g_settings.framegen == 0 && g_settings.fg_flow == 0 && !g_settings.fg_perf ? 0 : 1);
        }
        int st;
        waitpid(p, &st, 0);
        expect(WIFEXITED(st) && WEXITSTATUS(st) == 0, "frame generation off by default (flow scale Automatic)");
    }

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("modes: all checks passed\n");
    return 0;
}
