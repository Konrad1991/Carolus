"""Measure the trimmed pixel size of each figure action's south-facing
sprite, and print a scale-correction suggestion relative to a baseline
action (matches FIGURE_ACTION_SCALE_CORRECTION in src/textures/textures.c).

Usage:
  python3 measure_action_sizes.py ../farmer1 --baseline standing
"""
import argparse
import os

from PIL import Image

ACTIONS = ["standing", "walking", "sowing", "chopping", "digging", "hammering", "mowing"]


def action_south_path(base_dir, action):
    rotation_path = os.path.join(base_dir, action, "rotations", "south.png")
    if os.path.exists(rotation_path):
        return rotation_path
    return os.path.join(base_dir, action, "animations", action, "south", "frame_000.png")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("base_dir", help="e.g. ../farmer1")
    p.add_argument("--baseline", default="standing", choices=ACTIONS)
    args = p.parse_args()

    sizes = {}
    for action in ACTIONS:
        path = action_south_path(args.base_dir, action)
        if not os.path.exists(path):
            print(f"{action:10s} MISSING ({path})")
            continue
        im = Image.open(path).convert("RGBA")
        w, h = im.size
        sizes[action] = (w, h)

    if args.baseline not in sizes:
        print(f"baseline action '{args.baseline}' not found, skipping correction suggestions")
        return

    ref_h = sizes[args.baseline][1]
    print(f"\n{'action':10s} {'size':>8s} {'suggested correction (ref_h/h)':>30s}")
    for action, (w, h) in sizes.items():
        correction = ref_h / h
        print(f"{action:10s} {w:3d}x{h:<4d} {correction:>30.3f}")


if __name__ == "__main__":
    main()
