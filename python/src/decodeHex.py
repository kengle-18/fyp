import sys

# Try command-line argument first
if len(sys.argv) > 1:
    input_data = sys.argv[1]
else:
    print("Error: Provide hex string as argument")
    exit(1)

try:
    decoded = bytes.fromhex(input_data).decode("utf-8")
    print(decoded)
except Exception as e:
    print(f"Invalid hex input: {e}")
    exit(1)