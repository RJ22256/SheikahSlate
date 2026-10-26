#!/usr/bin/env python3
"""Sheikah Slate's launcher icon: an original design (no Nintendo artwork or emblems) - a slate tablet in landscape,
glowing cyan edges, on its screen a mountain range under a ringed amber sun. Drawn 4x and scaled down.

Usage: make_icon.py <out.png> [size]
"""
import math
import sys

from PIL import Image, ImageDraw, ImageFilter

OUT = sys.argv[1]
SIZE = int(sys.argv[2]) if len(sys.argv) > 2 else 1024
S = SIZE * 4


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))


img = Image.new("RGBA", (S, S), (0, 0, 0, 0))

# background: a rounded square, night blue to deep teal (top to bottom)
bg = Image.new("RGBA", (S, S))
px = bg.load()
top, bottom = (10, 18, 40), (8, 52, 62)
for y in range(S):
    c = lerp(top, bottom, y / (S - 1))
    for x in range(S):
        px[x, y] = c + (255,)
mask = Image.new("L", (S, S), 0)
ImageDraw.Draw(mask).rounded_rectangle([0, 0, S - 1, S - 1], radius=int(S * 0.22), fill=255)
img.paste(bg, (0, 0), mask)

# a soft glow behind the slate
glow = Image.new("RGBA", (S, S), (0, 0, 0, 0))
gd = ImageDraw.Draw(glow)
gd.ellipse([S * 0.12, S * 0.22, S * 0.88, S * 0.84], fill=(40, 200, 230, 70))
glow = glow.filter(ImageFilter.GaussianBlur(S * 0.06))
img = Image.alpha_composite(img, glow)

d = ImageDraw.Draw(img)
# the slate: landscape, rounded, dark stone with a cyan edge
L, T, R, B = S * 0.14, S * 0.27, S * 0.86, S * 0.73
rad = S * 0.07
d.rounded_rectangle([L, T, R, B], radius=rad, fill=(24, 30, 38, 255), outline=(70, 225, 245, 255), width=int(S * 0.012))
# its screen
sl, st, sr, sb = L + S * 0.045, T + S * 0.045, R - S * 0.045, B - S * 0.045
screen = Image.new("RGBA", (S, S), (0, 0, 0, 0))
sd = ImageDraw.Draw(screen)
spx = screen.load()
sky_top, sky_bot = (18, 40, 78), (236, 150, 70)
for y in range(int(st), int(sb) + 2):
    c = lerp(sky_top, sky_bot, (y - st) / (sb - st))
    for x in range(int(sl), int(sr) + 2):
        spx[x, y] = c + (255,)
# the sun: an amber disc with a thin ring
cx, cy, r = (sl + sr) * 0.5 + S * 0.10, st + (sb - st) * 0.40, S * 0.065
sd.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(255, 205, 96, 255))
rr = r * 1.55
sd.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], outline=(255, 226, 150, 220), width=int(S * 0.008))
# mountains: two ranges, the near one darker
far = [(sl, sb - (sb - st) * 0.30), (sl + (sr - sl) * 0.18, sb - (sb - st) * 0.52), (sl + (sr - sl) * 0.33, sb - (sb - st) * 0.36),
       (sl + (sr - sl) * 0.52, sb - (sb - st) * 0.62), (sl + (sr - sl) * 0.70, sb - (sb - st) * 0.40),
       (sl + (sr - sl) * 0.86, sb - (sb - st) * 0.50), (sr, sb - (sb - st) * 0.34), (sr, sb + 6), (sl, sb + 6)]
sd.polygon(far, fill=(46, 72, 110, 255))
near = [(sl, sb - (sb - st) * 0.16), (sl + (sr - sl) * 0.22, sb - (sb - st) * 0.34), (sl + (sr - sl) * 0.40, sb - (sb - st) * 0.18),
        (sl + (sr - sl) * 0.62, sb - (sb - st) * 0.38), (sl + (sr - sl) * 0.80, sb - (sb - st) * 0.20), (sr, sb - (sb - st) * 0.28),
        (sr, sb + 6), (sl, sb + 6)]
sd.polygon(near, fill=(20, 36, 58, 255))
smask = Image.new("L", (S, S), 0)
ImageDraw.Draw(smask).rounded_rectangle([sl, st, sr, sb], radius=rad * 0.55, fill=255)
img.paste(screen, (0, 0), smask)

# cyan circuit lines on the frame: short runs with dots, an original pattern
d = ImageDraw.Draw(img)
w = int(S * 0.006)
cyan = (110, 235, 250, 255)
mid = (L + R) * 0.5
for sx in (-1, 1):
    x0 = mid + sx * S * 0.05
    x1 = mid + sx * S * 0.22
    d.line([(x0, T + S * 0.022), (x1, T + S * 0.022)], fill=cyan, width=w)
    d.ellipse([x1 - w * 2, T + S * 0.022 - w * 2, x1 + w * 2, T + S * 0.022 + w * 2], fill=cyan)
    d.line([(x0, B - S * 0.022), (x1, B - S * 0.022)], fill=cyan, width=w)
    d.ellipse([x1 - w * 2, B - S * 0.022 - w * 2, x1 + w * 2, B - S * 0.022 + w * 2], fill=cyan)
for sy in (-1, 1):
    y0 = (T + B) * 0.5 + sy * S * 0.03
    y1 = (T + B) * 0.5 + sy * S * 0.12
    for x in (L + S * 0.022, R - S * 0.022):
        d.line([(x, y0), (x, y1)], fill=cyan, width=w)
        d.ellipse([x - w * 2, y1 - w * 2, x + w * 2, y1 + w * 2], fill=cyan)

img = img.resize((SIZE, SIZE), Image.LANCZOS)
img.save(OUT, optimize=True)
print(OUT, img.size)
