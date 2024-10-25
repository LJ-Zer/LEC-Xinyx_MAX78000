import serial

def capture_serial_data(port, baudrate, filename):
    ser = None
    try:
        # Attempt to open the serial port
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Successfully opened port {port}.")
        
        with open(filename, 'w') as file:
            print("Capturing serial data. Press Ctrl+C to stop.")
            while True:
                try:
                    # Read a line from the serial port
                    line = ser.readline().decode('utf-8')
                    if line:
                        # Write the line to the file
                        file.write(line)
                        print(line, end='')  # Print to console for feedback
                except KeyboardInterrupt:
                    print("\nCapture stopped.")
                    break
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except PermissionError as e:
        print(f"Permission error: {e}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if ser is not None:
            ser.close()
            print("Serial port closed.")

if __name__ == "__main__":
    # Configure these values based on your setup
    PORT = 'COM5'  # Replace with your serial port
    BAUDRATE = 115200  # Replace with your baud rate
    FILENAME = 'output.txt'

    capture_serial_data(PORT, BAUDRATE, FILENAME)
