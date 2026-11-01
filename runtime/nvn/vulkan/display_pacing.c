// Even frame pacing (display_pacing.h). How it goes:
//  - every present carries an id; the system reports, a few frames later, when each one went up on screen (a refresh's
//    time - VK_GOOGLE_display_timing's actualPresentTime);
//  - from one such time the frames ahead get slots: that refresh + DP_SLACK refreshes + k x n refreshes for the k-th
//    frame after it, n = the refreshes per frame (4 at 30 fps on 120 Hz). Each frame asks not to go up before half a
//    refresh ahead of its slot, so it goes up at its slot when it's ready by then, and at the first refresh after when
//    it isn't (late - the frames after keep their slots: one frame up a refresh longer, the next a refresh shorter);
//  - a frame shown on its slot moves the grid onto its real time (the display's clock and ours drift a little);
//    three aimed frames in a row late (or early) start a new grid from the last one's time - unless all three were off
//    by the same amount (within a quarter refresh): then that is how the system reports them, an offset kept and
//    allowed for (the frames go up evenly all the same); a new grid also when a frame's slot is over a frame ahead of
//    its present (the game's frames went out earlier than the grid expects, after a hitch) and after three frames in a
//    row presented past their slots (a hitch: the slots behind the frames for good);
//    300 aimed frames in a row on their slots that could each have gone up a refresh or more earlier (the system's
//    earliest present time) move the grid a refresh earlier: the slack back where frames are steadier than that;
//  - the refresh period itself is refined from the times (the system's figure is a nominal one), and the game's frame
//    grid (framepace.c) runs at n of those, so the frames come as often as their slots do.
// DP_SLACK: one refresh (8.3 ms at 120 Hz) between when frames would go up and their slots, room for frames that take
// a few milliseconds longer than the one the grid started from, at the cost of that much more latency.
#include "display_pacing.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define DP_SLACK 1.0
#define DP_RUN 3  // (aimed frames in a row off their slots before a new grid)

void dp_reset(DPace *p, double nominal_ns) {
    memset(p, 0, sizeof *p);
    p->nominal = p->refresh = nominal_ns > 0 ? nominal_ns : 0;
}

void dp_set_nominal(DPace *p, double nominal_ns) {
    if (nominal_ns <= 0) return;
    if (p->nominal > 0 && fabs(nominal_ns - p->nominal) <= 0.01 * p->nominal) return;
    DPace keep = *p;
    dp_reset(p, nominal_ns);
    // (the counters carry on: they are the stats line's)
    memcpy(p->shown, keep.shown, sizeof p->shown);
    p->aimed = keep.aimed, p->on_time = keep.on_time, p->late = keep.late, p->early = keep.early;
    p->regrids = keep.regrids, p->ahead = keep.ahead, p->missing = keep.missing, p->tightened = keep.tightened;
    p->offsets = keep.offsets;
}

int dp_frames_per(double refresh_ns, double period_ns) {
    if (refresh_ns <= 0 || period_ns <= 0) return 0;
    double x = period_ns / refresh_ns;
    int n = (int)(x + 0.5);
    if (n < 1 || n > 8 || fabs(x - n) > 0.05) return 0;
    return n;
}

double dp_locked_period(double refresh_ns, double period_ns) {
    int n = dp_frames_per(refresh_ns, period_ns);
    return n ? n * refresh_ns : period_ns;
}

static void regrid(DPace *p, uint32_t id, uint64_t actual) {
    p->anchored = true;
    p->gen++;
    p->anchor_id = id;
    p->anchor_t = (double)actual - p->bias + DP_SLACK * p->refresh;
    p->late_run = p->nrun = 0;
    p->tight_n = 0;
    p->regrids++;
}

uint64_t dp_desired(DPace *p, uint32_t id, uint64_t now_ns, double period_ns, bool aim) {
    uint64_t desired = 0;
    int n = aim ? dp_frames_per(p->refresh, period_ns) : 0;
    if (n != p->n) p->n = n, p->anchored = false, p->late_run = p->nrun = 0;
    if (n && p->anchored) {
        int32_t k = (int32_t)(id - p->anchor_id);
        double slot = p->anchor_t + (double)k * n * p->refresh, now = (double)now_ns;
        if (k <= 0 || slot < now) {
            // (past its slot already - a slow frame: as soon as it's ready, and the grid stays; several in a row - after
            // a hitch the slots are behind the frames for good - a new grid from the next frame shown)
            p->late++;
            p->tight_n = 0;
            if (++p->late_run >= DP_RUN) p->anchored = false;
        } else if (slot - now > (n * (p->burst_n > 1 ? p->burst_n + p->burst_pos : 1) + 0.5) * p->refresh) {
            // (more than a frame ahead of its present: a new grid from the next frame shown. WSword, frame generation:
            // a group's presents come together, a real frame's time apart - a frame there is the real one's, and the
            // k-th after the first is k slots further on)
            p->ahead++;
            p->anchored = false;
        } else {
            desired = (uint64_t)(slot - 0.5 * p->refresh);
            p->aimed++;
        }
    }
    unsigned s = id % DP_SENT;
    p->sent[s].id = id;
    p->sent[s].gen = p->gen;
    p->sent[s].desired = desired;
    p->sent[s].pos = (uint8_t)(p->burst_pos > 0 ? p->burst_pos : 0);
    p->sent[s].group = (uint8_t)(p->burst_n > 1 ? p->burst_n : 1);
    return desired;
}

void dp_feedback(DPace *p, uint32_t id, uint64_t actual, uint64_t earliest) {
    if (p->have_last && (int32_t)(id - p->last_id) <= 0) return;  // (an old record: already counted)
    if (!actual) return;  // (not shown: counted as missing by the next record's gap)
    double R = p->refresh;
    if (p->have_last && R > 0) {
        uint32_t gap = id - p->last_id;
        if (gap > 1) p->missing += gap - 1;  // (presents with no record: replaced before they went up, or lost)
        double d = (double)(actual - p->last_t);
        int k = (int)(d / R + 0.5);
        bool whole = k >= 1 && fabs(d - k * R) < 0.25 * R;
        if (gap == 1) p->shown[whole ? (k < 8 ? k : 8) : 0]++;
        if (whole && k <= 12 && p->nominal > 0) {  // (the period: d / k, averaged over ~50 frames, within 1.5%)
            double r = R + (d / k - R) * 0.02 / gap;
            if (r > p->nominal * 1.015) r = p->nominal * 1.015;
            if (r < p->nominal * 0.985) r = p->nominal * 0.985;
            p->refresh = r;
        }
    }
    p->have_last = true;
    p->last_id = id;
    p->last_t = actual;
    if (!p->n || R <= 0) return;
    unsigned s = id % DP_SENT;
    // (WSword, frame generation) a group - the generated frames and the real one, presented together - is judged by its
    // first present alone: it has the least time to be ready (each later one's slot is a frame further on, its GPU work a
    // few milliseconds). The others' on-time records kept the grid where the first one was late every time.
    bool group = p->sent[s].id == id && p->sent[s].group > 1;
    if (group && p->sent[s].pos > 0) return;
    if (!p->anchored) { regrid(p, id, actual); return; }
    if (p->sent[s].id != id || p->sent[s].gen != p->gen || !p->sent[s].desired) return;  // (not aimed with this grid)
    double slot = (double)p->sent[s].desired + 0.5 * R, err = (double)actual - slot - p->bias;
    if (group) {
        // no offsets learnt from groups: a first present late each time is its GPU work, not how the system reports
        // (an offset there put the generated frame a refresh late for good, its real one on time). Two in a row late (or
        // early): a new grid from it; the earlier-by-a-refresh step below as for single frames.
        if (fabs(err) >= 0.5 * R) {
            if (err > 0) p->late++;
            else p->early++;
            p->tight_n = 0;
            if (p->nrun && (p->run_err[0] > 0) != (err > 0)) p->nrun = 0;
            p->run_err[p->nrun++] = err;
            if (p->nrun >= 2) regrid(p, id, actual);
            return;
        }
    }
    if (fabs(err) < 0.5 * R) {
        p->on_time++;
        p->anchor_id = id, p->anchor_t = (double)actual - p->bias;  // (onto the display's real time)
        if (!group) p->bias += err * 0.05;  // (an offset, followed within the half refresh)
        p->late_run = p->nrun = 0;
        if (earliest && earliest <= actual) {
            double spare = (double)(actual - earliest);
            if (!p->tight_n || spare < p->tight_min) p->tight_min = spare;
            if (++p->tight_n >= 300) {
                if (p->tight_min >= 0.75 * R) {  // (a slot earlier; the frames on their way keep theirs, unjudged)
                    p->anchor_t -= R;
                    p->gen++;
                    p->tightened++;
                }
                p->tight_n = 0;
            }
        } else {
            p->tight_n = 0;
        }
        return;
    }
    if (err > 0) p->late++;
    else p->early++;
    p->tight_n = 0;
    if (p->nrun && (p->run_err[0] > 0) != (err > 0)) p->nrun = 0;  // (a run is one way)
    p->run_err[p->nrun++] = err;
    if (p->nrun < DP_RUN) return;
    double lo = p->run_err[0], hi = lo, sum = 0;
    for (int k = 0; k < p->nrun; k++) {
        if (p->run_err[k] < lo) lo = p->run_err[k];
        if (p->run_err[k] > hi) hi = p->run_err[k];
        sum += p->run_err[k];
    }
    double bias = p->bias + sum / p->nrun;
    p->nrun = 0;
    if (hi - lo < 0.25 * R && bias > -2 * R && bias < 2 * R) {
        p->bias = bias;  // (all as far off: how the system reports them - the frames go up evenly)
        p->offsets++;
        return;
    }
    regrid(p, id, actual);
}

int dp_fg_frames(double refresh_ns, int want, int fps) {
    if (refresh_ns <= 0 || want < 1 || fps <= 0) return want < 0 ? 0 : want;
    double hz = 1e9 / refresh_ns;
    for (int g = want; g >= 1; g--) {
        double per = hz / ((double)fps * (g + 1));
        if (per >= 0.97 && fabs(per - (int)(per + 0.5)) < 0.05) return g;
    }
    for (int g = want; g >= 1; g--)
        if ((double)fps * (g + 1) <= hz * 1.03) return g;
    return 0;
}

int dp_fg_subset(int n, int eff, int *which) {
    if (eff > n) eff = n;
    while (eff > 0 && (n + 1) % (eff + 1)) eff--;
    for (int k = 0; k < eff; k++) which[k] = (k + 1) * (n + 1) / (eff + 1) - 1;
    return eff < 0 ? 0 : eff;
}

int dp_stats_take(DPace *p, char *out, int size, bool paced) {
    uint32_t total = 0;
    for (int k = 0; k < 9; k++) total += p->shown[k];
    int len = 0;
    if (size > 0) out[0] = 0;
    if (total) {
        int best = 1;
        for (int k = 1; k < 9; k++) if (p->shown[k] > p->shown[best]) best = k;
        len += snprintf(out + len, (size_t)(size > len ? size - len : 0), ", on screen %dx %.0f%%", best,
                        100.0 * p->shown[best] / total);
        int others = 0;
        for (int k = 0; k < 9; k++) {
            if (k == best || !p->shown[k]) continue;
            char kb[8];
            if (k == 0) snprintf(kb, sizeof kb, "uneven");
            else if (k == 8) snprintf(kb, sizeof kb, "8x+");
            else snprintf(kb, sizeof kb, "%dx", k);
            len += snprintf(out + len, (size_t)(size > len ? size - len : 0), "%s%s %.1f%%", others++ ? ", " : " (", kb,
                            100.0 * p->shown[k] / total);
        }
        if (others) len += snprintf(out + len, (size_t)(size > len ? size - len : 0), ")");
        if (p->missing) len += snprintf(out + len, (size_t)(size > len ? size - len : 0), ", %u not shown", p->missing);
        if (paced)
            len += snprintf(out + len, (size_t)(size > len ? size - len : 0), ", paced: %u aimed, late %u, early %u, regrids %u, ahead %u, earlier %u, offset %.1f ms (%u)",
                            p->aimed, p->late, p->early, p->regrids, p->ahead, p->tightened, p->bias / 1e6, p->offsets);
        else
            len += snprintf(out + len, (size_t)(size > len ? size - len : 0), ", not paced");
    }
    memset(p->shown, 0, sizeof p->shown);
    p->aimed = p->on_time = p->late = p->early = p->regrids = p->ahead = p->missing = p->tightened = p->offsets = 0;
    return len < size ? len : size - 1;
}
