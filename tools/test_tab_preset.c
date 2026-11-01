// Desktop test of the Galaxy Tab S10 Ultra graphics preset (runtime/settings.c): the defaults on the tablet, applied
// once over an older settings file (and written back with tab_preset), not again over the player's own changes after,
// and not on another device.
//   clang -D__ANDROID__ -Iruntime -I<SDL3 include> -I<stub dir with sys/system_properties.h> -o build/test_tab_preset \
//         tools/test_tab_preset.c runtime/settings.c && build/test_tab_preset
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
    FILE *f = fopen("preset.out", "w");
    fclose(f);
    if (ini) { f = fopen("sheikahslate.ini", "w"); fputs(ini, f); fclose(f); }
    else unlink("sheikahslate.ini");
    fflush(stdout);
    pid_t p = fork();
    if (!p) {
        if (profile) setenv("LARECOMP_DEVICE_PROFILE", profile, 1);
        else unsetenv("LARECOMP_DEVICE_PROFILE");
        settings_load();
        FILE *o = fopen("preset.out", "w");
        fprintf(o, "fps %d res %d up %d out %d aa %d drs %d sync %d refresh %d preset %d",
                g_settings.fps_mode, g_settings.res_mode, g_settings.upscaler, g_settings.out_lines, g_settings.antialias,
                g_settings.drs_mode, g_settings.sync_lag, g_settings.display_refresh, settings_tab_preset_on());
        fclose(o);
        _exit(0);
    }
    int st;
    waitpid(p, &st, 0);
    f = fopen("preset.out", "r");
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
    char dir[] = "/tmp/tabpresetXXXXXX";
    if (!mkdtemp(dir) || chdir(dir)) return 2;
    char got[512];

    run("tab-s10-ultra", NULL, got, sizeof got);
    printf("  no settings file on the tablet: %s\n", got);
    expect(strstr(got, "fps 1 res 1 up 2 out -1 aa 0 drs 0 sync 1 refresh 0 preset 1") != NULL, "the preset is the tablet's default");

    run("tab-s10-ultra", "frame_rate = auto\nresolution = dynamic\nupscaler = sgsr1\nantialias = smaa\n", got, sizeof got);
    printf("  an older settings file on the tablet: %s\n", got);
    expect(strstr(got, "preset 1") != NULL, "applied once over settings from before it");
    expect(ini_has("tab_preset = 9") && ini_has("upscaler = fsr1"), "and written back, marked");

    run("tab-s10-ultra", "tab_preset = 9\nupscaler = sgsr1\nantialias = smaa\n", got, sizeof got);
    printf("  the player's own changes after it: %s\n", got);
    expect(strstr(got, "up 3") != NULL && strstr(got, "aa 1") != NULL && strstr(got, "preset 0") != NULL,
           "not applied again over the player's changes");

    run("phone", NULL, got, sizeof got);
    printf("  another device: %s\n", got);
    expect(strstr(got, "preset 0") != NULL, "not the default on another device");

    run("phone", "frame_rate = auto\n", got, sizeof got);
    expect(!ini_has("tab_preset"), "not applied on another device's settings file");

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("tab preset: all checks passed\n");
    return 0;
}
