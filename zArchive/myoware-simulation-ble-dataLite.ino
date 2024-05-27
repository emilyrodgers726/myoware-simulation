/*
    The code reads sensor data and sends it over BLE to a connected device. 
    The sensor data is stored in a CSV file on an SD card and is also sent
    over BLE in JSON format. 
*/ 

#include <SPI.h> // for communicating with the SD card
#include <SD.h> // for reading and writing to the SD card
#include <ArduinoJson.h> // for parsing JSON data
#include <ArduinoBLE.h> // for using the BLE module

String eventData; // String to store the event data
DynamicJsonDocument doc(2048); // JSON document to store the event data
DynamicJsonDocument bleDoc(256); // JSON document to store the BLE data

const int switchPin = 7; // digital pin for the switch
const int chipSelect = 10; // chip select pin for the SD card

int sensorValue = 0; // variable to store the value coming from the sensor
int sensorThreshold = 300; // threshold value to start recording
bool isRecording = false; // flag to indicate if recording is in progress

unsigned long startTime = 0; // variable to store the start time of the event
unsigned long lastTimeAboveThreshold = 0; // variable to store the last time the sensor value was above the threshold
unsigned long interval = 100; // interval set to 100 milliseconds
unsigned long lastSampleTime = 0; // variable to store the last time a sample was taken

int eventNumber = 0; // variable to store the event number
int maxSensorValue = 0; // variable to store the maximum sensor value
int sumSensorValues = 0; // variable to store the sum of sensor values
int sampleCount = 0; // variable to store the number of samples taken

File dataFile; // file to store the sensor data

// Create a BLE service
BLEService myoService("19b10000-e8f2-537e-4f6c-d104768a1214"); // create a BLE service
BLEStringCharacteristic myDataChar("b820ebfe-13ad-4604-925f-132c4c4f78e1", BLERead | BLENotify, 512); // UUID and property for the characteristic
bool bleConnected = false; // Global variable to track BLE connection status

void setup() {
  Serial.begin(115200); // start serial communication at 115200 bps
  pinMode(switchPin, INPUT); // Setup the switch pin as an input

  while (!Serial) continue; // Wait for the serial monitor to open
  delay(5000); // Give a little delay to settle the serial connection
  Serial.println("Starting"); // Print message to serial terminal

  // initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed!");
    return;
  }
  Serial.println("Card initialized."); // print message to serial terminal

  eventNumber = getMaxEventNumberFromSD(); // Set the starting event number

  delay (1000); // Delay for 1 second

  dataFile = SD.open("data.csv", FILE_WRITE); // Open the data file
  if (dataFile) { // If the file opened successfully
    if (dataFile.size() == 0) { // If the file is empty
      dataFile.println("EventID,CurrentTime,SensorValue"); // Write the header to the file
      Serial.println("CSV created successfully."); // Print a success message
    } else { // Print a message if the file already exists
        Serial.println("CSV already exists. This program will append new data to the exisiting file."); 
    }
    dataFile.close();  // Close the file
  } else { // If the file failed to open
    Serial.println("Error opening data.csv"); // Print an error message
  }

  // Initialize the BLE module
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!"); // Print a message if the BLE module failed to start
    while (1); // If the BLE module failed to start, stop the program
  }

  BLE.setLocalName("MyoGrind"); //  Set the local name of the device
  BLE.setAdvertisedService(myoService); // Set the advertised service
  myoService.addCharacteristic(myDataChar); // Add the characteristic to the service
  BLE.addService(myoService); // Add the service
  
  BLE.advertise(); // Start advertising the service
  Serial.println("BLE Ready. Waiting for connections...");

}
void loop() {
  unsigned long currentTime = millis(); // Get the current time
  bool switchState = digitalRead(switchPin); // Read the state of the switch

  // Generate random sensor values based on switch state
  sensorValue = switchState == HIGH ? random(301, 700) : random(0, 299); 

  // wait for a Bluetooth® Low Energy central 
  BLEDevice central = BLE.central();
  // Check if a central is connected/disconnected
  if (BLE.connected()) {
    if (!bleConnected) {
      bleConnected = true; // Update the connection status
      Serial.println("BLE Device Connected"); // Print message when device is connected
    }
  } else {
    if (bleConnected) {
      bleConnected = false; // Update the connection status
      Serial.println("BLE Device Disconnected"); // Print message when device is disconnected
    }
  }
  
  // Check if the sensor value is above the threshold and recording is not in progress
  if (sensorValue > sensorThreshold && !isRecording) { 
    isRecording = true; // Set the recording flag to true
    
    startTime = currentTime; // Set the start time to the current time
    maxSensorValue = sensorValue; // Set the max sensor value to the current sensor value
    sumSensorValues = sensorValue;  // Set the sum of sensor values to the current sensor value
    sampleCount = 1; // Set the sample count to 1
    
    eventData = ""; // Clear the eventData string
    eventNumber++; // Increment the event number
    doc.clear(); // Clear the JSON document
    doc["eventStart"] = true; // Signal that the event has started
    doc["eventNumber"] = eventNumber; // Add the event number to the JSON document
    doc["startTime"] = currentTime; // Add the start time to the JSON document
    bleDoc["startTime"] = currentTime; // Add the start time to the BLE JSON document
    JsonArray samples = doc.createNestedArray("samples"); // Initialize a JSON array for samples

    Serial.println("eventStart");
  } 

  // Check if recording is in progress
  if (isRecording) {
    if (currentTime - lastSampleTime >= interval) { // Check if the interval has elapsed
      // dataLite JSON handling
      maxSensorValue = max(maxSensorValue, sensorValue); // Update the max sensor value
      sumSensorValues += sensorValue;   // Update the sum of sensor values
      sampleCount++; // Increment the sample count
      
      // Big JSON creation (for local storage only, use read_serial.py script)
      JsonArray samples = doc["samples"]; // Get the JSON array of samples
      JsonObject sample = samples.createNestedObject(); // Create a new object in the array
      sample["time"] = currentTime; // Add the current time to the sample
      sample["value"] = sensorValue;  // Add the sensor value to the sample

      // Print the sensor value to the serial monitor
      Serial.print(eventNumber); // Print the event number
      Serial.print(", ");
      Serial.print(currentTime); // Print the current time
      Serial.print(", ");
      Serial.println(sensorValue); // Print the sensor value

      // Save to SD card
      saveToSDCard(eventNumber, currentTime, sensorValue);
      
      lastSampleTime = currentTime;   // Update the last sample time
    }

    // Check if the sensor value is below the threshold and the recording has been going on for more than 1 second
    if (sensorValue > sensorThreshold) { // If the sensor value is above the threshold
      lastTimeAboveThreshold = currentTime; // Update the last time the sensor value was above the threshold
    } else if (currentTime - lastTimeAboveThreshold > 1000) { // If the sensor value is below the threshold and the recording has been going on for more than 1 second
      isRecording = false; // Set the recording flag to false
      
      // dataLite JSON handling
      unsigned long eventDuration = currentTime - startTime; // Calculate the event duration
      int averageSensorValue = sampleCount > 0 ? sumSensorValues / sampleCount : 0; // Calculate the average sensor value
      bleDoc["eventNumber"] = eventNumber; // Add the event number to the JSON document
      bleDoc["eventDuration"] = eventDuration; // Add the event duration to the JSON document
      bleDoc["maxSensorValue"] = maxSensorValue; // Add the max sensor value to the JSON document
      bleDoc["averageSensorValue"] = averageSensorValue; // Add the average sensor value to the JSON document
      bleDoc["endTime"] = currentTime; // Add the end time to the JSON document
      String output; // String to store the JSON data
      serializeJson(bleDoc, output);    // Serialize the JSON data
      sendBLEData(output); // Send the JSON data over BLE
      Serial.println("Data sent over BLE"); // Print message to serial monitor

      Serial.println("eventEnd");

      doc["endTime"] = currentTime; // Add the end time to the JSON document
      doc["eventEnd"] = true; // Signal that the event has ended
      serializeJson(doc, eventData); // Serialize the JSON data
      Serial.println(eventData); // Send JSON string via Serial

      eventData = ""; // Reset eventData string
      Serial.println(); // End the line for serial monitor clarity
    }
  } 
  
  delay(50); // Delay for 50 milliseconds
      
}

// Function to save the sensor data to the SD card
void saveToSDCard(int eventNumber, unsigned long time, int value) {
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(eventNumber);
    dataFile.print(",");
    dataFile.print(time);
    dataFile.print(",");
    dataFile.println(value);
    dataFile.close();
  }
}

// Function to send the JSON data over BLE
void sendBLEData(const String& jsonData) { 
  if (!BLE.connected()) {
    Serial.println("BLE Disconnected. Stopping data transmission.");
    return;
  }

  if (myDataChar.writeValue(jsonData.c_str())) { 
    Serial.print("Sent: ");
    Serial.println(jsonData); // Print the JSON data to the serial monitor
  } else {
    Serial.println("Failed to send data.");
  }
}

int getMaxEventNumberFromSD() {
  int maxEvent = 0;
  dataFile = SD.open("data.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      int firstCommaIndex = line.indexOf(',');
      int eventId = line.substring(0, firstCommaIndex).toInt();
      if (eventId > maxEvent) {
        maxEvent = eventId;
      }
    }
    dataFile.close();
  } else {
    Serial.println("Failed to open data.csv for reading");
  }
  return maxEvent;
}

