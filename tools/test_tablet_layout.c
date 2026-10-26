// Sheikah Slate's large-screen logic, on the desktop: the on-screen controls' unit and notch avoidance
// (runtime/ui/touch_units.h) and the output's shape on a 16:10 screen (runtime/nvn/output_scale.c).
//   cc -O1 -Iruntime -o build/test_tablet_layout tools/test_tablet_layout.c -lm && build/test_tablet_layout
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "settings.h"
#include "ui/touch_units.h"

// (output_scale.c's dependencies: the settings and the frame pacer)
Settings g_settings;
int framepace_target(void) { return 30; }
bool framepace_on(void) { return true; }
#include "../runtime/nvn/output_scale.c"

static int fails;
#define CHECK(cond, ...) do { bool ok_ = (cond); printf("%s ", ok_ ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); \
                              if (!ok_) fails++; } while (0)
static bool near(double a, double b, double eps) { return fabs(a - b) <= eps; }

int main(void) {
    // ---- the unit
    // Galaxy Tab S10 Ultra: 2960x1848 at 239 ppi - 0.95 mm a unit, not 1% of 196 mm
    float tab = touch_unit_px(1848, 239);
    CHECK(near(tab, 239 / 25.4 * 0.95, 0.01), "Tab S10 Ultra unit %.2f px (%.2f mm)", tab, tab / (239 / 25.4));
    CHECK(near(15 * tab / (239 / 25.4), 14.25, 0.05), "face button %.1f mm across", 15 * tab / (239 / 25.4));
    // the phone the layout was made on (POCO F7 Ultra, 1440 px tall, ~526 ppi): unchanged, 1% of the height
    CHECK(touch_unit_px(1440, 526) == 14.4f, "phone unit %.2f px: upstream's 1%% of the height", touch_unit_px(1440, 526));
    // an unknown or implausible density: upstream's unit
    CHECK(touch_unit_px(1848, 0) == 18.48f, "unknown density: %.2f px", touch_unit_px(1848, 0));
    CHECK(touch_unit_px(1848, 5000) == 18.48f, "implausible density: %.2f px", touch_unit_px(1848, 5000));
    // continuity: never larger than upstream's
    for (int h = 600; h <= 3000; h += 100)
        for (int dpi = 100; dpi <= 800; dpi += 50)
            if (touch_unit_px((float)h, (float)dpi) > h / 100.0f + 1e-4f) { CHECK(false, "unit above 1%% at %d px %d ppi", h, dpi); }

    // ---- the notch: a 200 x 70 px cutout at the top edge's middle of a 2960 x 1848 window
    int top[4] = {1380, 0, 1580, 70}, bottom[4] = {1380, 1778, 1580, 1848};
    float u = tab, e = 8 * u, m = 1.5f * u;
    float y = touch_avoid_cutouts(1480, 6 * u, e, m, 1848, top, 1);  // the menu button's default place
    CHECK(y - e >= 70 + m - 0.01f, "menu button moved below the notch: y %.1f (top edge %.1f)", y, y - e);
    y = touch_avoid_cutouts(1480 - 9 * u, 1848 - 7 * u, e, m, 1848, bottom, 1);  // Minus, tablet turned around
    CHECK(y + e <= 1778 - m + 0.01f, "Minus moved above the notch at the bottom: y %.1f (bottom edge %.1f)", y, y + e);
    y = touch_avoid_cutouts(4 * u + 7.5f * u, 10 * u, e, m, 1848, top, 1);  // ZL, top left: nowhere near it
    CHECK(y == 10 * u, "ZL stays put: y %.1f", y);
    y = touch_avoid_cutouts(1480, 6 * u, e, m, 1848, top, 0);
    CHECK(y == 6 * u, "no cutouts: nothing moves");

    // ---- the output's shape (Video > Aspect ratio "Unlocked", game 1.8.2: reshape allowed)
    OutScale o;
    g_settings.res_mode = 1;
    output_scale(2960, 1848, true, 1.0, -1, &o);  // Tab S10 Ultra, 16:10
    CHECK(o.shape == 1.0, "16:10: the image keeps 16:9 (shape %.3f; the widescreen hooks only widen)", o.shape);
    CHECK(near(o.vw, 2960, 0.5) && near(o.vh, 1665, 0.5), "16:10: fitted to the width, %.0f x %.0f", o.vw, o.vh);
    CHECK(near(o.vy, 91.5, 0.5) && near(o.vx, 0, 0.5), "16:10: bars of %.1f px above and below", o.vy);
    CHECK(near(o.sx / o.sy, 1.0, 1e-9), "16:10: final image targets not squeezed (sx %.3f sy %.3f)", o.sx, o.sy);
    output_scale(3200, 1440, true, 1.0, -1, &o);  // the 20:9 phone: widened as before
    CHECK(near(o.shape, 1.25, 1e-6) && o.vw == 3200 && o.vh == 1440, "20:9 phone: widened, shape %.3f", o.shape);
    output_scale(2960, 1848, false, 1.0, -1, &o);  // Original 16:9 (or 1.0.0 / 1.6.0): the same letterbox
    CHECK(o.shape == 1.0 && near(o.vh, 1665, 0.5), "16:10, Original 16:9: %.0f x %.0f", o.vw, o.vh);

    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}
