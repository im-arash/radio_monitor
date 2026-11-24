
from pathlib import Path

OUTPUT_FILE = "fullcode.txt"
EXTENSIONS = {".h", ".cpp", ".qml"}
SEPARATOR = "<----->\n<----->\n"

current_dir = Path.cwd()

files = sorted(
    f for f in current_dir.iterdir()
    if f.is_file() and f.suffix.lower() in EXTENSIONS
)

with open(current_dir / OUTPUT_FILE, "w", encoding="utf-8") as output:
    for i, file in enumerate(files):
        if i > 0:
            output.write("\n" + SEPARATOR + "\n")

        output.write(f"// FILE: {file.name}\n\n")

        try:
            output.write(file.read_text(encoding="utf-8"))
        except UnicodeDecodeError:
            output.write(file.read_text(encoding="utf-8", errors="replace"))

        output.write("\n")

print(f"Done! Combined {len(files)} files into {OUTPUT_FILE}")
