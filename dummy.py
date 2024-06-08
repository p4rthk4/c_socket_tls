import os
import random

def generate_dummy_files(directory, num_files, min_size_mb, max_size_mb):
    os.makedirs(directory, exist_ok=True)
    for i in range(num_files):
        size_mb = random.randint(min_size_mb, max_size_mb)
        file_path = os.path.join(directory, f'dummy_file_{i+1}.bin')
        with open(file_path, 'wb') as f:
            f.write(os.urandom(size_mb * 1024 * 1024))
    print(f'{num_files} files generated in {directory}')

generate_dummy_files('dummy_files', 400, 5, 40)