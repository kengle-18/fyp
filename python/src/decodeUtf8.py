import sys

# Read entire input
if len(sys.argv) > 1:
    data = sys.argv[1]
else:
    print("Error: Provide utf8 string as argument")
    exit(1)
try:
    # Decode backslash escape sequences (like \344)
    decoded = data.encode().decode("unicode_escape")
    # Convert resulting bytes to proper UTF-8
    final = decoded.encode("latin1").decode("utf-8", errors="strict")
    print(final)
except UnicodeDecodeError as e:
    print(f"Error: Invalid UTF-8 sequence ({e})", file=sys.stderr)
    sys.exit(1)