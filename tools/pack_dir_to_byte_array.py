#!/usr/bin/env python3

import os
import re
import argparse

def sanitize_variable_name(name):
    """Sanitize the file name to be a valid C variable name."""
    return re.sub(r'\W|^(?=\d)', '_', name)

def file_to_c_byte_array(directory, output_file):
    """Read each file in the directory and generate a C byte array for it, then save to output_file."""
    with open(output_file, 'w') as out_f:
        for filename in os.listdir(directory):
            if os.path.isfile(os.path.join(directory, filename)):
                # Create a valid C variable name from the file name
                var_name = sanitize_variable_name(filename)
                # Read the file's bytes
                with open(os.path.join(directory, filename), 'rb') as file:
                    file_bytes = file.read()
                    # Start generating the C byte array string
                    byte_array_str = f"unsigned char {var_name}[] = {{\n    "
                    # Convert each byte to its hexadecimal representation and add to the string
                    byte_array_str += ', '.join(f'0x{byte:02x}' for byte in file_bytes)
                    byte_array_str += "\n};\n\n"
                    out_f.write(byte_array_str)

def main():
    parser = argparse.ArgumentParser(description="Generate C byte arrays from files in a directory.")
    parser.add_argument("directory", type=str, help="Path to the directory containing files to convert.")
    parser.add_argument("output_file", type=str, help="Path to the output .c file to store the byte arrays.")
    args = parser.parse_args()

    file_to_c_byte_array(args.directory, args.output_file)
    print(f"Byte arrays have been written to {args.output_file}")

if __name__ == "__main__":
    main()
