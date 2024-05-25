import serial
import json
import csv
import time
from datetime import datetime

# Create a serial connection
ser = serial.Serial('/dev/cu.usbmodemF412FA9B46602', 115200) # change this to the port your Arduino is connected to
output_csv_path = '/Users/emilyrodgers/Documents/myoware-simulation/output-files/output.csv' # change this to the path where you want to save the CSV file
output_json_path_template = '/Users/emilyrodgers/Documents/myoware-simulation/output-files/output{:06d}.json' #c hange this to the path where you want to save the JSON files
event_counter = 0  # Initialize an event counter
recording = False  # Flag to indicate if recording is active

# Open CSV file and write headers
with open(output_csv_path, 'w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['EventID', 'CurrentTime', 'SensorValue'])

# Function to handle CSV logging
def log_to_csv(data):
    with open(output_csv_path, 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(data)


try:
    print("Started listening on the serial port...")
    current_json_data = None

    while True:  # Run forever
        if ser.in_waiting > 0: # Check if there is data in the serial buffer
            line = ser.readline().decode('utf-8').strip() # Read the line and decode it
            print(line)  # Optionally print the line to the console

            # Handling potential JSON data
            if line.startswith('{') and line.endswith('}'):
                try:
                    data = json.loads(line)  # Parse the JSON data
                    if "eventStart" in data and data["eventStart"]:
                        event_counter += 1  # Increment event counter
                        recording = True  # Start recording
                        output_json_path = output_json_path_template.format(event_counter)
                        with open(output_json_path, 'w') as file_json:  # Open the JSON file for the current event
                            json.dump(data, file_json, indent=4)  # Write the JSON data to the file
                    elif "eventEnd" in data and data["eventEnd"]:
                        recording = False  # Stop recording
                        output_json_path = output_json_path_template.format(event_counter)
                        with open(output_json_path, 'w') as file_json:  # Open the JSON file for the current event
                            json.dump(data, file_json, indent=4)  # Write the JSON data to the file
                except json.JSONDecodeError:
                    print("Data is not in JSON format:", line)  # Print the error

            else:
                # Assume it's CSV or other data and handle accordingly
                parts = line.split(',')
                log_to_csv(parts)  # Log the data to CSV

except KeyboardInterrupt:  # If the user presses Ctrl+C
    print("Stopped by User")  # Print a message

except Exception as e:
    print("Error:", str(e))

finally:
    ser.close()
    print("Serial connection closed.")
