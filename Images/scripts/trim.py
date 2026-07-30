import sys
from PIL import Image


def trim(path):
    im = Image.open(path).convert("RGBA")
    bbox = im.getchannel("A").getbbox()
    if bbox is None:
        print(f"skip (fully transparent) -> {path}")
        return
    out_path = path.replace("_raw.png", "_trimmed.png")
    im.crop(bbox).save(out_path)
    print(f"{path} {im.size} -> {out_path} {im.crop(bbox).size}")


if __name__ == "__main__":
    for path in sys.argv[1:]:
        trim(path)
