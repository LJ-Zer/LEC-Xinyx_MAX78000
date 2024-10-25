import subprocess
import time
import threading
import os

def run_script_for_duration(script_name, duration):
    """Run a script for a specified duration."""
    process = subprocess.Popen(['python', script_name])
    start_time = time.time()
    while time.time() - start_time < duration:
        # Check if process is still running
        if process.poll() is not None:
            print(f"{script_name} finished early.")
            return
        time.sleep(1)  # Sleep briefly to avoid busy-waiting
    # Terminate the process if it is still running after the duration
    process.terminate()
    process.wait()
    print(f"{script_name} terminated after {duration} seconds.")

if __name__ == "__main__":
    # Script durations in seconds
    durations = {
        'serial_dec.py': 90,      # 1.5 minutes
        'cleaner_hex.py': 5,
        'img_maker.py': 5,
        'git_uploader.py': 10     # 10 seconds
    }
    
    scripts = list(durations.keys())
    
    try:
        while True:
            print("Starting new iteration.")
            
            # Run each script for its specified duration
            for script in scripts:
                print(f"Running {script} for {durations[script]} seconds...")
                run_script_for_duration(script, durations[script])
                print(f"{script} completed.")
            
            print("Iteration completed.")
            
            # Optional: wait for a specified period before starting the next iteration
            time.sleep(5)  # Wait 10 seconds before starting the next iteration (adjust as needed)
    
    except KeyboardInterrupt:
        print("Process interrupted by user. Exiting...")
