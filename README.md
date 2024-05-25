### READ ME

#### Project Description
This project simulates sensor data from a MyoWare muscle sensor and logs it to both a CSV file and a JSON format based on certain thresholds. The Arduino code generates random sensor data and logs this data to an SD card and over serial communication, mimicking the behavior of an actual sensor setup. The Python script reads this serial data, interprets it, and saves it to files on the computer.

#### Components
- **Arduino Uno or similar microcontroller**
- **MyoWare Muscle Sensor** (simulated in this code)
- **SD Card Module**
- **Python 3.6+ environment**

#### File Descriptions
- `myoware-simulation.ino`: Arduino sketch to simulate the MyoWare sensor data logging to an SD card and outputting over serial. Uses a switch connected to the Arduino digital port 7 instead of an actual EMG sensor.
- `read_serial.py`: Python script to read the serial output from the Arduino, log data to a CSV file on local machine, and save events as JSON files on local machine.  Use this to ensure the csv and JSON files are being generated correctly before sending over Bluetooth.

### Installation & Setup

#### Hardware Setup
1. **Connect the Switch** to your Arduino according to the switch's specifications (simulated in this sketch). Note that if you do not connect the switch to digitial input pin 7, you will need to change the input on the myoware-simulation.ino software.
2. **Attach the SD Card Module** to the Arduino using the SPI pins.
3. **Ensure that the Arduino is connected** to the computer via a USB cable.

#### Software Setup

**For the Arduino:**
1. **Install Arduino IDE** if not already installed, available at [Arduino Software](https://www.arduino.cc/en/software).
2. **Install Required Libraries**: Open the Arduino IDE, go to Sketch > Include Library > Manage Libraries. Install the following:
   - `SPI`
   - `SD`
   - `ArduinoJson`
3. **Load the Sketch**: Open `myoware-simulation.ino` in the Arduino IDE.
4. **Select the Correct Board and Port**: Under Tools > Board and Tools > Port.
5. **Upload the Sketch** to the Arduino.

**For the Python Script:**
0. **Change the Python Script Code**: Change lines 8, 9, and 10 of the read_serial.py file to be specific to your Arduino port and file path.  Be careful not to save the output files to a place where they might get synced with Github.
1. **Ensure Python is Installed**: Check by running `python --version` or `python3 --version` in your terminal.  You need python3 for this script.
2. **Install Required Python Libraries**:
   ```
   pip3 install pyserial
   ```
3. **Run the Python Script**: Navigate to the script’s directory and run:
   ```
   python3 read_serial.py
   ```

#### Running the System
- Power the Arduino, and it will start sending data over serial whenever the switch is in the on or 5V position (to simulate a bruxism event), which the Python script will read, display, and log to files.
- Data logging in CSV and JSON formats will commence automatically when the sensor values exceed predefined thresholds.
- Whenever you turn the switch back to GND or 0, the serial terminal will stop printing data and "non-events" will not be recorded in the CSV or JSON files.

This setup and documentation ensure the project components interact correctly, simulating and recording sensor data efficiently.

### Additional Information About the Motivation for Creating a Simulation
Certainly! The decision to use a switch connected to the Arduino instead of real MyoWare EMG sensors in a project setup like the one described can be driven by several factors:

#### 1. **Cost-Effectiveness**
Using a simple switch is far more cost-effective compared to using actual MyoWare EMG sensors. EMG sensors are specialized devices that are typically more expensive. For initial testing, prototyping, or educational projects, minimizing cost while still demonstrating functionality can be crucial.

#### 2. **Simplicity and Accessibility**
A switch simplifies the setup considerably. EMG sensors require proper placement on the muscle, correct wiring, and potentially complex calibration to ensure accurate readings. A switch, on the other hand, offers a straightforward, binary input (on/off) that is easy to implement and understand, making it ideal for demonstrations, educational purposes, or initial software testing.

#### 3. **Control Over Inputs**
Using a switch allows developers to control when data logging should start without having to physically exert themselves to generate muscle activity. This can be particularly useful in scenarios where the focus is on testing the data logging and processing capabilities of the system rather than the sensor input itself.

#### 4. **Safety and Reliability**
In some cases, especially in educational environments or when the final application of the project is still under development, using a switch avoids any issues related to the safety and reliability of attaching sensors to the body. It removes the need for considering skin sensitivity, sensor placement, and the nuances of interpreting EMG signals.

#### 5. **Focus on Software Development**
Using a switch can shift the focus purely to software development and system integration aspects of a project. This setup allows developers to perfect the data logging, file handling, and other system functionalities without being bogged down by the complexities of accurate sensor data acquisition and interpretation.

#### 6. **Demonstration and Testing**
For demonstration purposes, a switch can simulate sensor activation in a controlled manner, allowing stakeholders or students to clearly see how the system responds to "on" and "off" states. This can be especially useful in teaching scenarios, where the concept of digital signals and their impact on system behavior are being explained.

#### 7. **Modularity and Flexibility**
Using a switch provides a modular approach to building a project. Initially, a simple switch can be used to develop and test the system. Later, it can be replaced with an actual MyoWare sensor or other types of input as the project requirements evolve. This flexibility can be vital in phased project developments or iterative testing environments.

In sum, using a switch instead of a MyoWare EMG sensor in Arduino projects, especially during the early stages of development or in educational settings, allows for simplified setup, controlled testing environments, cost savings, and focuses on developing robust software before integrating more complex hardware.