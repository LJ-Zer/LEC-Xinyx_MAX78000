import serial
from datetime import datetime
import re
import os

SERIAL_PORT = 'COM5'
BAUD_RATE = 115200
TIMEOUT = 10
OUTPUT_DIR = 'hex'  # Directory to save files

# Function to generate filename with datetime first, followed by the cloud type
def generate_filename(special_word):
    timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
    return os.path.join(OUTPUT_DIR, f'{timestamp}_{special_word}.txt')

# Function to remove the first line of a file
def remove_first_line(filename):
    try:
        with open(filename, 'r') as file:
            lines = file.readlines()
        
        # Rewrite the file without the first line
        with open(filename, 'w') as file:
            file.writelines(lines[1:])
            
        print(f"First line removed from file: {filename}")
    except Exception as e:
        print(f"Error while removing the first line: {e}")

# Function to clean the data in the file
def clean_file_data(filename):
    try:
        with open(filename, 'r') as file:
            data = file.read()

        # Extract all hexadecimal values (0-9 and A-F) and ignore any non-hex characters
        hex_values = re.findall(r'[0-9A-Fa-f]{2}', data)
        
        # Join the values with a comma and ensure no extra spaces
        cleaned_data = ', '.join(hex_values)
        
        # Write cleaned data back to the file
        with open(filename, 'w') as file:
            file.write(cleaned_data)
        
        print(f"Data cleaned in file: {filename}")
    except Exception as e:
        print(f"Error while cleaning file data: {e}")

def read_serial_data(serial_port, baud_rate, timeout):
    try:
        # Ensure the output directory exists
        if not os.path.exists(OUTPUT_DIR):
            os.makedirs(OUTPUT_DIR)
        
        ser = serial.Serial(serial_port, baud_rate, timeout=timeout)
        print(f"Opening serial port {serial_port} with baud rate {baud_rate}...")

        buffer = ""
        file = None  # Start with no open file
        filename = None  # To keep track of the file name
        
        while True:
            data = ser.read(ser.in_waiting or 1).decode('utf-8', errors='ignore')
            if data:
                buffer += data
                print(data, end='')  # Display data in the console
                
                # Check for specific cloud types to create special filenames
                cloud_types = ["cirrus", "cumulus", "nimbostratus", "stratus"]
                for cloud_type in cloud_types:
                    if cloud_type in buffer:
                        if file: 
                            file.close()
                        filename = generate_filename(cloud_type)  # Use the timestamp + cloud type as filename
                        file = open(filename, 'w')  # Open a new file for writing
                        print(f"\nDetected cloud type '{cloud_type}'. Now saving data to: {filename}")
                        
                        buffer = ""  # Clear the buffer after creating a new file
                        break  # Stop checking other cloud types for now

                if file:
                    file.write(data)  # Write data to the current file
                    file.flush()  # Ensure data is written to the file immediately

    except serial.SerialException as e:
        print(f"Error: {e}")

    except KeyboardInterrupt:
        print("\nInterrupted by user, closing...")

    finally:
        if file and not file.closed:
            file.close()  # Close the file after writing all the data

        # Remove the first line and clean the file data after file is closed
        if filename:
            remove_first_line(filename)  # Call function to remove the first line
            clean_file_data(filename)  # Call function to clean the file data
        
        if 'ser' in locals() and ser.is_open:
            ser.close()  # Close the serial port
            print("Serial port closed.")

if __name__ == "__main__":
    read_serial_data(SERIAL_PORT, BAUD_RATE, TIMEOUT)
