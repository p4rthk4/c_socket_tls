# python3 cmp_checksum.py dummy_files.json tmp_files.json -o cmp.json

import json
import argparse

def load_checksums(file_path):
    """Load checksums from a JSON file."""
    try:
        with open(file_path, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return None
    except json.JSONDecodeError:
        print(f"Error: Failed to parse JSON from {file_path}.")
        return None

def compare_checksums(file1_checksums, file2_checksums):
    """Compare checksums from two files and report differences."""
    differences = {
        "different_checksums": [],
        "missing_in_first": [],
        "missing_in_second": []
    }

    all_files = set(file1_checksums.keys()).union(set(file2_checksums.keys()))

    for file_path in all_files:
        checksum1 = file1_checksums.get(file_path)
        checksum2 = file2_checksums.get(file_path)

        if checksum1 and checksum2:
            if checksum1 != checksum2:
                differences["different_checksums"].append(file_path)
        elif checksum1 is None:
            differences["missing_in_first"].append(file_path)
        elif checksum2 is None:
            differences["missing_in_second"].append(file_path)

    return differences

def main(file1, file2, output_file=None):
    """Main function to compare two checksum files and output differences."""
    file1_checksums = load_checksums(file1)
    file2_checksums = load_checksums(file2)

    if file1_checksums is None or file2_checksums is None:
        return

    differences = compare_checksums(file1_checksums, file2_checksums)

    output = {
        "different_checksums": differences["different_checksums"],
        "missing_in_first": differences["missing_in_first"],
        "missing_in_second": differences["missing_in_second"]
    }

    if output_file:
        with open(output_file, 'w') as f:
            json.dump(output, f, indent=4)
        print(f"Differences written to {output_file}")
    else:
        print(json.dumps(output, indent=4))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare two checksum JSON files.")
    parser.add_argument('file1', help='First checksum JSON file')
    parser.add_argument('file2', help='Second checksum JSON file')
    parser.add_argument('-o', '--output', help='Output file for differences (default: print to console)')

    args = parser.parse_args()

    main(args.file1, args.file2, args.output)
