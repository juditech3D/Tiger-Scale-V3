#!/usr/bin/env python3
"""make-presse-icons.py — draws the heat press memo's product thumbnails.

Writes TigerTagSplashESP32/presse_icons.h: one 8-bit alpha mask per object
(mug, T-shirt, hoodie...), as LVGL LV_IMG_CF_ALPHA_8BIT images. An alpha mask
takes its colour from the image's recolor style, so the screen tints each
thumbnail with its category's colour and one bitmap serves every category.

The shapes are drawn here, in code, from primitives: there is no source image
to lose, and running this again reproduces the header byte for byte. Each
shape is drawn at SCALE times its size and shrunk, which is what gives the
edges their anti-aliasing.

Usage:  python3 scripts/make-presse-icons.py
Needs:  Pillow
"""
import pathlib
from PIL import Image, ImageDraw

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "TigerTagSplashESP32" / "presse_icons.h"

SIZE = 44                 # pixels on the panel
SCALE = 8                 # drawn this many times larger, then shrunk
G = SIZE * SCALE          # the drawing grid: coordinates below are 0..100 of it


def P(*xy):
    """0..100 coordinates to drawing-grid pixels."""
    return [v * G / 100 for v in xy]


def canvas():
    img = Image.new("L", (G, G), 0)
    return img, ImageDraw.Draw(img)


ON, OFF = 255, 0


def tshirt(d, long_sleeves=False):
    if long_sleeves:
        body = [(30, 12), (42, 12), (50, 20), (58, 12), (70, 12), (92, 30), (96, 78),
                (82, 80), (78, 44), (76, 44), (76, 92), (24, 92), (24, 44), (22, 44),
                (18, 80), (4, 78), (8, 30)]
    else:
        body = [(30, 12), (42, 12), (50, 20), (58, 12), (70, 12), (96, 30), (86, 46),
                (76, 40), (76, 92), (24, 92), (24, 40), (14, 46), (4, 30)]
    d.polygon([tuple(P(x, y)) for x, y in body], fill=ON)


def draw_tshirt(d):
    tshirt(d)


def draw_hoodie(d):
    tshirt(d, long_sleeves=True)
    # the hood behind the neck, and the face opening as a dark V
    d.polygon([tuple(P(x, y)) for x, y in [(28, 14), (36, 2), (64, 2), (72, 14), (50, 30)]], fill=ON)
    d.polygon([tuple(P(x, y)) for x, y in [(40, 10), (60, 10), (50, 26)]], fill=OFF)
    # drawstrings
    d.line(P(45, 26, 45, 40), fill=OFF, width=int(G * 0.025))
    d.line(P(55, 26, 55, 40), fill=OFF, width=int(G * 0.025))
    # the front pocket
    d.rounded_rectangle(P(36, 64, 64, 78), radius=G * 0.03, outline=OFF, width=int(G * 0.035))


def draw_mug(d):
    d.ellipse(P(56, 30, 92, 72), outline=ON, width=int(G * 0.09))
    d.rounded_rectangle(P(10, 18, 68, 88), radius=G * 0.08, fill=ON)


def draw_cap(d):
    # side view: the crown on the left, the visor reaching out to the right
    d.pieslice(P(6, 22, 70, 86), 180, 360, fill=ON)
    d.polygon([tuple(P(x, y)) for x, y in [(56, 50), (94, 58), (96, 64), (60, 62)]], fill=ON)
    d.rectangle(P(6, 53, 62, 62), fill=ON)
    d.line(P(38, 24, 38, 54), fill=OFF, width=int(G * 0.025))
    d.ellipse(P(34, 16, 42, 24), fill=ON)


def draw_bag(d):
    d.arc(P(30, 4, 70, 52), 180, 360, fill=ON, width=int(G * 0.07))
    d.rectangle(P(30, 27, 37, 36), fill=ON)
    d.rectangle(P(63, 27, 70, 36), fill=ON)
    d.polygon([tuple(P(x, y)) for x, y in [(14, 34), (86, 34), (92, 94), (8, 94)]], fill=ON)


def draw_mousepad(d):
    d.rounded_rectangle(P(4, 18, 96, 82), radius=G * 0.1, fill=ON)
    d.ellipse(P(56, 30, 80, 66), fill=OFF)
    d.line(P(68, 30, 68, 44), fill=ON, width=int(G * 0.03))


def draw_cushion(d):
    # a puffed square: rounded corners, each side pulled in a little
    d.rounded_rectangle(P(6, 6, 94, 94), radius=G * 0.16, fill=ON)
    for box in [(-60, 2, 160, 12), (-60, 88, 160, 98), (2, -60, 12, 160), (88, -60, 98, 160)]:
        x0, y0, x1, y1 = box
        if x0 < 0:
            d.ellipse(P(10, y0 - 6, 90, y1 - 4) if y0 < 50 else P(10, y0 + 4, 90, y1 + 6), fill=OFF)
        else:
            d.ellipse(P(x0 - 6, 10, x1 - 4, 90) if x0 < 50 else P(x0 + 4, 10, x1 + 6, 90), fill=OFF)
    d.line(P(30, 50, 70, 50), fill=OFF, width=int(G * 0.03))


def draw_plate(d):
    d.rounded_rectangle(P(8, 14, 92, 86), radius=G * 0.05, fill=ON)
    d.rounded_rectangle(P(18, 24, 82, 76), radius=G * 0.03, fill=OFF)
    d.polygon([tuple(P(x, y)) for x, y in [(22, 72), (42, 46), (56, 62), (64, 54), (78, 72)]], fill=ON)
    d.ellipse(P(62, 30, 72, 40), fill=ON)


def draw_bottle(d):
    d.rounded_rectangle(P(40, 4, 60, 16), radius=G * 0.02, fill=ON)
    d.rectangle(P(43, 16, 57, 26), fill=ON)
    d.rounded_rectangle(P(26, 24, 74, 96), radius=G * 0.12, fill=ON)
    d.rectangle(P(30, 48, 70, 70), fill=OFF)


def draw_phone(d):
    d.rounded_rectangle(P(24, 4, 76, 96), radius=G * 0.1, fill=ON)
    d.rounded_rectangle(P(30, 10, 46, 30), radius=G * 0.04, fill=OFF)
    d.ellipse(P(34, 14, 42, 22), fill=ON)


# The order is the order of the editor's list. The id is what /presse.json
# stores in an item's "icon"; never rename one, lists in the field use it.
ICONS = [
    ("tshirt", draw_tshirt),
    ("hoodie", draw_hoodie),
    ("mug", draw_mug),
    ("cap", draw_cap),
    ("bag", draw_bag),
    ("mousepad", draw_mousepad),
    ("cushion", draw_cushion),
    ("plate", draw_plate),
    ("bottle", draw_bottle),
    ("phone", draw_phone),
]


def render(fn):
    img, d = canvas()
    fn(d)
    return img.resize((SIZE, SIZE), Image.LANCZOS)


def main():
    out = []
    out.append("// presse_icons.h -- GENERATED by scripts/make-presse-icons.py. Do not edit;")
    out.append("// change the shapes there and run it again.")
    out.append("//")
    out.append(f"// {len(ICONS)} product thumbnails for the heat press memo, {SIZE}x{SIZE} 8-bit")
    out.append("// alpha masks (LV_IMG_CF_ALPHA_8BIT): the image's recolor style gives the colour.")
    out.append("#pragma once")
    out.append("#include <lvgl.h>")
    out.append("")
    out.append(f"#define PRESSE_ICON_SIZE {SIZE}")
    out.append("")
    for name, fn in ICONS:
        data = render(fn).tobytes()
        out.append(f"static const uint8_t presse_icon_{name}_map[{len(data)}] = {{")
        for i in range(0, len(data), 22):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 22]) + ",")
        out.append("};")
        out.append(f"static const lv_img_dsc_t presse_icon_{name} = {{")
        out.append(f"    {{ LV_IMG_CF_ALPHA_8BIT, 0, 0, {SIZE}, {SIZE} }},")
        out.append(f"    {len(data)}, presse_icon_{name}_map }};")
        out.append("")
    out.append("struct PresseIcon { const char *id; const lv_img_dsc_t *img; };")
    out.append("static const PresseIcon PRESSE_ICONS[] = {")
    for name, _ in ICONS:
        out.append(f'    {{ "{name}", &presse_icon_{name} }},')
    out.append("};")
    out.append(f"static const int PRESSE_ICON_COUNT = {len(ICONS)};")
    OUT.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")

    # A contact sheet to look at, next to nothing that gets committed.
    sheet = Image.new("L", (SIZE * len(ICONS) + 4 * (len(ICONS) + 1), SIZE + 8), 40)
    for i, (_, fn) in enumerate(ICONS):
        sheet.paste(render(fn), (4 + i * (SIZE + 4), 4))
    print(f"wrote {OUT.relative_to(ROOT)} ({len(ICONS)} icons, {SIZE}px)")
    return sheet


if __name__ == "__main__":
    import sys
    sheet = main()
    if len(sys.argv) > 1:
        sheet.resize((sheet.width * 3, sheet.height * 3), Image.NEAREST).save(sys.argv[1])
