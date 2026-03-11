import sys

# example 0a 05 66 69 6e 61 6c 10 03
# formaty is key_value
# 0a --> split into 00001 | 010, field name 1. wire type of 010 (2) (length-delimited, string)
# 05 --> length of string is 5 btyes (66 69 6e 61 6c)
# 10 --> 00010 | 000, field name 2. wire type 000 (0) (varaint)
# 03 --> is the btye for value of 3. 

def read_varint(data, i):
    value = 0
    shift = 0
    while True:
        b = data[i]
        i += 1
        value |= (b & 0x7F) << shift
        if not (b & 0x80):
            break
        shift += 7
    return value, i


def decode(data, indent=0):
    i = 0
    while i < len(data):
        key, i = read_varint(data, i)
        # extract field number 1
        # extract field number 2 
        field = key >> 3
        # extarct wire type of 010
        # extact wire type of 000
        wire = key & 7

        pad = " " * indent

        if wire == 0:  # varint
            value, i = read_varint(data, i)
            print(f"{pad}field {field} (varint) = {value}")

        elif wire == 2:  # length-delimited
            # Get length of 5
            length, i = read_varint(data, i)
            value = data[i:i+length]
            i += length

            try:
                # convert btyes to ASCII
                text = value.decode("utf-8")
                print(f"{pad}field {field} (string) = '{text}'")
            except:
                print(f"{pad}field {field} (bytes) ->")
                decode(value, indent + 2)

        else:
            print(f"{pad}field {field} unsupported wire type {wire}")
            return


if len(sys.argv) < 2:
    print("Usage: python decode.py HEXSTRING")
    sys.exit(1)

data = bytes.fromhex(sys.argv[1])
decode(data)