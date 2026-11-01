// Desktop test of Video > Mode (runtime/settings.c): Performance the defaults on the Galaxy Tab S10 Ultra, applied once
// over an older settings file (and written back with tab_preset), not again over the player's own changes after, and
// not on another device; Battery, Performance and Turbo each recognised, Custom once an option is changed; 40 fps kept
// in the settings file.
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
        fprintf(o, "fps %d res %d up %d out %d aa %d drs %d sync %d refresh %d mode %d",
                g_settings.fps_mode, g_settings.res_mode, g_settings.upscaler, g_settings.out_lines, g_settings.antialias,
                g_settings.drs_mode, g_settings.sync_lag, g_settings.display_refresh, settings_current_mode());
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
    expect(strstr(got, "fps 1 res 1 up 2 out -1 aa 0 drs 0 sync 1 refresh 0 mode 1") != NULL, "Performance is the tablet's default");

    run("tab-s10-ultra", "frame_rate = auto\nresolution = dynamic\nupscaler = sgsr1\nantialias = smaa\n", got, sizeof got);
    printf("  an older settings file on the tablet: %s\n", got);
    expect(strstr(got, "mode 1") != NULL, "Performance applied once over settings from before it");
    expect(ini_has("tab_preset = 9") && ini_has("upscaler = fsr1"), "and written back, marked");

    run("tab-s10-ultra", "tab_preset = 9\nupscaler = sgsr1\nantialias = smaa\n", got, sizeof got);
    printf("  the player's own changes after it: %s\n", got);
    expect(strstr(got, "up 3") != NULL && strstr(got, "aa 1") != NULL && strstr(got, "mode 3") != NULL,
           "not applied again over the player's changes (Custom)");

    run("phone", NULL, got, sizeof got);
    printf("  another device: %s\n", got);
    expect(strstr(got, "mode 3") != NULL, "not the default on another device");

    run("phone", "frame_rate = auto\n", got, sizeof got);
    expect(!ini_has("tab_preset"), "not applied on another device's settings file");

    // each mode applied (in a child, as the menu does after settings_load), recognised, and Custom once changed
    for (int m = MODE_BATTERY; m <= MODE_TURBO; m++) {
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
        if (m == MODE_TURBO) expect(ini_has("frame_rate = 40") && ini_has("display_refresh = max"), "Turbo: 40 fps on the highest refresh, saved");
        if (m == MODE_BATTERY) expect(ini_has("frame_rate = 30") && ini_has("display_refresh = 60"), "Battery: 30 fps at 60 Hz, saved");
    }

    run("tab-s10-ultra", "tab_preset = 9\nframe_rate = 40\ndisplay_refresh = max\nresolution = 1x\nupscaler = sgsr1\n", got, sizeof got);
    printf("  40 fps in the settings file: %s\n", got);
    expect(strstr(got, "fps 3") != NULL, "frame_rate = 40 read back as 40 fps");

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("modes: all checks passed\n");
    return 0;
}
