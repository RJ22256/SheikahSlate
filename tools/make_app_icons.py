#!/usr/bin/env python3
"""Sheikah Slate's other app icons (Settings > Look > App icon; android/.../AndroidManifest.xml activity-alias): original
designs - no Nintendo artwork, emblems or symbols - in the same shape as the original (tools/make_icon.py): a rounded
square, drawn 4x and scaled down.

  midnight  a night-blue slate: a cyan ring of arcs around a crescent moon, stars
  dawn      mountains in layers under a pink and amber sunrise
  ancient   a weathered stone tablet with an amber meander carved in it, glowing
  forest    pines in layers under a full moon, fireflies

Usage: make_app_icons.py <out dir> [size]   (writes app-icon-<name>.png)
"""
import math
import os
import random
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

OUT = sys.argv[1]
SIZE = int(sys.argv[2]) if len(sys.argv) > 2 else 512
S = SIZE * 4


def vgrad(stops):
    """A vertical gradient through (position 0..1, rgb) stops, as an RGBA image S x S."""
    y = np.linspace(0, 1, S)[:, None]
    out = np.zeros((S, 1, 3))
    for (p0, c0), (p1, c1) in zip(stops, stops[1:]):
        t = np.clip((y - p0) / max(p1 - p0, 1e-6), 0, 1)
        inside = (y >= p0) & (y <= p1)
        out = np.where(inside[..., None], np.array(c0) + (np.array(c1) - np.array(c0)) * t[..., None], out)
    arr = np.broadcast_to(out, (S, S, 3)).astype(np.uint8)
    a = np.full((S, S, 1), 255, np.uint8)
    return Image.fromarray(np.concatenate([arr, a], axis=2), "RGBA")


def rounded(img):
    mask = Image.new("L", (S, S), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, S - 1, S - 1], radius=int(S * 0.22), fill=255)
    out = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    out.paste(img, (0, 0), mask)
    return out


def glow(img, shape_fn, color, blur):
    g = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    shape_fn(ImageDraw.Draw(g), color)
    g = g.filter(ImageFilter.GaussianBlur(blur))
    return Image.alpha_composite(img, g)


def ridge(rng, y_mid, amp, n, x0=0.0, x1=1.0):
    """A mountain range's outline: points across the width, peaks and saddles."""
    pts = []
    for i in range(n + 1):
        x = x0 + (x1 - x0) * i / n
        y = y_mid - amp * (0.55 + 0.45 * math.sin(i * 1.9 + rng.random())) * (1 if i % 2 else 0.45)
        pts.append((x * S, y * S))
    return pts + [(S, S), (0, S)]


def midnight():
    img = vgrad([(0, (8, 12, 34)), (1, (14, 34, 70))])
    rng = random.Random(7)
    d = ImageDraw.Draw(img)
    for _ in range(70):  # stars
        x, y, r = rng.random() * S, rng.random() * S, S * (0.002 + rng.random() * 0.004)
        d.ellipse([x - r, y - r, x + r, y + r], fill=(220, 235, 255, 255))
    cx, cy = S * 0.5, S * 0.5
    cyan = (90, 230, 250, 255)

    def ring(dd, col):
        for k, (r, a0, a1) in enumerate([(0.34, 200, 340), (0.34, 20, 160), (0.27, 250, 470), (0.41, 300, 400),
                                         (0.41, 120, 220)]):
            rr = S * r
            dd.arc([cx - rr, cy - rr, cx + rr, cy + rr], a0, a1, fill=col, width=int(S * (0.016 if k < 2 else 0.01)))
        for ang in range(0, 360, 30):  # ticks on the outer ring
            a = math.radians(ang)
            r0, r1 = S * 0.36, S * 0.385
            dd.line([(cx + r0 * math.cos(a), cy + r0 * math.sin(a)), (cx + r1 * math.cos(a), cy + r1 * math.sin(a))],
                    fill=col, width=int(S * 0.008))

    img = glow(img, ring, (60, 210, 255, 200), S * 0.02)
    ring(ImageDraw.Draw(img), cyan)
    # the crescent: a disc, a night-coloured disc over it
    d = ImageDraw.Draw(img)
    r = S * 0.15
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(250, 236, 190, 255))
    o = S * 0.07
    d.ellipse([cx - r + o, cy - r - o * 0.5, cx + r + o, cy + r - o * 0.5], fill=(11, 22, 50, 255))
    return rounded(img)


def dawn():
    img = vgrad([(0, (60, 40, 110)), (0.35, (226, 104, 120)), (0.62, (255, 178, 96)), (1, (255, 214, 150))])
    d = ImageDraw.Draw(img)
    cx, cy, r = S * 0.62, S * 0.56, S * 0.16
    img = glow(img, lambda dd, c: dd.ellipse([cx - r * 1.6, cy - r * 1.6, cx + r * 1.6, cy + r * 1.6], fill=c),
               (255, 240, 200, 150), S * 0.05)
    d = ImageDraw.Draw(img)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(255, 238, 190, 255))
    rng = random.Random(3)
    for y_mid, amp, col in [(0.62, 0.16, (168, 86, 120)), (0.74, 0.15, (112, 56, 104)), (0.86, 0.12, (60, 32, 78))]:
        d.polygon(ridge(rng, y_mid, amp, 7), fill=col + (255,))
    for bx, by in [(0.24, 0.30), (0.31, 0.26), (0.36, 0.32)]:  # birds
        x, y, w = bx * S, by * S, S * 0.035
        d.line([(x - w, y - w * 0.4), (x, y), (x + w, y - w * 0.4)], fill=(70, 40, 80, 255), width=int(S * 0.008), joint="curve")
    return rounded(img)


def ancient():
    base = vgrad([(0, (104, 96, 86)), (1, (62, 56, 52))])
    noise = (np.random.default_rng(5).random((S // 8, S // 8)) * 255).astype(np.uint8)
    n = Image.fromarray(noise, "L").resize((S, S), Image.BICUBIC).filter(ImageFilter.GaussianBlur(S * 0.004))
    shade = Image.merge("RGBA", [n, n, n, Image.new("L", (S, S), 46)])
    img = Image.alpha_composite(base, shade)
    d = ImageDraw.Draw(img)
    # cracks
    rng = random.Random(11)
    for _ in range(6):  # (from near the edges: none across the carving)
        x, y = rng.random() * S, rng.random() * S
        if abs(x - S / 2) < S * 0.3 and abs(y - S / 2) < S * 0.3:
            x = S * 0.1 if x < S / 2 else S * 0.9
        pts = [(x, y)]
        for _ in range(5):
            x += (rng.random() - 0.5) * S * 0.08
            y += (rng.random() - 0.3) * S * 0.06
            pts.append((x, y))
        d.line(pts, fill=(40, 36, 34, 255), width=int(S * 0.004))
    # a square meander (a key pattern), centred
    u = S * 0.07
    x0, y0 = S * 0.5 - 3 * u, S * 0.5 - 3 * u
    path = [(0, 6), (0, 0), (6, 0), (6, 6), (2, 6), (2, 2), (4, 2), (4, 4)]
    pts = [(x0 + px * u, y0 + py * u) for px, py in path]
    amber = (255, 168, 60, 255)
    img = glow(img, lambda dd, c: dd.line(pts, fill=c, width=int(S * 0.05), joint="curve"), (255, 140, 40, 210), S * 0.03)
    d = ImageDraw.Draw(img)
    d.line(pts, fill=(52, 40, 30, 255), width=int(S * 0.05), joint="curve")  # the carving
    d.line(pts, fill=amber, width=int(S * 0.022), joint="curve")             # its glow
    for px, py in [(0, 6), (4, 4)]:
        x, y, r = x0 + px * u, y0 + py * u, S * 0.03
        d.ellipse([x - r, y - r, x + r, y + r], fill=(255, 214, 140, 255))
    return rounded(img)


def forest():
    img = vgrad([(0, (10, 24, 34)), (0.6, (22, 56, 60)), (1, (12, 30, 30))])
    cx, cy, r = S * 0.66, S * 0.30, S * 0.12
    img = glow(img, lambda dd, c: dd.ellipse([cx - r * 1.8, cy - r * 1.8, cx + r * 1.8, cy + r * 1.8], fill=c),
               (200, 240, 220, 110), S * 0.06)
    d = ImageDraw.Draw(img)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(236, 246, 226, 255))
    rng = random.Random(9)
    for row, (base_y, h, col) in enumerate([(0.70, 0.30, (30, 72, 64)), (0.82, 0.34, (18, 50, 44)), (0.97, 0.40, (8, 28, 26))]):
        x = -0.05 + rng.random() * 0.08
        while x < 1.05:
            w, hh = 0.07 + rng.random() * 0.05, h * (0.7 + rng.random() * 0.4)
            for k in range(3):  # three tiers a pine
                t0 = base_y - hh * (k / 3.2)
                d.polygon([((x - w * (1 - k * 0.22)) * S, t0 * S), ((x + w * (1 - k * 0.22)) * S, t0 * S),
                           (x * S, (t0 - hh * 0.45) * S)], fill=col + (255,))
            d.rectangle([(x - 0.006) * S, base_y * S, (x + 0.006) * S, (base_y + 0.05) * S], fill=col + (255,))
            x += w * (1.1 + rng.random() * 0.5)
        d.rectangle([0, base_y * S, S, S], fill=col + (255,))
    for _ in range(14):  # fireflies
        x, y = rng.random() * S, S * (0.6 + rng.random() * 0.3)
        img = glow(img, lambda dd, c, x=x, y=y: dd.ellipse([x - S * 0.012, y - S * 0.012, x + S * 0.012, y + S * 0.012], fill=c),
                   (220, 255, 140, 220), S * 0.006)
    return rounded(img)


os.makedirs(OUT, exist_ok=True)
for name, fn in [("midnight", midnight), ("dawn", dawn), ("ancient", ancient), ("forest", forest)]:
    im = fn().resize((SIZE, SIZE), Image.LANCZOS)
    p = os.path.join(OUT, f"app-icon-{name}.png")
    im.save(p, optimize=True)
    print(p, im.size, os.path.getsize(p) // 1024, "KB")
