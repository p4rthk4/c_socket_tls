# python3 checksum.py ./dummy_files -o dummy_files.json -a sha1
# python3 checksum.py ./tmp_files -o tmp_files.json -a sha1

import hashlib
import json
import os
import argparse

def compute_checksum(file_path, algorithm='md5'):
    """Compute the checksum of a file using the specified algorithm."""
    hash_func = hashlib.new(algorithm)
    
    try:
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b''):
                hash_func.update(chunk)
    except FileNotFoundError:
        return None
    except IOError as e:
        print(f"Error reading file {file_path}: {e}")
        return None

    return hash_func.hexdigest()

def list_files_in_directory(directory):
    """List all files in the given directory recursively."""
    file_list = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            file_list.append(file_path)
    return file_list

def main(directory, output_file, algorithm):
    """Compute checksums for all files in the given directory and save to JSON."""
    files = list_files_in_directory(directory)
    checksums = {}

    for file_path in files:
        checksum = compute_checksum(file_path, algorithm)
        if checksum:
            checksums[file_path.split("/")[-1]] = checksum
        else:
            checksums[file_path.split("/")[-1]] = 'Error computing checksum'

    with open(output_file, 'w') as f:
        json.dump(checksums, f, indent=4)

    print(f"Checksums written to {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compute checksums for all files in a directory.")
    parser.add_argument('directory', help='Directory to scan for files')
    parser.add_argument('-o', '--output', default='checksums.json', help='Output JSON file (default: checksums.json)')
    parser.add_argument('-a', '--algorithm', default='md5', choices=hashlib.algorithms_guaranteed, help='Hash algorithm to use (default: md5)')

    args = parser.parse_args()

    main(args.directory, args.output, args.algorithm)
