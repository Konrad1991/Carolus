"""Reproduce the game's exact tile/object placement math (see draw_scene,
draw_ground_tile, draw_object in src/main.c) to preview an object's scale
and anchor against a real tile grid, without building or running the app.

Usage:
  python3 calibrate_placement.py --image ../house_krita2_trimmed.png \\
      --footprint-w 2 --footprint-h 4 --scale 0.5581 \\
      --anchor-x 0.6667 --anchor-y 0.70 --out /tmp/preview.png
"""
import argparse
import os

from PIL import Image

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGES_DIR = os.path.dirname(SCRIPT_DIR)
TILE_W = 64
TILE_H = 32


def iso_to_screen(x, y, offset_x, offset_y):
    return (x - y) * TILE_W / 2 + offset_x, (x + y) * TILE_H / 2 + offset_y


def paste(canvas, sprite, x, y):
    canvas.alpha_composite(sprite, (round(x), round(y)))


def draw_ground_tile(canvas, grass, sx, sy):
    scale = TILE_W / grass.width
    w, h = round(grass.width * scale), round(grass.height * scale)
    tile = grass.resize((w, h), Image.LANCZOS)
    paste(canvas, tile, sx - w / 2, sy + TILE_H - h)


def draw_diamond_outline(draw, sx, sy, color):
    top = (sx, sy)
    right = (sx + TILE_W / 2, sy + TILE_H / 2)
    bottom = (sx, sy + TILE_H)
    left = (sx - TILE_W / 2, sy + TILE_H / 2)
    draw.line([top, right, bottom, left, top], fill=color, width=2)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--image", required=True)
    p.add_argument("--footprint-w", type=int, required=True)
    p.add_argument("--footprint-h", type=int, required=True)
    p.add_argument("--scale", type=float, required=True)
    p.add_argument("--anchor-x", type=float, required=True)
    p.add_argument("--anchor-y", type=float, required=True)
    p.add_argument("--grid", type=int, default=8, help="grid_w x grid_h tiles to render")
    p.add_argument("--grass", default=os.path.join(IMAGES_DIR, "grass_krita_trimmed.png"))
    p.add_argument("--out", default=os.path.join(SCRIPT_DIR, "calibration_preview.png"))
    args = p.parse_args()

    obj = Image.open(args.image).convert("RGBA")
    grass = Image.open(args.grass).convert("RGBA")

    grid_w = grid_h = args.grid
    canvas_w = grid_w * TILE_W + 400
    canvas_h = (grid_w + grid_h) * TILE_H // 2 + 400
    offset_x = canvas_w // 2
    offset_y = 100

    canvas = Image.new("RGBA", (canvas_w, canvas_h), (30, 30, 30, 255))

    for y in range(grid_h):
        for x in range(grid_w):
            sx, sy = iso_to_screen(x, y, offset_x, offset_y)
            draw_ground_tile(canvas, grass, sx, sy)

    tx = grid_w // 2 - args.footprint_w // 2
    ty = grid_h // 2 - args.footprint_h // 2

    from PIL import ImageDraw
    draw = ImageDraw.Draw(canvas)
    for fy in range(args.footprint_h):
        for fx in range(args.footprint_w):
            sx, sy = iso_to_screen(tx + fx, ty + fy, offset_x, offset_y)
            draw_diamond_outline(draw, sx, sy, (255, 0, 0, 255))

    anchor_x, anchor_y = iso_to_screen(tx, ty, offset_x, offset_y)
    anchor_y += TILE_H
    scaled_w = round(obj.width * args.scale)
    scaled_h = round(obj.height * args.scale)
    sprite = obj.resize((scaled_w, scaled_h), Image.LANCZOS)
    pos_x = anchor_x - scaled_w * args.anchor_x
    pos_y = anchor_y - scaled_h * args.anchor_y
    paste(canvas, sprite, pos_x, pos_y)

    canvas.save(args.out)
    print(f"wrote {args.out}  (sprite {scaled_w}x{scaled_h}px, footprint origin tile ({tx},{ty}))")


if __name__ == "__main__":
    main()
