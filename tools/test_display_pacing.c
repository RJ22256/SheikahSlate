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
    double report_offset;          // the system reports present times this much later than the frames went up (ns)
    int frames;
    double fps;                    // the game's frame rate (0: 30)
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
        double fp = 1e9 / (s->fps > 0 ? s->fps : 30);
        double period = s->pace ? dp_locked_period(res->p.refresh, fp) : fp;
        if (i > 1) grid += period;
        if (s->hitch_at && i == s->hitch_at) grid += 150e6;  // (a stall: the grid starts again after it)
        double q = grid + s->render_lo + urand() * (s->render_hi - s->render_lo);
        double g = q + s->gpu_lo + urand() * (s->gpu_hi - s->gpu_lo);
        if (s->spike_every > 0 && urand() < 1.0 / s->spike_every) g += s->spike_ns;
        // the records available by this present
        while (next_feedback < i && avail_at[next_feedback] <= q) {
            double off = shown_at[next_feedback] ? s->report_offset : 0;
            dp_feedback(&res->p, (uint32_t)next_feedback, (uint64_t)(shown_at[next_feedback] + off),
                        s->no_earliest ? 0 : (uint64_t)(earliest_at[next_feedback] + off));
            next_feedback++;
        }
        uint64_t desired = dp_desired(&res->p, (uint32_t)i, (uint64_t)q, (uint64_t)fp, s->pace);
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

// (WSword) Frame generation: each game frame's `m` presents come together - the frames generated between the frame
// before and this one (each ready fg_ms after the one before it, on the GPU) and then this frame itself - asked for
// fps x m per second, burst_pos telling dp_desired a present's place in its group. The compositor as in run(); m = 1 is
// the plain case.
static void run_burst(const Sim *s, int m, double fg_ms, Result *res) {
    memset(res, 0, sizeof *res);
    dp_reset(&res->p, s->nominal);
    double grid = 1e9;
    int next_feedback = 1, prev_id = 0, id = 0;
    for (int i = 1; i <= s->frames && id + m < MAXF; i++) {
        double fp = 1e9 / (s->fps > 0 ? s->fps : 30);
        double period = s->pace ? dp_locked_period(res->p.refresh, fp) : fp;
        if (i > 1) grid += period;
        double q = grid + s->render_lo + urand() * (s->render_hi - s->render_lo);
        double g = q + s->gpu_lo + urand() * (s->gpu_hi - s->gpu_lo);
        for (int j = 0; j < m; j++) {
            id++;
            double qj = q + j * 0.3e6, gj = g + (j + 1) * fg_ms * 1e6;  // (the presents' calls: a fraction of a ms apart)
            while (next_feedback < id && avail_at[next_feedback] <= qj) {
                dp_feedback(&res->p, (uint32_t)next_feedback, (uint64_t)shown_at[next_feedback],
                            s->no_earliest ? 0 : (uint64_t)earliest_at[next_feedback]);
                next_feedback++;
            }
            res->p.burst_pos = j, res->p.burst_n = m;
            uint64_t desired = dp_desired(&res->p, (uint32_t)id, (uint64_t)qj, fp / m, s->pace);
            double ready = gj + 0.5e6;
            if (desired && desired > ready) ready = (double)desired;
            double R = s->true_refresh;
            double v = s->phase + ceil((ready - s->phase) / R) * R;
            earliest_at[id] = s->phase + ceil((gj + 0.5e6 - s->phase) / R) * R;
            if (prev_id && shown_at[prev_id] >= v) shown_at[prev_id] = 0;
            shown_at[id] = v;
            avail_at[id] = v + 3 * R;
            res->total++;
            res->latency_sum += v - grid;
            prev_id = id;
        }
    }
    double last = 0;
    for (int k = 31 * m; k <= id; k++) {
        if (!shown_at[k]) { res->dropped++; continue; }
        if (last) {
            double d = (shown_at[k] - last) / s->true_refresh;
            int kk = (int)(d + 0.5);
            if (fabs(d - kk) > 0.25) res->hist[0]++;
            else res->hist[kk < 9 ? kk : 9]++;
        }
        last = shown_at[k];
        res->shown++;
    }
}

static double share(const Result *r, int k) {
    int t = 0;
    for (int i = 0; i < 10; i++) t += r->hist[i];
    return t ? (double)r->hist[k] / t : 0;
}

static void report(const char *name, const Sim *s, const Result *r) {
    printf("%-34s %s: 3x %4.1f%% 4x %5.1f%% 5x %4.1f%% 2x %5.1f%% 1x %5.1f%% 6x+ %4.1f%% uneven %d, dropped %d, latency %.1f ms, refresh %.4f ms\n",
           name, s->pace ? "paced  " : "unpaced", 100 * share(r, 3), 100 * share(r, 4), 100 * share(r, 5), 100 * share(r, 2), 100 * share(r, 1),
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

    // 5b. Turbo: 40 fps on 120 Hz, 3 refreshes a frame (render 8-14 ms, GPU 2-9 ms after the present).
    s = base;
    s.fps = 40, s.render_lo = 8e6, s.render_hi = 14e6, s.gpu_lo = 2e6, s.gpu_hi = 9e6;
    s.pace = false;
    run(&s, &r);
    report("120 Hz, 40 fps", &s, &r);
    double unpaced3 = share(&r, 3);
    s.pace = true;
    run(&s, &r);
    report("120 Hz, 40 fps", &s, &r);
    expect(share(&r, 3) >= 0.97, "40 fps paced: 97% at 3 refreshes");
    expect(share(&r, 3) > unpaced3, "40 fps paced better than unpaced");
    expect(r.dropped == 0, "40 fps paced: no frame overtaken");

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

    // 6b. A system that reports present times a refresh late (or 2 ms late, or a refresh early): still even, and the
    // grid doesn't run away.
    {
        double offs[] = {1e9 / 120.0, 2e6, -1e9 / 120.0};
        const char *names[] = {"120 Hz, times reported +1 refresh", "120 Hz, times reported +2 ms", "120 Hz, times reported -1 refresh"};
        for (int k = 0; k < 3; k++) {
            s = base;
            s.report_offset = offs[k];
            s.pace = true;
            run(&s, &r);
            report(names[k], &s, &r);
            expect(share(&r, 4) >= 0.97, "an offset in the reported times: 97% at 4 refreshes");
            expect(r.latency_sum / r.total < 50e6, "an offset in the reported times: latency stays under 50 ms");
        }
    }

    // 6c. (WSword) Frame generation's presents in groups: 30 fps x 2 = 60 on 120 Hz (each up 2 refreshes), x 4 = 120
    // (1 each), 40 x 3 = 120, 60 x 2 = 120; a generated frame 3 ms of GPU each. Unpaced, the generated frame went up a
    // refresh before its real one, then that one for 3 (the S25's log, 0.2.8-slate.15).
    {
        struct { double fps; int m; const char *name; } fg[] = {{30, 2, "120 Hz, 30 fps x 2 (frame gen)"},
                                                                 {30, 4, "120 Hz, 30 fps x 4 (frame gen)"},
                                                                 {40, 3, "120 Hz, 40 fps x 3 (frame gen)"},
                                                                 {60, 2, "120 Hz, 60 fps x 2 (frame gen)"}};
        for (int k = 0; k < 4; k++) {
            s = base;
            s.fps = fg[k].fps;
            if (fg[k].fps > 30) s.render_lo = 6e6, s.render_hi = 9e6, s.gpu_lo = 1e6, s.gpu_hi = 4e6;
            int each = (int)(120 / fg[k].fps / fg[k].m + 0.5);
            s.pace = false;
            run_burst(&s, fg[k].m, 3, &r);
            report(fg[k].name, &s, &r);
            double unpaced = share(&r, each);
            s.pace = true;
            run_burst(&s, fg[k].m, 3, &r);
            report(fg[k].name, &s, &r);
            char what[120];
            snprintf(what, sizeof what, "%s paced: 95%% up for %d refresh(es) each", fg[k].name, each);
            expect(share(&r, each) >= 0.95, what);
            snprintf(what, sizeof what, "%s paced: better than unpaced", fg[k].name);
            expect(share(&r, each) > unpaced, what);
            snprintf(what, sizeof what, "%s paced: at most 1%% overtaken", fg[k].name);
            expect(r.dropped <= r.total / 100, what);
        }
    }

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
    expect(dp_frames_per(1e9 / 120, 1e9 / 40) == 3, "40 fps on 120 Hz (Turbo): 3");
    expect(dp_frames_per(1e9 / 60, 1e9 / 40) == 0, "40 fps on 60 Hz: not whole");
    expect(dp_frames_per(1e9 / 90, 1e9 / 60) == 0, "60 fps on 90 Hz: not whole");
    expect(dp_frames_per(1e9 / 120, 1e9 / 300) == 0, "300 fps on 120 Hz: none");

    // 8b. (WSword) Frame generation's multiplier on the display, and a context's passes for fewer frames.
    {
        double r120 = 1e9 / 120, r60 = 1e9 / 60, r90 = 1e9 / 90;
        struct { double r; int want, fps, got; const char *what; } c[] = {
            {r120, 1, 30, 1, "30 fps 2x on 120 Hz: 2x (2 refreshes each)"},
            {r120, 2, 30, 1, "30 fps 3x on 120 Hz: 2x (3x is 90 a second, uneven)"},
            {r120, 3, 30, 3, "30 fps 4x on 120 Hz: 4x"},
            {r120, 2, 40, 2, "40 fps 3x on 120 Hz: 3x"},
            {r120, 3, 40, 2, "40 fps 4x on 120 Hz: 3x (160 is past 120)"},
            {r120, 1, 40, 1, "40 fps 2x on 120 Hz: 2x kept (80, uneven, but the screen shows it)"},
            {r120, 3, 60, 1, "60 fps 4x on 120 Hz: 2x"},
            {r60, 3, 30, 1, "30 fps 4x on 60 Hz: 2x"},
            {r60, 1, 60, 0, "60 fps on 60 Hz: none"},
            {r90, 2, 30, 2, "30 fps 3x on 90 Hz: 3x"},
            {0, 3, 30, 3, "display rate unknown: as asked"},
        };
        for (unsigned k = 0; k < sizeof c / sizeof c[0]; k++) {
            int g = dp_fg_frames(c[k].r, c[k].want, c[k].fps);
            if (g != c[k].got) printf("  got %d\n", g);
            expect(g == c[k].got, c[k].what);
        }
        int w[3];
        expect(dp_fg_subset(3, 1, w) == 1 && w[0] == 1, "a 4x context makes 2x's frame with its middle pass");
        expect(dp_fg_subset(3, 3, w) == 3 && w[0] == 0 && w[1] == 1 && w[2] == 2, "a 4x context at 4x: all three");
        expect(dp_fg_subset(2, 1, w) == 0, "a 3x context has no halfway pass: none");
        expect(dp_fg_subset(1, 3, w) == 1 && w[0] == 0, "no more than the context makes");
        printf("frame generation multipliers checked\n");
    }

    // 10. (WSword PC) fill pacing without present times: a FIFO swapchain, each real frame's presents (its generated
    // ones and itself) each presented k times in a burst (dp_fill_repeats), the game's clock on the display's grid from
    // the rate Windows reports (a little off the true one). Every present should stay up for one refresh, the queue's
    // depth stay bounded, and without the repeats frames go up unevenly.
    {
        expect(dp_fill_repeats(1e9 / 120, 30, 1, 6) == 4, "fill: 30 fps on 120 Hz, 4 presents a frame");
        expect(dp_fill_repeats(1e9 / 60, 30, 1, 6) == 2, "fill: 30 fps on 60 Hz, 2");
        expect(dp_fill_repeats(1e9 / 120, 30, 2, 6) == 2, "fill: 2x at 30 fps on 120 Hz, each twice");
        expect(dp_fill_repeats(1e9 / 120, 30, 4, 6) == 1, "fill: 4x at 30 fps on 120 Hz, none needed");
        expect(dp_fill_repeats(1e9 / 144, 60, 1, 6) == 1, "fill: 60 fps on 144 Hz, not whole");
        expect(dp_fill_repeats(1e9 / 240, 30, 1, 6) == 1, "fill: 30 fps on 240 Hz, 8 needed - more than 6");
        expect(dp_fill_repeats(1e9 / 59.94, 30, 1, 6) == 2, "fill: 30 fps on 59.94 Hz, 2");
        struct { double hz, reported; int fps, presents, images; const char *what; } c[] = {
            {120.0, 119.998, 30, 1, 6, "fill 30 fps on 120 Hz"},
            {59.9401, 59.94, 30, 1, 4, "fill 30 fps on 59.94 Hz"},
            {120.0, 120.0, 30, 2, 6, "fill 2x at 30 fps on 120 Hz"},
            {60.0, 60.0, 30, 2, 4, "2x at 30 fps on 60 Hz (no repeats needed)"},
        };
        for (unsigned ci = 0; ci < sizeof c / sizeof c[0]; ci++) {
            for (int fill = 0; fill < 2; fill++) {
                double R = 1e9 / c[ci].hz, Rk = 1e9 / c[ci].reported;
                int k = fill ? dp_fill_repeats(Rk, c[ci].fps, c[ci].presents, 6) : 1;
                int burst = c[ci].presents * k;
                double P = dp_locked_period(Rk, 1e9 / c[ci].fps);  // (the game's clock, locked to the reported rate)
                int cap = c[ci].images - 1;  // (presents waiting in FIFO's queue at most: one image is on screen)
                double q_ready[64];
                int qn = 0;
                double vb = R * 0.37;  // (the first vblank's phase)
                double game_t = 0, next_present = 0;
                int shown_for = 0, counts[10] = {0}, maxq = 0, frames_done = 0;
                double lat_sum = 0;
                int lat_n = 0;
                double q_enq[64];
                for (int n = 0; frames_done < 3000; n++) {
                    // the game's frame n done on its grid (late when back-pressure held it), its presents queued
                    double t = n * P + urand() * 4e6;
                    if (t < game_t) t = game_t;
                    for (int b = 0; b < burst; b++) {
                        double enq = t + b * 0.2e6;
                        while (qn >= cap) {  // (no image free: wait for a vblank to take one)
                            if (q_ready[0] <= vb - 0.5e6) {
                                if (shown_for) counts[shown_for < 9 ? shown_for : 9]++;
                                shown_for = 0;
                                lat_sum += vb - q_enq[0], lat_n++;
                                memmove(q_ready, q_ready + 1, --qn * sizeof *q_ready);
                                memmove(q_enq, q_enq + 1, qn * sizeof *q_enq);
                            }
                            shown_for++;
                            vb += R;
                            if (enq < vb) enq = vb;
                        }
                        double gpu = 1e6 + urand() * 5e6;  // (the present's GPU work: 1-6 ms)
                        q_enq[qn] = enq;
                        q_ready[qn++] = (next_present > enq ? next_present : enq) + gpu;
                        next_present = q_ready[qn - 1];
                        game_t = enq;
                    }
                    if (qn > maxq) maxq = qn;
                    // vblanks up to the next frame's time
                    double until = (n + 1) * P;
                    while (vb < until) {
                        if (qn && q_ready[0] <= vb - 0.5e6) {
                            if (shown_for && n > 100) counts[shown_for < 9 ? shown_for : 9]++;
                            shown_for = 0;
                            if (n > 100) lat_sum += vb - q_enq[0], lat_n++;
                            memmove(q_ready, q_ready + 1, --qn * sizeof *q_ready);
                            memmove(q_enq, q_enq + 1, qn * sizeof *q_enq);
                        }
                        shown_for++;
                        vb += R;
                    }
                    if (n > 100) frames_done++;
                }
                int total = 0;
                for (int i = 0; i < 10; i++) total += counts[i];
                double one = total ? (double)counts[1] / total : 0;
                printf("%s %s: k %d, %.1f%% of presents up for 1 refresh (2: %.1f%%, 3+: %.1f%%), queue at most %d, "
                       "latency %.1f ms\n", c[ci].what, fill ? "with repeats" : "without", k, 100 * one,
                       total ? 100.0 * counts[2] / total : 0,
                       total ? 100.0 * (total - counts[1] - counts[2]) / total : 0, maxq, lat_n ? lat_sum / lat_n / 1e6 : 0);
                if (fill && (k > 1 || c[ci].presents * c[ci].fps >= c[ci].hz * 0.99)) {
                    expect(one >= 0.97, c[ci].what);
                    expect(lat_n && lat_sum / lat_n < 6 * R, "fill: the queue's delay stays bounded");
                }
                int each = (int)lround(c[ci].hz / ((double)c[ci].fps * c[ci].presents));
                if (!fill && each >= 2 && each <= 9) {
                    printf("  without repeats: %.1f%% of presents up for the %d refreshes each should be\n",
                           total ? 100.0 * counts[each] / total : 0, each);
                }
            }
        }
        printf("fill pacing checked\n");
    }

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

    {
        // 5c. (WSword) 60 fps on 120 Hz, frames done 8-14 ms after their present (render 3-8 ms after the grid): the time to
        // the screen is often over 1.5 refreshes, which put every frame "ahead" of a grid that included it - a new grid each
        // frame, frames up for 1, 2 or 3 refreshes as unpaced (the Galaxy Tab S10 Ultra's log, 2026-11-02).
        Sim s = base;  // (last: the cases before keep their random sequences)
        s.fps = 60, s.render_lo = 3e6, s.render_hi = 8e6, s.gpu_lo = 8e6, s.gpu_hi = 14e6, s.frames = 7200;
        s.pace = false;
        run(&s, &r);
        report("120 Hz, 60 fps, slow compositor", &s, &r);
        double unpaced2 = share(&r, 2);
        s.pace = true;
        run(&s, &r);
        report("120 Hz, 60 fps, slow compositor", &s, &r);
        expect(share(&r, 2) >= 0.98, "60 fps paced with the compositor's latency: 98% at 2 refreshes");
        expect(share(&r, 2) > unpaced2, "60 fps paced better than unpaced");
        expect(r.p.ahead < 10 && r.p.regrids < 10, "60 fps paced: the grid kept (under 10 new grids in 2 minutes)");
        expect(r.dropped == 0, "60 fps paced: no frame overtaken");
        // 5d. ... and across a 150 ms hitch, and with a slow GPU frame now and then
        s.hitch_at = 3000, s.spike_every = 60, s.spike_ns = 6e6;
        run(&s, &r);
        report("120 Hz, 60 fps, hitch + spikes", &s, &r);
        expect(share(&r, 2) >= 0.90, "60 fps paced across a hitch and spikes: 90% at 2 refreshes");
        expect(r.dropped <= 1, "60 fps paced across a hitch: at most one frame overtaken");
    }

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("display pacing: all checks passed\n");
    return 0;
}
