import sys

# Read input
if len(sys.argv) > 1:
    hex_data = sys.argv[1]
else:
    print("Error: Provide hexadecimal string as argument")
    exit(1)

try:
    # Convert hex string to bytes
    byte_data = bytes.fromhex(hex_data)
    # Decode bytes to ASCII
    ascii_text = byte_data.decode("ascii")
    print(ascii_text)
except ValueError as e:
    print(f"Error: Invalid hexadecimal input ({e})", file=sys.stderr)
    sys.exit(1)
except UnicodeDecodeError as e:
    print(f"Error: Invalid ASCII sequence ({e})", file=sys.stderr)
    sys.exit(1)