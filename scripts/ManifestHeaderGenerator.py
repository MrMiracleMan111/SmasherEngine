'''

This script is used to generate the Mainfest Header that the Resource Manager utilizes

'''
import argparse
import sys
import os
import json
import re
from pathlib import Path

parser = argparse.ArgumentParser(description ='Generate Resource Manifest Header file from JSON manifest.')
parser.add_argument('--files',
                    type = str, nargs ='+',
                    help ='Space separated list of files to generate Manifest Headers for')

parser.add_argument('--out',
                    type = str, nargs ='+',
                    help ='Space separated list of paths to generate Manifest Headers at')

parser.add_argument('--resources',
                    type = str, nargs ='+',
                    help ='Path to Resource Directory')


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

if args.out:
    if len(args.files) != len(args.out):
        print("Number of outputs doesn't match number of inputs", file=sys.stderr)
        exit(1)

if args.resources:
    if len(args.files) != len(args.resources):
        print("Number of Resource Paths doesn't match number of inputs", file=sys.stderr)
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

def RecursiveParse(data, name, out_file, depth, resource_dir=None):
    if isinstance(data, dict):
        out_file.write((depth * "\t") + f"namespace {FormatName(name)}" + " {\n")
        
        # Write metadata
        if depth == 1:
            out_file.write(
            """
        namespace Metadata {
            static const inline ResourcePath RESOURCES_DIRECTORY {"WORKING_PATH"};
        }
""".replace("WORKING_PATH", resource_dir).replace("\\","/"))

        for key, value in data.items():
            RecursiveParse(value, key, out_file, depth + 1) # Recursive call for nested dictionaries/lists
        out_file.write((depth * "\t") + "}\n")

    elif isinstance(data, list):
        out_file.write((depth * "\t") + f"struct {FormatName(name)}" + " {\n")
        out_file.write(((depth + 1) * "\t") + f'static constexpr ResourceId Id = hash_str("{name}");\n')
        out_file.write(((depth + 1) * "\t") + f'static inline const ResourcePath PATHS[] = ' + '{')
        first = True
        for item in data:
            if not isinstance(item, dict) and not isinstance(item, list):
                if first:
                    out_file.write(f'ResourcePath {{ "{item}" }}')
                    first = False
                else:
                    out_file.write(f', ResourcePath {{ "{item}" }}')
        out_file.write("};\n")
        out_file.write((depth * "\t") + "};\n")
    else:
        out_file.write((depth * "\t") + f"struct {FormatName(name)}" + " {\n")
        out_file.write(((depth + 1) * "\t") + f'static constexpr ResourceId Id = hash_str("{data}");\n')
        out_file.write(((depth + 1) * "\t") + f'static inline const ResourcePath PATH {{ "{data}" }};\n')
        out_file.write((depth * "\t") + "};\n")

def GenerateHeaders(data, out_path, resource_dir=None):
    try:
        with open(out_path, "w") as out:
            out.write(
"""
#pragma once
#include "Smasher/Base.h"

namespace Smasher {
""")
            RecursiveParse(data, "Manifest", out, 1, resource_dir)
            out.write("}\n");
            out.close()
    except Exception as e:
        print(f"An unexpected error occurred when opening output file: {e}")
        exit(8)
def ParseHeadersManifestJSON(json_path, out_path=None, resource_dir=None):
    try:
        with open(json_path, 'r') as out:
            if out_path == None:
                (dir, filename) = os.path.split(json_path)
                filename_without_extension = os.path.splitext(filename)[0]
                out_path = os.path.join(dir, filename_without_extension + ".h")
            print(f"Using out path {out_path}")
            data = json.load(out)
            GenerateHeaders(data, out_path, resource_dir)
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
for index in range(len(args.files)):
    path = args.files[index]
    resource_dir = ""
    if args.resources:
        resource_dir = args.resources[index]
        if not resource_dir.endswith('/'):
            resource_dir = resource_dir + '/'
    if args.out:
        ParseHeadersManifestJSON(path, args.out[index], resource_dir)
    else:
        ParseHeadersManifestJSON(path, None, resource_dir)