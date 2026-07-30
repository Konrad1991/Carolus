import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "scripts"))
import slice_sheet
import trim

HERE = Path(__file__).parent
sheet = HERE / "earth.png"
out_dir = HERE / "."

sys.argv = [
    "slice_sheet",
    "--image", str(sheet),
    "--rows", "4",
    "--cols", "4",
    "--out", str(out_dir),
]
slice_sheet.main()

for f in sorted(out_dir.glob("*.png")):
    trim.trim(str(f))
