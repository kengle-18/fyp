import sys
import base64

# Try command-line argument first, then fall back to environment variable
if len(sys.argv) > 1:
    input_data = sys.argv[1]
else:
    print("Error: Provide base64 string as argument or set BASE64_INPUT environment variable")
    exit(1)

try:
    decoded = base64.b64decode(input_data).decode("utf-8")
    print(decoded)
except Exception as e:
    print(f"Invalid base64 input: {e}")
    exit(1)