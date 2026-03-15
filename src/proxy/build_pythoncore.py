import sys
import zipfile
from pathlib import Path

_EXCLUDE_MODULES = ["ensurepip", "idlelib", "test", "tkinter", "turtle_demo", "venv"]
if sys.platform == "win32":
    libdir = Path(sys.executable).parent.joinpath("Lib")
else:
    libdir = Path("/lib/python%d.%d" % (sys.version_info.major, sys.version_info.minor))
assert libdir.exists()

with zipfile.PyZipFile(sys.argv[1], optimize=2, mode="w") as fp:
    fp.writepy(libdir)  # pyright: ignore[reportArgumentType]
    for path in libdir.iterdir():
        if path.is_dir() and path.name not in _EXCLUDE_MODULES:
            fp.writepy(path)  # pyright: ignore[reportArgumentType]
