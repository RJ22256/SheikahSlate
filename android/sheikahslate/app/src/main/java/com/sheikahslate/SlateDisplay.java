// Sheikah Slate's large-screen setup, made for the Samsung Galaxy Tab S10 Ultra (SM-X920 / SM-X926B / SM-X926N /
// SM-X920N: 14.6" 2960x1848 16:10 Dynamic AMOLED 2X at up to 120 Hz, a camera notch on the top long edge in landscape,
// MediaTek Dimensity 9300+ with an Arm Immortalis-G720 MC12 GPU). Everything here also runs, harmlessly, elsewhere.
// - the device profile and the screen's physical density, for the runtime (LARECOMP_DEVICE_PROFILE, LARECOMP_SCREEN_DPI:
//   runtime/ui/touch_controls.mm sizes the on-screen controls in millimetres on a big screen, not in shares of it);
// - the window drawn under the camera notch on every Android version (LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS: what
//   Android 15+ does anyway for an app targeting API 35+; on One UI 6 / Android 14 SHORT_EDGES left a strip under the
//   long-edge notch), and the notch's rectangles handed to the runtime so no on-screen control sits under it - it
//   moves from the top edge to the bottom one when the tablet is turned around (sensorLandscape);
// - the display's refresh rate (Video > Display refresh rate, runtime/settings.h display_refresh): the highest mode
//   at the current resolution (120 Hz here: the game's 30 fps frames land on a 8.3 ms grid instead of 16.7 ms, so a
//   frame that misses its slot shows for 25 ms rather than 50), 60 Hz to save battery, or One UI's own choice.
package com.sheikahslate;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.DisplayCutout;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;

import java.util.List;

final class SlateDisplay {
    private static final String TAG = "SheikahSlate";

    // Video > Display refresh rate (runtime/settings.h display_refresh)
    static final int REFRESH_HIGHEST = 0, REFRESH_60 = 1, REFRESH_SYSTEM = 2;

    private SlateDisplay() {}

    // The Galaxy Tab S10 Ultra's model numbers (Wi-Fi SM-X920, 5G SM-X926B; the Korean SM-X920N / SM-X926N)
    static boolean isTabS10Ultra() {
        String m = Build.MODEL != null ? Build.MODEL.toUpperCase(java.util.Locale.ROOT) : "";
        return m.startsWith("SM-X920") || m.startsWith("SM-X926");
    }

    // "tab-s10-ultra", "tablet" (smallest width 600 dp or more: Android's large screen) or "phone"
    static String profile(Activity a) {
        if (isTabS10Ultra()) return "tab-s10-ultra";
        Configuration c = a.getResources().getConfiguration();
        return c.smallestScreenWidthDp >= 600 ? "tablet" : "phone";
    }

    // The screen's physical pixels per inch: the panel's own figure on the Tab S10 Ultra (14.6", 2960x1848: 239 ppi),
    // else what the system reports, when plausible; 0 when unknown (the runtime then keeps its share-of-the-screen sizes)
    static float physicalDpi(Activity a) {
        if (isTabS10Ultra()) return 239.0f;
        DisplayMetrics dm = a.getResources().getDisplayMetrics();
        float dpi = (dm.xdpi + dm.ydpi) * 0.5f;
        // (some devices report the density bucket, or garbage: within 20% of each other and of a believable range)
        if (dm.xdpi <= 0 || dm.ydpi <= 0 || Math.abs(dm.xdpi - dm.ydpi) > 0.2f * dpi || dpi < 100 || dpi > 800) return 0;
        return dpi;
    }

    // In onCreate, after SDL's own window setup: drawn under the camera notch too
    static void applyWindow(Activity a) {
        Window w = a.getWindow();
        WindowManager.LayoutParams lp = w.getAttributes();
        if (lp.layoutInDisplayCutoutMode != WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS) {
            lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS;
            w.setAttributes(lp);
        }
    }

    // The display mode to ask for: at the current resolution, the highest refresh rate (or the one nearest 60 Hz);
    // 0 lets the system choose
    static int pickMode(Display d, int how) {
        if (d == null || how == REFRESH_SYSTEM) return 0;
        Display.Mode cur = d.getMode();
        Display.Mode best = null;
        for (Display.Mode m : d.getSupportedModes()) {
            if (m.getPhysicalWidth() != cur.getPhysicalWidth() || m.getPhysicalHeight() != cur.getPhysicalHeight()) continue;
            if (best == null) { best = m; continue; }
            float r = m.getRefreshRate(), b = best.getRefreshRate();
            if (how == REFRESH_60 ? Math.abs(r - 60) < Math.abs(b - 60) : r > b) best = m;
        }
        return best != null ? best.getModeId() : 0;
    }

    // (UI thread) Video > Display refresh rate, from the runtime at start and on each change
    static void applyRefresh(Activity a, int how) {
        try {
            Display d = a.getDisplay();
            int id = pickMode(d, how);
            Window w = a.getWindow();
            WindowManager.LayoutParams lp = w.getAttributes();
            if (lp.preferredDisplayModeId != id) {
                lp.preferredDisplayModeId = id;
                w.setAttributes(lp);
            }
            float hz = 0;
            if (d != null) for (Display.Mode m : d.getSupportedModes()) if (m.getModeId() == id) hz = m.getRefreshRate();
            Log.i(TAG, "display refresh: " + (how == REFRESH_SYSTEM ? "the system's choice"
                                                : (how == REFRESH_60 ? "60 Hz" : "highest") + " -> mode " + id + " (" + hz + " Hz)"));
        } catch (Throwable t) {
            Log.w(TAG, "display refresh: " + t);
        }
    }

    // The camera notch's rectangles in window pixels (left, top, right, bottom each), empty without one; null when the
    // window has no insets yet
    static int[] cutouts(Activity a) {
        View v = a.getWindow().getDecorView();
        WindowInsets in = v.getRootWindowInsets();
        if (in == null) return null;
        DisplayCutout c = in.getDisplayCutout();
        if (c == null) return new int[0];
        List<Rect> rs = c.getBoundingRects();
        int n = Math.min(rs.size(), 4);
        int[] out = new int[n * 4];
        for (int i = 0; i < n; i++) {
            Rect r = rs.get(i);
            out[i * 4] = r.left;
            out[i * 4 + 1] = r.top;
            out[i * 4 + 2] = r.right;
            out[i * 4 + 3] = r.bottom;
        }
        return out;
    }
}
