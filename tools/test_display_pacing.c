// Desktop test of runtime/nvn/vulkan/display_pacing.c against a simulated display: a 120 Hz (or 60 Hz) panel whose
// clock drifts a little from ours, the game's frame grid (framepace.c: 30 fps, locked to the panel's refresh when
// pacing), a render thread that presents each frame some milliseconds after its grid time, a GPU that finishes it
// some milliseconds after that, and a compositor that shows a frame at the first refresh after it is ready and not
// before the time it asked for (in order; a frame overtaken at the same refresh is never shown). The present times
// come back a few refreshes after they happen, as Android reports them.
//   clang -O1 -Iruntime/nvn/vulkan -o build/test_display_pacing tools/test_display_pacing.c \
//         runtime/nvn/vulkan/display_pacing.c -lm && build/test_display_pacing
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display_pacing.h"

static uint64_t rng = 88172645463325252ull;
static double urand(void) {  // [0, 1)
    rng ^= rng << 13, rng ^= rng >> 7, rng ^= rng << 17;
    return (double)(rng >> 11) / 9007199254740992.0;
}

typedef struct {
    double true_refresh, nominal;  // ns
    double phase;                  // the panel's first refresh
    double render_lo, render_hi;   // present call after the frame's grid time (ns)
    double gpu_lo, gpu_hi;         // GPU done after the present call (ns)
    double spike_every, spike_ns;  // now and then a frame's GPU takes this much longer
    int hitch_at;                  // a frame whose game work stalls 150 ms (the grid starts again): 0 none
    bool ignore_desired;           // a compositor that ignores the asked-for times
    bool pace;                     // ask for times (else only measure)
    bool no_earliest;              // the system doesn't report the earliest times
    int frames;
} Sim;

typedef struct {
    int hist[10];  // on-screen time of each shown frame (to the next shown one) in refreshes; [0] uneven, [9] 9+
    int shown, dropped, total;
    double latency_sum;  // grid time -> on screen
    DPace p;
} Result;

#define MAXF 20000
static double shown_at[MAXF], avail_at[MAXF], earliest_at[MAXF];

static void run(const Sim *s, Result *res) {
    memset(res, 0, sizeof *res);
    dp_reset(&res->p, s->nominal);
    double grid = 1e9, last_vsync_shown = 0;
    int next_feedback = 1;
    double prev_shown = 0;
    int prev_id = 0;
    for (int i = 1; i <= s->frames && i < MAXF; i++) {
        double period = s->pace ? dp_locked_period(res->p.refresh, 1e9 / 30) : 1e9 / 30;
        if (i > 1) grid += period;
        if (s->hitch_at && i == s->hitch_at) grid += 150e6;  // (a stall: the grid starts again after it)
        double q = grid + s->render_lo + urand() * (s->render_hi - s->render_lo);
        double g = q + s->gpu_lo + urand() * (s->gpu_hi - s->gpu_lo);
        if (s->spike_every > 0 && urand() < 1.0 / s->spike_every) g += s->spike_ns;
        // the records available by this present
        while (next_feedback < i && avail_at[next_feedback] <= q) {
            dp_feedback(&res->p, (uint32_t)next_feedback, (uint64_t)shown_at[next_feedback],
                        s->no_earliest ? 0 : (uint64_t)earliest_at[next_feedback]);
            next_feedback++;
        }
        uint64_t desired = dp_desired(&res->p, (uint32_t)i, (uint64_t)q, 1e9 / 30, s->pace);
        double ready = g + 0.5e6;  // (the compositor's latch: half a millisecond before the refresh)
        if (!s->ignore_desired && desired && desired > ready) ready = (double)desired;
        // the first refresh at or after ready; a frame eligible at the refresh the one before was waiting for goes up
        // instead of it (in order: the one before is never shown)
        double R = s->true_refresh;
        double v = s->phase + ceil((ready - s->phase) / R) * R;
        earliest_at[i] = s->phase + ceil((g + 0.5e6 - s->phase) / R) * R;
        if (prev_id && shown_at[prev_id] >= v) shown_at[prev_id] = 0;
        shown_at[i] = v;
        avail_at[i] = v + 3 * R;  // (reported a few refreshes later)
        (void)last_vsync_shown;
        res->total++;
        res->latency_sum += v - grid;
        (void)prev_shown;
        prev_id = i;
    }
    // histogram of the shown frames, after the first second (the grid settling)
    double last = 0;
    for (int i = 31; i <= s->frames && i < MAXF; i++) {
        if (!shown_at[i]) { res->dropped++; continue; }
        if (last) {
            double d = (shown_at[i] - last) / s->true_refresh;
            int k = (int)(d + 0.5);
            if (fabs(d - k) > 0.25) res->hist[0]++;
            else res->hist[k < 9 ? k : 9]++;
        }
        last = shown_at[i];
        res->shown++;
    }
}

static double share(const Result *r, int k) {
    int t = 0;
    for (int i = 0; i < 10; i++) t += r->hist[i];
    return t ? (double)r->hist[k] / t : 0;
}

static void report(const char *name, const Sim *s, const Result *r) {
    printf("%-34s %s: 3x %4.1f%% 4x %5.1f%% 5x %4.1f%% 2x %4.1f%% 6x+ %4.1f%% uneven %d, dropped %d, latency %.1f ms, refresh %.4f ms\n",
           name, s->pace ? "paced  " : "unpaced", 100 * share(r, 3), 100 * share(r, 4), 100 * share(r, 5), 100 * share(r, 2),
           100 * (share(r, 6) + share(r, 7) + share(r, 8) + share(r, 9)), r->hist[0], r->dropped, r->latency_sum / r->total / 1e6,
           r->p.refresh / 1e6);
}

static int failures;
static void expect(bool ok, const char *what) {
    if (!ok) { printf("FAIL: %s\n", what); failures++; }
}

int main(void) {
    Sim base = {.true_refresh = 1e9 / 120.0 * 1.0003, .nominal = 1e9 / 120.0, .phase = 1.0e9 + 3.1e6,
                .render_lo = 12e6, .render_hi = 18e6, .gpu_lo = 2e6, .gpu_hi = 11e6, .frames = 3600};
    Result r;

    // 1. The tablet's case: 30 fps on 120 Hz, a few ms of jitter in when frames are ready.
    Sim s = base;
    s.pace = false;
    run(&s, &r);
    report("120 Hz, jitter 9 ms", &s, &r);
    double unpaced4 = share(&r, 4);
    s.pace = true;
    run(&s, &r);
    report("120 Hz, jitter 9 ms", &s, &r);
    expect(share(&r, 4) >= 0.98, "paced: 98% of frames up for 4 refreshes");
    expect(share(&r, 4) > unpaced4, "paced better than unpaced");
    expect(r.dropped == 0, "paced: no frame overtaken");
    expect(fabs(r.p.refresh - base.true_refresh) < 2000, "the refresh period learnt within 2 us");

    // 2. Now and then a slow GPU frame (+12 ms, 1 in 40): only those frames late.
    s = base;
    s.spike_every = 40, s.spike_ns = 12e6;
    s.pace = false;
    run(&s, &r);
    report("120 Hz, spikes 1 in 40", &s, &r);
    s.pace = true;
    run(&s, &r);
    report("120 Hz, spikes 1 in 40", &s, &r);
    expect(share(&r, 4) >= 0.90, "paced with spikes: 90% at 4 refreshes");
    expect(r.dropped == 0, "paced with spikes: no frame overtaken");

    // 3. A 150 ms stall (a loading hitch): back on the grid after.
    s = base;
    s.hitch_at = 1500;
    s.pace = true;
    run(&s, &r);
    report("120 Hz, a 150 ms hitch", &s, &r);
    expect(share(&r, 4) >= 0.97, "paced across a hitch: 97% at 4 refreshes");
    expect(r.dropped <= 1, "paced across a hitch: at most one frame overtaken");

    // 4. More jitter than the slack (render 8-20 ms, GPU 2-16 ms): the grid moves later until frames fit.
    s = base;
    s.render_lo = 8e6, s.render_hi = 20e6, s.gpu_lo = 2e6, s.gpu_hi = 16e6;
    s.pace = false;
    run(&s, &r);
    report("120 Hz, jitter 26 ms", &s, &r);
    unpaced4 = share(&r, 4);
    s.pace = true;
    run(&s, &r);
    report("120 Hz, jitter 26 ms", &s, &r);
    expect(share(&r, 4) >= unpaced4, "paced with big jitter: no worse than unpaced");
    expect(r.dropped == 0, "paced with big jitter: no frame overtaken");

    // 5. A 60 Hz panel: 2 refreshes a frame.
    s = base;
    s.true_refresh = 1e9 / 60.0 * 0.9998, s.nominal = 1e9 / 60.0;
    s.pace = true;
    run(&s, &r);
    report("60 Hz, jitter 9 ms", &s, &r);
    expect(share(&r, 2) >= 0.98, "60 Hz paced: 98% at 2 refreshes");

    // 6. A compositor that ignores the asked-for times: nothing worse than without.
    s = base;
    s.ignore_desired = true;
    s.pace = false;
    run(&s, &r);
    double ign_unpaced = share(&r, 4);
    s.pace = true;
    run(&s, &r);
    report("120 Hz, times ignored", &s, &r);
    expect(fabs(share(&r, 4) - ign_unpaced) < 0.02, "times ignored: the same as unpaced");

    // 7. No refresh period from the system: no times asked, no crash.
    s = base;
    s.nominal = 0;
    s.pace = true;
    run(&s, &r);
    report("refresh unknown", &s, &r);
    expect(r.p.aimed == 0, "refresh unknown: nothing aimed");

    // 8. A rate that isn't a whole number of refreshes (30 fps on 90.5 Hz... 40 on 120 is 3): not paced.
    expect(dp_frames_per(1e9 / 120, 1e9 / 30) == 4, "30 fps on 120 Hz: 4");
    expect(dp_frames_per(1e9 / 90, 1e9 / 30) == 3, "30 fps on 90 Hz: 3");
    expect(dp_frames_per(1e9 / 90, 1e9 / 60) == 0, "60 fps on 90 Hz: not whole");
    expect(dp_frames_per(1e9 / 120, 1e9 / 300) == 0, "300 fps on 120 Hz: none");

    // 9. The stats line.
    {
        DPace p;
        dp_reset(&p, 1e9 / 120);
        p.shown[4] = 97, p.shown[3] = 1, p.shown[5] = 2, p.aimed = 100, p.late = 2, p.regrids = 1;
        char line[256];
        dp_stats_take(&p, line, sizeof line, true);
        printf("stats: \"%s\"\n", line);
        expect(strstr(line, "on screen 4x 97%") != NULL, "stats line: the main share");
        expect(strstr(line, "5x 2.0%") != NULL, "stats line: the others");
        expect(p.shown[4] == 0 && p.aimed == 0, "stats line: counters reset");
        char small[8];
        p.shown[4] = 5;
        int n = dp_stats_take(&p, small, sizeof small, false);
        expect(n < (int)sizeof small && small[sizeof small - 1] == 0, "stats line: cut to the buffer");
    }

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("display pacing: all checks passed\n");
    return 0;
}
