// The on-screen controls' measure (ui/touch_controls.mm), kept apart so it can be tested on the desktop
// (tools/test_touch_units.c). Sheikah Slate, for the Galaxy Tab S10 Ultra.
//
// The layout is drawn in units: upstream's unit is 1% of the screen's height, made on a 6.7" phone, where it is about
// 0.7 mm. On a 14.6" tablet that unit is 2 mm - face buttons 3 cm across, the left stick's resting place 5 cm from
// the corner, out of a thumb's reach while holding the tablet by its sides. So the unit is that share of the height,
// but at most TOUCH_UNIT_MM millimetres of the screen (its physical density from the activity: LARECOMP_SCREEN_DPI):
// phones keep the layout they had, big screens get thumb-sized controls near the edges (on the Tab S10 Ultra, 239 ppi:
// 8.9 px a unit instead of 18.5). The size slider (Controls tab) still scales them.
#pragma once

#define TOUCH_UNIT_MM 0.95f

static inline float touch_unit_px(float disp_h, float dpi) {
    float share = disp_h / 100.0f;
    if (!(dpi >= 100.0f && dpi <= 1000.0f)) return share;  // (unknown or implausible density: as upstream)
    float mm = dpi / 25.4f * TOUCH_UNIT_MM;
    return mm < share ? mm : share;
}

// A control centred at (x, y), extending e pixels each way, moved up or down out of the screen's cutouts (the camera
// notch: n rectangles, left / top / right / bottom each, in window pixels) with a margin: down below one in the top
// half of the screen, up above one in the bottom half. Returns the new y (y itself when it overlaps none).
static inline float touch_avoid_cutouts(float x, float y, float e, float margin, float disp_h, const int *rects, int n) {
    for (int pass = 0; pass < 2; pass++)  // (a second pass: moved out of one, into another)
        for (int i = 0; i < n; i++) {
            const int *r = rects + 4 * i;
            float l = (float)r[0] - margin, t = (float)r[1] - margin, rr = (float)r[2] + margin, b = (float)r[3] + margin;
            if (x + e <= l || x - e >= rr || y + e <= t || y - e >= b) continue;
            float mid = ((float)r[1] + (float)r[3]) * 0.5f;
            y = mid < disp_h * 0.5f ? b + e : t - e;
        }
    return y;
}
