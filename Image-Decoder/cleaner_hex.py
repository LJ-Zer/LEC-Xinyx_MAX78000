import re
import os
import shutil

def clean_hex_file(filename, cleaned_folder):
    try:
        with open(filename, 'r') as file:
            raw_data = file.read()

        # Extract valid hexadecimal values
        cleaned_data = re.findall(r'[0-9A-Fa-f]{2}', raw_data)

        # Join values with a comma and ensure no extra spaces
        cleaned_data_str = ', '.join(cleaned_data)
        
        # Write cleaned data back to the file
        with open(filename, 'w') as file:
            file.write(cleaned_data_str)

        print(f"Data cleaned in file: {filename}")

        # Move the cleaned file to the cleaned_hex folder
        if not os.path.exists(cleaned_folder):
            os.makedirs(cleaned_folder)
        
        # Move file
        base_filename = os.path.basename(filename)
        new_file_path = os.path.join(cleaned_folder, base_filename)
        shutil.move(filename, new_file_path)

        print(f"File moved to: {new_file_path}")

    except Exception as e:
        print(f"Error while cleaning or moving file: {e}")

if __name__ == "__main__":
    hex_folder = 'hex'
    cleaned_hex_folder = 'cleaned_hex'

    # Ensure the 'hex' folder exists and contains files
    if os.path.exists(hex_folder):
        for filename in os.listdir(hex_folder):
            file_path = os.path.join(hex_folder, filename)
            if os.path.isfile(file_path):
                clean_hex_file(file_path, cleaned_hex_folder)
    else:
        print(f"Folder '{hex_folder}' does not exist.")
