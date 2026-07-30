"""Correct pixellab.ai exports that don't match the game's 2:1 dimetric tile
ratio (grass/water/road are 2:1; pixellab's "low top-down" view often comes
out steeper, e.g. ~1.14:1 or ~1.32:1), and derive the game anchor for each
rotation from the shared pivot in pixellab's untrimmed square canvas.

Usage:
  python3 fix_iso_ratio.py --src <dir with rotations/*.png from unzipped
      pixellab export> --out <dir to write corrected trimmed PNGs>
      [--target-ratio 2.0] [--squash 0.66] [--canvas 48]

If --squash is omitted, it is measured automatically from the south-east.png
(or first available) rotation's silhouette and derived from --target-ratio.
Prints a bridge_anchors-style snippet for main.c at the end.
"""
import argparse
import glob
import os

import numpy as np
from PIL import Image

ROTATIONS = [
    "south", "south-east", "east", "north-east",
    "north", "north-west", "west", "south-west",
]


def measure_ratio(im):
    a = np.array(im)[:, :, 3]
    ys, xs = np.nonzero(a > 10)
    bot_y = ys.max()
    bot_x = (xs[ys == bot_y].min() + xs[ys == bot_y].max()) / 2
    left_x = xs.min()
    left_y = (ys[xs == left_x].min() + ys[xs == left_x].max()) / 2
    right_x = xs.max()
    right_y = (ys[xs == right_x].min() + ys[xs == right_x].max()) / 2
    dx1, dy1 = bot_x - left_x, bot_y - left_y
    dx2, dy2 = right_x - bot_x, bot_y - right_y
    return (dx1 / dy1 + dx2 / dy2) / 2


def process(path, out_dir, squash, canvas, tile_h, sprite_scale):
    im = Image.open(path).convert("RGBA")
    new_h = round(im.height * squash)
    squashed = im.resize((im.width, new_h), Image.LANCZOS)
    a = np.array(squashed)[:, :, 3]
    ys, xs = np.nonzero(a > 10)
    left, top, right, bot = xs.min(), ys.min(), xs.max(), ys.max()
    trimmed = squashed.crop((int(left), int(top), int(right) + 1, int(bot) + 1))

    name = os.path.basename(path)
    trimmed.save(os.path.join(out_dir, name))

    width, height = trimmed.size
    pivot_x_canvas = canvas / 2
    pivot_y_canvas = (canvas / 2) * squash
    trimmed_px = pivot_x_canvas - left
    trimmed_py = pivot_y_canvas - top
    anchor_x = trimmed_px / width
    anchor_y = trimmed_py / height + tile_h / (2 * sprite_scale * height)
    return name, (round(anchor_x, 4), round(anchor_y, 4))


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--src", required=True, help="dir containing rotations/*.png (or the *.png directly)")
    p.add_argument("--out", required=True)
    p.add_argument("--target-ratio", type=float, default=2.0)
    p.add_argument("--squash", type=float, default=None)
    p.add_argument("--canvas", type=float, default=48.0, help="pixellab export canvas size (square)")
    p.add_argument("--tile-h", type=float, default=32.0)
    p.add_argument("--sprite-scale", type=float, default=1.5, help="scale value used in main.c for this asset")
    args = p.parse_args()

    os.makedirs(args.out, exist_ok=True)

    candidates = glob.glob(os.path.join(args.src, "*.png")) or \
        glob.glob(os.path.join(args.src, "rotations", "*.png"))
    by_name = {os.path.basename(c): c for c in candidates}

    squash = args.squash
    if squash is None:
        probe_name = "south-east.png" if "south-east.png" in by_name else next(iter(by_name))
        ratio = measure_ratio(Image.open(by_name[probe_name]).convert("RGBA"))
        squash = ratio / args.target_ratio
        print(f"measured ratio={ratio:.4f} on {probe_name} -> squash={squash:.4f}")

    results = {}
    for rot in ROTATIONS:
        if rot + ".png" not in by_name:
            continue
        name, anchor = process(by_name[rot + ".png"], args.out, squash, args.canvas, args.tile_h, args.sprite_scale)
        results[rot] = anchor
        print(f"{name}: anchor=({anchor[0]}, {anchor[1]})")

    order = ["south-east", "south-west", "north-west", "north-east"]
    if all(r in results for r in order):
        print("\nconst Vector2 bridge_anchors[BUILDING_DIR_COUNT] = {")
        line = "  " + ", ".join("{%.4ff, %.4ff}" % results[r] for r in order) + ","
        print(line)
        print("};")


if __name__ == "__main__":
    main()
