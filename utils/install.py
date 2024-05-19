import os
import shutil
import sys

def parse_arguments():
    args = sys.argv[1:]
    arg_dict = {}
    for arg in args:
        k, v = arg.split("=")
        arg_dict[k] = v

    return arg_dict

def copy_file_to_directories(source_file, directories_file):
    # Open the file containing the list of directories
    with open(directories_file, 'r') as file:
        directories = file.readlines()

    # Loop through each directory in the list
    for directory in directories:
        directory = directory.strip()  # Remove any extra whitespace or newlines
        if os.path.isdir(directory):  # Check if the directory exists
            # Construct the full path where the file will be copied
            destination = os.path.join(directory, os.path.basename(source_file))
            # Copy the file
            shutil.copy(source_file, destination)
            print(f'Copied to: {destination}')
        else:
            print(f'Directory does not exist: {directory}')


if __name__ == "__main__":
    script_path = os.path.abspath(__file__)
    script_dir = os.path.dirname(script_path)

    dll_install_locations = os.path.join(script_dir, "dll_install_locations.txt")
    args = parse_arguments()

    for k,v in args.items():
        if k == "dll":
            print(v)
            copy_file_to_directories(v, dll_install_locations)