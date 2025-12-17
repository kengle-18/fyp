import os
import sys

config_dir = os.getenv("CONFIG_DIR", "/app/src/test")
default_config_file = os.getenv("DEFAULT_CONFIG_FILE", "config.txt")
default_config_file_full_path = os.path.join(config_dir, default_config_file)
file_to_use = default_config_file_full_path

# List files
files = os.listdir(config_dir)
# print("Files in test directory:", files)

all_files = [f for f in os.listdir(config_dir) if os.path.isfile(os.path.join(config_dir, f))]

# Remove config.txt from the list
other_files = [f for f in all_files if f != "config.txt"]

for f in other_files:
    custom_path = os.path.join(config_dir, f)
    # Check if file is not empty
    if os.path.getsize(custom_path) > 0:
        file_to_use = custom_path
        break  # use the first non-empty custom file

# print(f"Using file: {file_to_use}")

def process(line):
    # Example processing function
    print(f"Processing line: {line}")
    # parse all commands flag with values
    tokens = line.strip().split()
    if (len(tokens) % 2) != 0:
        raise ValueError("Uneven number inputs, flags must have values")
    
    i = 0
    while i < len(tokens):
        token = tokens[i]
        if not token.startswith("--"):
            raise ValueError(f"Expected flag, got '{token}'")
        if i + 1 >= len(tokens):
            raise ValueError(f"Flag '{token}' has no value")
        
        value = tokens[i+1]
        if "," in value and "--m" in token:
            first, _ = value.split(",", 1)
            if not first.strip():
                raise ValueError(
                    f"First argument before comma missing which is the key for map that cant be empty"
                )
        i += 2
    return


line_count = 0
# Read content
try:
    with open(file_to_use, "r") as f:
        for line_number, line in enumerate(f, start=1):
            line = line.rstrip("\n")  # remove newline only
            process(line)
except ValueError as e:
    print(f"Healthcheck failed: {e}")
    sys.exit(1)


print(f"Number of lines: {line_number}")
sys.exit(0)
