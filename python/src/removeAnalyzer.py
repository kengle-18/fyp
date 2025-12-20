from pathlib import Path
import shutil
import os 

analyzer_dir = Path(os.getenv("ANALYZER_DIR", "/app/src/analyzer"))

# Iterate over all items in the directory
for item in analyzer_dir.iterdir():
    if item.is_file() or item.is_symlink():
        item.unlink()  # remove file or symlink
    elif item.is_dir():
        shutil.rmtree(item)  # remove directory and all its contents
