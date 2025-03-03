import os
import shutil

# Define root src and include paths
SRC_DIR = "src"   # Adjust if needed
INCLUDE_DIR = os.path.join(SRC_DIR, "include")

# Ensure the include directory exists
os.makedirs(INCLUDE_DIR, exist_ok=True)

# Walk through src and find .h files
for root, _, files in os.walk(SRC_DIR):
    if "include" in root:  # Skip existing include folders
        continue

    for file in files:
        if file.endswith(".h"):
            full_path = os.path.join(root, file)

            # Get module name (libk, drivers, kernel, etc.)
            module = os.path.basename(root)

            # Target directory in include/
            target_dir = os.path.join(INCLUDE_DIR, module)
            os.makedirs(target_dir, exist_ok=True)

            # Move the file
            target_path = os.path.join(target_dir, file)
            shutil.move(full_path, target_path)
            print(f"Moved: {full_path} → {target_path}")

print("\n✅ All header files have been moved to 'src/include/'.")
