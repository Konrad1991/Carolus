import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "scripts"))
import trim

HERE = Path(__file__).parent

files = sorted(HERE.glob("rotations/*.png"))

for f in files:
    trim.trim(str(f))
