import sys

from PIL import Image

for path in sys.argv[1:]:
    w, h = Image.open(path).size
    print(f"{path}: {w}x{h}")
