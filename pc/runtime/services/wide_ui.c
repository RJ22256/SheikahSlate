// The HUD, the start screen and the map laid out for a wide window (21:9, 32:9) instead of a 16:9 middle with the
// world around it (Video > Wide HUD and menus, with Fill wide screens). See wide_ui.h.
//
// The game's layouts (nn::ui2d, Layout/Common.sblarc and Layout/Title.sblarc) are 1280 x 720 with the origin in the
// middle; their views are widened with the window (nvn/output_scale.c rt_persp keeps the height), so a wider window
// shows 1280 * shape units across and the layouts stay in the 16:9 middle. Here, before the game reads them, each
// layout of the HUD, the start screen and the map, and the parts they place over the whole screen, is changed in
// place (same sizes, so the archive keeps its layout):
//   - a picture or window as wide as the screen (a dimming, a blur, a frame) is stretched across: its width * shape;
//   - on the start screen, a group of pictures with no text (its paintings) is zoomed by shape, cut above and below;
//   - a clipping pane named N_Cut* as wide as the screen (the map's) is made as wide as the window;
//   - anything else placed 200 units or more off the middle (narrower than 900 units, or 400 or more off it) (hearts, the mini-map, the stamina wheel's neighbours,
//     the logo, the button guides) moves out by the extra half width, 640 * (shape - 1), as a whole - and so do its
//     animations' horizontal positions, in proportion (an animation that brings it to the middle still ends there);
//   - groups at the middle are looked into; what is near the middle stays.
// Parts placed at the middle over the whole screen (the weather, the camera's frame) are their own layouts: they get
// the same treatment. SAVAGEWIND_NO_WIDE_UI=1: the layouts as they are.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../rt.h"
#include "../settings.h"
#include "../window_size.h"
#include "archive.h"
#include "wide_ui.h"

#define OFF_MIDDLE 200.0f   // layout units off the middle: placed toward a side
#define WIDE_PANE 1270.0f   // as wide as the screen
#define MAX_UNIT 900.0f     // wider than this: a band across the middle, not a side's
#define MAX_NODES 4096
#define MAX_SHIFTED 256

float g_wide_ui_shape = 1.0f;

typedef struct {
    uint32_t off, size;
    int parent;
    char kind;  // n null, p part, i picture, t text, w window, b bounding, o other
} Node;

typedef struct { char name[25]; float base; } Shifted;

static float rf(const uint8_t *p) { float f; memcpy(&f, p, 4); return f; }
static void wf(uint8_t *p, float f) { memcpy(p, &f, 4); }

static int parse(const uint8_t *b, size_t n, Node *nodes) {
    if (n < 0x14 || memcmp(b, "FLYT", 4) || b[4] != 0xFF || b[5] != 0xFE) return -1;
    size_t o = (size_t)b[6] | (size_t)b[7] << 8;
    int count = 0, stack[64], depth = 0, last = -1;
    while (o + 8 <= n) {
        uint32_t sz;
        memcpy(&sz, b + o + 4, 4);
        if (sz < 8 || sz > n - o) return -1;
        const uint8_t *s = b + o;
        if (!memcmp(s, "pas1", 4)) {
            if (depth == 64) return -1;
            stack[depth++] = last;
        } else if (!memcmp(s, "pae1", 4)) {
            if (!depth) return -1;
            depth--;
        } else {
            char kind = !memcmp(s, "pan1", 4) ? 'n' : !memcmp(s, "prt1", 4) ? 'p' : !memcmp(s, "pic1", 4) ? 'i' :
                        !memcmp(s, "txt1", 4) ? 't' : !memcmp(s, "wnd1", 4) ? 'w' : !memcmp(s, "bnd1", 4) ? 'b' :
                        !memcmp(s, "scr1", 4) || !memcmp(s, "ali1", 4) ? 'o' : 0;
            if (kind) {
                if (sz < 84 || count == MAX_NODES) return -1;
                nodes[count] = (Node){(uint32_t)o, sz, depth ? stack[depth - 1] : -1, kind};
                last = count++;
            }
        }
        o += sz;
    }
    return o == n && depth == 0 ? count : -1;
}

// A pane that only groups: at the middle, unturned, unscaled
static bool neutral(const uint8_t *s, char kind) {
    if (kind != 'n' && kind != 'b') return false;
    for (int i = 0; i < 6; i++) if (fabsf(rf(s + 44 + 4 * i)) > 0.01f) return false;  // translate x, y, z, rotate x, y, z
    return fabsf(rf(s + 68) - 1) < 0.001f && fabsf(rf(s + 72) - 1) < 0.001f;
}

// The subtree of i: pictures only (no text) with one at least 700 units wide
static bool art(const uint8_t *b, const Node *nd, int count, int i) {
    bool big = false;
    for (int k = i + 1; k < count; k++) {
        int a = nd[k].parent;
        float sx = 1;
        while (a >= 0 && a != i) a = nd[a].parent;
        if (a != i) continue;
        if (nd[k].kind == 't' || nd[k].kind == 'p' || nd[k].kind == 'w') return false;
        for (int p = k; p != i; p = nd[p].parent) sx *= fabsf(rf(b + nd[p].off + 68));
        if (nd[k].kind == 'i' && rf(b + nd[k].off + 76) * sx >= 700) big = true;
    }
    return big;
}

typedef struct {
    uint8_t *b;
    const Node *nd;
    int count;
    float shape, delta;
    Shifted *shifted;
    int nshifted;
    char parts[16][32];
    int nparts;
    int changes;
    bool paintings;  // (the start screen's: its groups of pictures zoomed)
} Walk;

static void walk(Walk *w, int parent) {
    for (int c = parent + 1; c < w->count; c++) {
        if (w->nd[c].parent != parent) continue;
        uint8_t *s = w->b + w->nd[c].off;
        char kind = w->nd[c].kind;
        float tx = rf(s + 44), ty = rf(s + 48), sx = rf(s + 68), wd = rf(s + 76) * fabsf(sx);
        bool middle = fabsf(tx) < 1 && fabsf(ty) < 1;
        if ((kind == 'i' || kind == 'w') && fabsf(tx) < 1 && wd >= WIDE_PANE) {  // across the screen: stretched
            wf(s + 68, sx * w->shape);
            w->changes++;
            continue;
        }
        if (kind == 'n' && !strncmp((const char *)s + 12, "N_Cut", 5) && fabsf(tx) < 1 && wd >= WIDE_PANE) {
            wf(s + 76, rf(s + 76) * w->shape);  // a clipping pane: as wide as the window
            w->changes++;
            continue;
        }
        if (neutral(s, kind)) {
            if (w->paintings && art(w->b, w->nd, w->count, c)) {  // paintings: zoomed to fill, cut above and below
                wf(s + 68, w->shape), wf(s + 72, w->shape);
                w->changes++;
            } else {
                walk(w, c);
            }
            continue;
        }
        if (kind == 'p' && middle && fabsf(sx - 1) < 0.001f && fabsf(rf(s + 72) - 1) < 0.001f) {
            uint32_t props;  // a part over the whole screen: its own layout, the same way
            memcpy(&props, s + 0x54, 4);
            size_t at = 0x60 + (size_t)props * 0x28;
            if (props < 64 && at + 2 < w->nd[c].size && w->nparts < 16) {
                snprintf(w->parts[w->nparts], sizeof w->parts[0], "%.*s", (int)(w->nd[c].size - at), (const char *)s + at);
                if (w->parts[w->nparts][0]) w->nparts++;
            }
            continue;
        }
        if (fabsf(tx) >= OFF_MIDDLE && (wd < MAX_UNIT || fabsf(tx) >= 2 * OFF_MIDDLE) &&
            w->nshifted < MAX_SHIFTED) {  // toward a side: moved out
            wf(s + 44, tx + copysignf(w->delta, tx));
            Shifted *sh = &w->shifted[w->nshifted++];
            snprintf(sh->name, sizeof sh->name, "%.24s", (const char *)s + 12);
            sh->base = tx;
            w->changes++;
        }
    }
}

// The layout's animations (anim/<layout>_*.bflan): the moved panes' horizontal positions, moved as far as their
// distance from the middle is the layout's (the middle stays the middle; past the layout's place: the full move)
static int anims(uint8_t *arc, size_t an, const char *layout, const Shifted *sh, int nsh, float delta) {
    int changed = 0, n = sarc_count(arc, an);
    char prefix[64];
    snprintf(prefix, sizeof prefix, "anim/%s_", layout);
    size_t pl = strlen(prefix);
    for (int i = 0; i < n; i++) {
        const char *name;
        const uint8_t *cd;
        size_t sz;
        if (!sarc_node(arc, an, i, &name, &cd, &sz) || strncmp(name, prefix, pl) || !strstr(name, ".bflan")) continue;
        uint8_t *d = (uint8_t *)cd;
        if (sz < 0x14 || memcmp(d, "FLAN", 4) || d[4] != 0xFF || d[5] != 0xFE) continue;
        size_t o = (size_t)d[6] | (size_t)d[7] << 8;
        while (o + 8 <= sz) {
            uint32_t ssz;
            memcpy(&ssz, d + o + 4, 4);
            if (ssz < 8 || ssz > sz - o) break;
            if (!memcmp(d + o, "pai1", 4) && ssz >= 0x14) {
                uint16_t cnt;
                uint32_t tab;
                memcpy(&cnt, d + o + 0xE, 2), memcpy(&tab, d + o + 0x10, 4);
                for (uint32_t e = 0; e < cnt && tab + 4 * e + 4 <= ssz; e++) {
                    uint32_t eo;
                    memcpy(&eo, d + o + tab + 4 * e, 4);
                    if (eo + 32 > ssz) continue;
                    uint8_t *ent = d + o + eo;
                    const Shifted *hit = NULL;
                    for (int k = 0; k < nsh && !hit; k++)
                        if (!strncmp((const char *)ent, sh[k].name, 28)) hit = &sh[k];
                    if (!hit) continue;
                    for (uint32_t t = 0; t < ent[28]; t++) {
                        uint32_t to;
                        memcpy(&to, ent + 32 + 4 * t, 4);
                        if (eo + to + 8 > ssz || memcmp(ent + to, "FLPA", 4)) continue;
                        uint8_t *tag = ent + to;
                        for (uint32_t c = 0; c < tag[4]; c++) {
                            uint32_t co;
                            memcpy(&co, tag + 8 + 4 * c, 4);
                            uint8_t *cur = tag + co;
                            if (eo + to + co + 12 > ssz || cur[1] != 0 || cur[2] != 2) continue;  // translate x, Hermite keys
                            uint16_t nk;
                            uint32_t ko;
                            memcpy(&nk, cur + 4, 2), memcpy(&ko, cur + 8, 4);
                            if (eo + to + co + ko + 12ull * nk > ssz) continue;
                            for (uint32_t k = 0; k < nk; k++) {
                                uint8_t *v = cur + ko + 12 * k + 4;
                                float x = rf(v), f = hit->base ? x / hit->base : 0;
                                f = f < 0 ? 0 : f > 1 ? 1 : f;
                                wf(v, x + copysignf(delta, hit->base) * f);
                            }
                            changed++;
                        }
                    }
                }
            }
            o += ssz;
        }
    }
    return changed;
}

static int layout_wide(uint8_t *arc, size_t an, const char *layout, float shape, char parts[][32], int *nparts,
                       int maxparts, int *anim_curves) {
    char path[64];
    snprintf(path, sizeof path, "blyt/%s.bflyt", layout);
    uint32_t doff, s, e;
    if (sarc_find(arc, an, path, &doff, &s, &e) < 0) return -1;
    static Node nodes[MAX_NODES];
    static Shifted shifted[MAX_SHIFTED];
    uint8_t *b = arc + doff + s;
    int count = parse(b, e - s, nodes);
    if (count < 1 || nodes[0].parent != -1 || fabsf(rf(b + nodes[0].off + 76) - 1280) > 1 ||
        fabsf(rf(b + nodes[0].off + 80) - 720) > 1)
        return -1;  // (only the screen's 1280 x 720 layouts)
    Walk w = {b, nodes, count, shape, 640.0f * (shape - 1), shifted, 0, {{0}}, 0, 0, !strncmp(layout, "Title", 5)};
    walk(&w, 0);
    *anim_curves += anims(arc, an, layout, shifted, w.nshifted, w.delta);
    for (int i = 0; i < w.nparts && *nparts < maxparts; i++) {
        bool seen = false;
        for (int k = 0; k < *nparts; k++) seen |= !strcmp(parts[k], w.parts[i]);
        if (!seen) snprintf(parts[(*nparts)++], 32, "%s", w.parts[i]);
    }
    return w.changes;
}

int wide_ui_archive(uint8_t *arc, size_t an, const char *const *layouts, int n, float shape) {
    if (shape < 1.05f || getenv("SAVAGEWIND_NO_WIDE_UI")) return 0;
    enum { MAXL = 96 };
    static char todo[MAXL][32];
    int ntodo = 0, total = 0, done = 0, curves = 0;
    for (int i = 0; i < n && ntodo < MAXL; i++) snprintf(todo[ntodo++], 32, "%s", layouts[i]);
    for (int i = 0; i < ntodo; i++) {  // (the list grows with the parts they place over the screen)
        int c = layout_wide(arc, an, todo[i], shape, todo, &ntodo, MAXL, &curves);
        if (c > 0) total += c, done++;
    }
    LOG("[wide ui] %d layouts for %.2f x 16:9: %d panes moved or widened, %d animation curves\n", done, shape, total,
        curves);
    return done;
}

float wide_ui_shape_wanted(void) {
#ifdef __PROSPERO__
    return 1;  // (a TV: 16:9)
#else
    if (!g_settings.wide_view || !g_settings.wide_ui || getenv("SAVAGEWIND_NO_WIDE_UI")) return 1;
    WinSize s = winsize_effective(g_settings.fullscreen);
    if (s.w <= 0 || s.h <= 0) return 1;
    float shape = ((float)s.w / s.h) / (16.0f / 9.0f);
    return shape >= 1.05f ? fminf(shape, 2.5f) : 1;
#endif
}
