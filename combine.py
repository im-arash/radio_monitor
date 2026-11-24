from pathlib import Path

OUTPUT_FILE = "fullcode.txt"
EXTENSIONS = {".h", ".cpp", ".qml"}
SOURCE_DIRS = ["src", "qml"]                   # searched recursively (subfolders included)
EXTRA_FILES = ["CMakeLists.txt", "main.cpp"]   # root files, written first, in this order
SEPARATOR = "<----->\n<----->\n"

root = Path(__file__).resolve().parent

# root files first (in the order listed above)
files = []
for name in EXTRA_FILES:
    path = root / name
    if path.is_file():
        files.append(path)
    else:
        print(f"Warning: '{name}' not found, skipping")

# then everything under src/ and qml/, sorted by path
found = []
for dir_name in SOURCE_DIRS:
    folder = root / dir_name
    if not folder.is_dir():
        print(f"Warning: '{dir_name}' not found, skipping")
        continue
    found.extend(
        f for f in folder.rglob("*")
        if f.is_file() and f.suffix.lower() in EXTENSIONS
    )
files.extend(sorted(set(found) - set(files)))

with open(root / OUTPUT_FILE, "w", encoding="utf-8") as output:
    for i, file in enumerate(files):
        if i > 0:
            output.write("\n" + SEPARATOR + "\n")

        # relative path, so same-named files in different folders stay distinguishable
        output.write(f"// FILE: {file.relative_to(root).as_posix()}\n\n")

        try:
            output.write(file.read_text(encoding="utf-8"))
        except UnicodeDecodeError:
            output.write(file.read_text(encoding="utf-8", errors="replace"))

        output.write("\n")

print(f"Done! Combined {len(files)} files into {OUTPUT_FILE}")
