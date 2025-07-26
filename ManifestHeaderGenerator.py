'''

This script is used to generate the Mainfest Header that the Resource Manager utilizes

'''
import argparse
import sys
import os
import json
import re

parser = argparse.ArgumentParser(description ='Generate Resource Manifest Header file from JSON manifest.')
parser.add_argument('--files',
                    type = str, nargs ='+',
                    help ='Space separated list of files to generate Manifest Headers for')

# Check that filws were provided
args = parser.parse_args()
if args.files:
    num_files = len(args.files)
    if num_files == 0:
        print("No JSON files were provided", file=sys.stderr)
        exit(1)
else:
    print("No JSON files were provided", file=sys.stderr)
    exit(1)

# Check that files exist and are JSON files
invalid_path = False
for path in args.files:
    if not (os.path.exists(path)):
        print(f"Could not find Manifest file: {path}", file=sys.stderr)
        invalid_path = True
        continue

    filename, file_extension = os.path.splitext(path)
    if file_extension != ".json":
        print(f"Manifest {path} must be a JSON file", file=sys.stderr)
        invalid_path = True
        continue

if invalid_path:
    exit(2)

pattern = r"[\-\+]"

# Replaces "- and + symbols with _"
def FormatName(name):
    return re.sub(pattern, "_", name)

def RecursiveParse(data, name, out_file, depth):
    if isinstance(data, dict):
        out_file.write((depth * "\t") + f"namespace {FormatName(name)}" + " {\n")
        for key, value in data.items():
            RecursiveParse(value, key, out_file, depth + 1) # Recursive call for nested dictionaries/lists
        out_file.write((depth * "\t") + "}\n")

    elif isinstance(data, list):
        out_file.write((depth * "\t") + f"namespace {FormatName(name)}" + " {\n")
        out_file.write(((depth + 1) * "\t") + f'constexpr UUID {FormatName(name)} = hash_str("{name}");\n')
        out_file.write(((depth + 1) * "\t") + f'constexpr PATHS {FormatName(name)} = ' + '{')
        first = True
        for item in data:
            if not isinstance(item, dict) and not isinstance(item, list):
                if first:
                    out_file.write(f'hash_str("{item}")')
                    first = False
                else:
                    out_file.write(f', hash_str("{item}")')
        out_file.write("}\n")
        out_file.write((depth * "\t") + "}\n")
    else:
        out_file.write((depth * "\t") + f"namespace {FormatName(name)}" + " {\n")
        out_file.write(((depth + 1) * "\t") + f'constexpr UUID {FormatName(name)} = hash_str("{data}");\n')
        out_file.write(((depth + 1) * "\t") + f'constexpr PATH {FormatName(name)} = "{data}";\n')
        out_file.write((depth * "\t") + "}\n")

def GenerateHeaders(data, out_filename):
    with open(out_filename + ".h", "w") as out:
        out.write("#pragma once\n")
        out.write('#include"UUID.h"\n\n')
        RecursiveParse(data, "Resources", out, 0)
        out.close()
def ParseHeadersManifestJSON(path):
    try:
        with open(path, 'r') as out:
            filename_with_extension = os.path.basename(path)
            filename_without_extension = os.path.splitext(filename_with_extension)[0]
            data = json.load(out)
            GenerateHeaders(data, filename_without_extension)
    except FileNotFoundError:
        print(f"Error: File not found at {path}")
        exit(5)
    except json.JSONDecodeError:
        print(f"Error: File at {path} is not a valid JSON file.")
        exit(6)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        exit(7)



# Generate each manifest
for path in args.files:
    ParseHeadersManifestJSON(path)