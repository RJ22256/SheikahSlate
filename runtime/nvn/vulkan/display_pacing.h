// Even frame pacing on a fast display (Sheikah Slate): each frame aimed at a refresh of its own, a whole number of
// refreshes after the one before - 4 at 30 fps on the Galaxy Tab S10 Ultra's 120 Hz panel. Without it a frame goes up
// at the first refresh after the GPU finishes it, and GPU times that differ by a few milliseconds from frame to frame
// left frames up for 3, 4 or 5 refreshes (25, 33 or 42 ms) at a steady 30 fps.
// The logic only (no Vulkan): device.c feeds it the presents' times as the system reports them
// (VK_GOOGLE_display_timing) and passes on the time it asks for; tools/test_display_pacing.c runs it against a
// simulated display.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define DP_SENT 64

typedef struct {
    double nominal;         // ns per refresh, as the system reports it (0: unknown - no pacing)
    double refresh;         // the same, refined from the frames' present times
    int n;                  // refreshes per frame at the current frame rate (0: not a whole number - no pacing)
    bool anchored;          // a grid of slots to aim frames at
    uint32_t gen;           // the grid's generation: frames are judged only against the grid they were aimed with
    uint32_t anchor_id;     // the frame the slots count from
    double anchor_t;        // its slot (a refresh, ns on CLOCK_MONOTONIC)
    int late_run;           // frames in a row presented after their slot had gone by (dp_desired)
    int nrun;               // aimed frames in a row shown half a refresh or more off their slot, the same way: how far
    double run_err[4];
    int burst_pos;          // (WSword, frame generation) the next present's place in its frame's group - the generated
    int burst_n;            // frames, then the real one, presented together - and the group's size (0 / 1: no groups). A
                            // group is judged by its first present (the least time to be ready); slots up to burst_pos
                            // frames ahead are a later one's own
    double bias;            // a steady offset between when the system says frames went up and when they were asked for
    double anchor_lat;      // the grid's first frame: from its present (or the time it asked for) to on screen - frames
                            // present that far ahead of their slots all the time (the compositor's own latency), so only
                            // more than a frame beyond it is a frame ahead (dp_desired)
    int tight_n;            // aimed frames in a row on their slots, and of them the least spare (could have gone up
    double tight_min;       // that much earlier: the system's earliest time): a refresh or more over 300 - a slot earlier
    bool have_last;
    uint32_t last_id;
    uint64_t last_t;
    struct { uint32_t id, gen; uint64_t desired, at; uint8_t pos, group; } sent[DP_SENT];  // the last frames' requests, by id
    // since dp_stats_take:
    uint32_t shown[9];      // frames by how long the one before stayed up: [k] k refreshes (1..7), [8] longer, [0] other
    uint32_t aimed, on_time, late, early, regrids, ahead, missing, tightened, offsets;
} DPace;

// A new swapchain or refresh rate: everything learnt starts again. nominal_ns 0: unknown (no pacing).
void dp_reset(DPace *p, double nominal_ns);
// The system's refresh period again (a display mode change): more than 1% off starts again.
void dp_set_nominal(DPace *p, double nominal_ns);
// Refreshes per frame for a frame period (0: not a whole number of this display's refreshes).
int dp_frames_per(double refresh_ns, double period_ns);
// The time to ask for frame id, presented now at the given frame period (0: none - as soon as it's ready). aim: false
// only records the frame (measuring, no pacing).
uint64_t dp_desired(DPace *p, uint32_t id, uint64_t now_ns, double period_ns, bool aim);
// A past present's time on screen, as the system reports it (actual_ns 0: never shown), and the earliest it could have
// gone up (earliest_ns 0: unknown).
void dp_feedback(DPace *p, uint32_t id, uint64_t actual_ns, uint64_t earliest_ns);
// The frame period on this display: n refreshes when the period is close to that, else the period itself.
double dp_locked_period(double refresh_ns, double period_ns);
// (WSword, frame generation) The frames to make between two real ones at `fps` on this display, of the `want` asked for:
// never more than the display shows (frames past its rate are made and never go up), a whole number of refreshes each
// where one fits (30 fps at 3x on 120 Hz is 90 a second, uneven: 2x). 0: the game fills the display's rate already.
// refresh_ns 0 (unknown): want.
int dp_fg_frames(double refresh_ns, int want, int fps);
// Of a context making n frames between two real ones, the passes for `eff` of them evenly between (4x's middle one for
// 2x) into which[]: how many (eff lowered until n + 1 is a multiple of eff + 1).
int dp_fg_subset(int n, int eff, int *which);
// (WSword PC, a driver without present times) How many times to present each of a real frame's `presents` (it and its
// generated ones) so a FIFO swapchain shows every present for one refresh: the refreshes each stays up at `fps` on this
// display (30 fps on 120 Hz: 4; 2x at 30 on 120 Hz: 2) - 1 where that isn't a whole number (60 fps on 144 Hz), or where
// more than max_burst presents a real frame would be needed.
int dp_fill_repeats(double refresh_ns, int fps, int presents, int max_burst);
// ", screen 4x 97% (3x 1%, 5x 2%), paced: late 3, ..." for the stats line; resets the counters. Returns its length.
int dp_stats_take(DPace *p, char *out, int size, bool paced);
