"""Slice a tile-sheet image (e.g. bridge.png, a grid of NxM tiles) into
individual PNGs.

Usage:
  python3 slice_sheet.py --image ../../bridge.png --rows 4 --cols 4 --out /tmp/sliced
  python3 slice_sheet.py --image ../../bridge.png --count 16 --out /tmp/sliced

--count picks the smallest square grid that fits (e.g. 16 -> 4x4, 8 -> 3x3
with one empty cell skipped). Cell boundaries are computed with linspace so
sheets whose size isn't evenly divisible by rows/cols are still sliced
without gaps or overlaps. Fully transparent cells are skipped.
"""
import argparse
import math
import os

import numpy as np
from PIL import Image


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--image", required=True)
    p.add_argument("--rows", type=int)
    p.add_argument("--cols", type=int)
    p.add_argument("--count", type=int, help="total tiles; picks a square-ish grid if --rows/--cols omitted")
    p.add_argument("--out", required=True)
    p.add_argument("--prefix", default="tile")
    p.add_argument("--trim", action="store_true", help="crop each cell to its non-transparent bbox")
    args = p.parse_args()

    im = Image.open(args.image).convert("RGBA")
    W, H = im.size

    rows, cols = args.rows, args.cols
    if rows is None or cols is None:
        if not args.count:
            raise SystemExit("need --rows/--cols or --count")
        cols = math.ceil(math.sqrt(args.count))
        rows = math.ceil(args.count / cols)

    os.makedirs(args.out, exist_ok=True)

    xs = [round(i * W / cols) for i in range(cols + 1)]
    ys = [round(i * H / rows) for i in range(rows + 1)]

    n = 0
    for r in range(rows):
        for c in range(cols):
            cell = im.crop((xs[c], ys[r], xs[c + 1], ys[r + 1]))
            if np.array(cell)[:, :, 3].max() < 10:
                continue
            if args.trim:
                bbox = cell.getchannel("A").getbbox()
                if bbox:
                    cell = cell.crop(bbox)
            out_path = os.path.join(args.out, f"{args.prefix}_{n:02d}.png")
            cell.save(out_path)
            print(out_path, cell.size)
            n += 1

    print(f"wrote {n} tiles to {args.out}")


if __name__ == "__main__":
    main()
