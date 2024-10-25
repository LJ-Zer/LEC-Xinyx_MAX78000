import matplotlib.pyplot as plt
import re
import os
import numpy as np
import shutil

# Directory paths
input_dir = 'cleaned_hex'
output_dir = 'Xinyx_Images'
processed_dir = 'processed_c-hex'

# Ensure the output and processed directories exist
if not os.path.exists(output_dir):
    os.makedirs(output_dir)
if not os.path.exists(processed_dir):
    os.makedirs(processed_dir)

# Get a list of all .txt files in the input directory
txt_files = [f for f in os.listdir(input_dir) if f.endswith('.txt')]

# Current font size
current_fontsize = 12

for file_name in txt_files:
    file_path = os.path.join(input_dir, file_name)
    
    # Read and process the data from the file
    with open(file_path, 'r') as file:
        raw_data = file.read()

    # Clean the data: remove newlines, spaces, and ensure valid hexadecimal values
    raw_data = raw_data.replace('\n', ' ').replace(' ', '')  # Remove newlines and spaces
    hex_data = re.findall(r'[0-9A-Fa-f]{2}', raw_data)       # Extract valid 2-digit hex values

    try:
        data = [int(x, 16) for x in hex_data]
    except ValueError as e:
        print(f"Error in converting hex to int: {e}")
        continue  # Skip this file if there is an error

    max_data_size = 128 * 128  # 16384 values for a 128x128 image

    # Check if the data length is sufficient
    if len(data) < max_data_size:
        print(f"Data length is insufficient ({len(data)} values). Skipping file: {file_name}")
        continue  # Skip this file if there are not enough values

    # If the length is sufficient, truncate to the required size
    data = data[:max_data_size]

    print(f"Data length after adjustment: {len(data)}")

    # Convert the data to a NumPy array and reshape it
    image_data = np.array(data, dtype=np.uint8).reshape((128, 128))

    # Extract base name of the file without extension
    base_name = os.path.splitext(file_name)[0]

    # Path for the output image
    output_file_path = os.path.join(output_dir, f'{base_name}.jpeg')

    # Create a figure and axis
    fig, ax = plt.subplots(figsize=(8, 8))  # Adjust figsize as needed

    # Display the image
    ax.imshow(image_data, cmap='gray')
    ax.axis('off')  # Hide the axes

    # Increase font size by 5%
    new_fontsize = current_fontsize * 1.5

    # Add the full filename as the heading at the top of the image
    plt.text(0.5, 1.0, base_name, fontsize=new_fontsize, ha='center', va='bottom', transform=ax.transAxes, bbox=dict(facecolor='white', alpha=0.7))

    # Save the image as a JPEG file
    plt.savefig(output_file_path, format='jpeg', bbox_inches='tight', pad_inches=0)
    plt.close()  # Close the plot to free up memory

    print(f"Image saved as: {output_file_path}")

    # Move the processed file to the processed directory
    shutil.move(file_path, os.path.join(processed_dir, file_name))

print("Processing complete. All files have been processed and moved.")
