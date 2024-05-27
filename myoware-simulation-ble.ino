// This version of the code is formatting everything correctly but the JSON it needs to send is too large so I'm reqwriting to send each data point over one by one

#include <SPI.h> // for communicating with the SD card
#include <SD.h> // for reading and writing to the SD card
#include <ArduinoJson.h> // for parsing JSON data

#include <ArduinoBLE.h>

String eventData; // String to store the event data
DynamicJsonDocument doc(512); // JSON document to store the event data

const int switchPin = 7; // digital pin for the switch
const int chipSelect = 10; // chip select pin for the SD card

int sensorValue = 0; // variable to store the value coming from the sensor
int sensorThreshold = 300; // threshold value to start recording
bool isRecording = false; // flag to indicate if recording is in progress

unsigned long lastTimeAboveThreshold = 0; // variable to store the last time the sensor value was above the threshold
unsigned long interval = 100; // interval set to 100 milliseconds
unsigned long lastSampleTime = 0; // variable to store the last time a sample was taken

int eventNumber = 0; // variable to store the event number
File dataFile; // file to store the sensor data

// Create a BLE service
BLEService myoService("19b10000-e8f2-537e-4f6c-d104768a1214"); // create a BLE service
BLEStringCharacteristic myDataChar("b820ebfe-13ad-4604-925f-132c4c4f78e1", BLERead | BLENotify, 512); // UUID and property for the characteristic

void setup() {
  Serial.begin(115200); // start serial communication at 115200 bps
  pinMode(switchPin, INPUT); // Setup the switch pin as an input

  while (!Serial) continue; // Wait for the serial monitor to open
  Serial.println("Starting"); // Print message to serial terminal

  // initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed!");
    return;
  }
  Serial.println("Card initialized."); // print message to serial terminal

  dataFile = SD.open("data.csv", FILE_WRITE); // Open the data file
  if (dataFile) { // If the file opened successfully
    if (dataFile.size() == 0) { // If the file is empty
      dataFile.println("EventID,CurrentTime,SensorValue"); // Write the header to the file
      Serial.println("CSV created successfully."); // Print a success message
    }
    dataFile.close();  // Close the file
  } else { // If the file failed to open
    Serial.println("Error opening data.csv"); // Print an error message
  }

  // Initialize the BLE module
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("MyoGrind");
  BLE.setAdvertisedService(myoService);
  myoService.addCharacteristic(myDataChar);
  BLE.addService(myoService);
  // myDataChar.writeValue("{\"eventStart\": false, \"eventNumber\": 0, \"startTime\": 0, \"samples\": [{\"time\": 0,\"value\": 0},{\"time\": 1000,\"value\": 0}],\"eventEnd\": false,\"endTime\": 1000}");  // Initial value
  
  BLE.advertise(); // Start advertising the service
  Serial.println("BLE Ready. Waiting for connections...");

}

/*
  The loop function reads the state of the switch and generates random sensor values based on the switch state.
  If the sensor value is above the threshold, it starts recording the sensor data.
  The sensor data is stored in a csv file and written to the SD card.
  The JSON object is then serialized and sent via Serial.
*/
void loop() {
  unsigned long currentTime = millis(); // Get the current time
  bool switchState = digitalRead(switchPin); // Read the state of the switch

  // Generate random sensor values based on switch state
  sensorValue = switchState == HIGH ? random(301, 700) : random(0, 299); 

  // wait for a Bluetooth® Low Energy central 
  BLEDevice central = BLE.central();
  
  // Check if the sensor value is above the threshold and recording is not in progress
  if (sensorValue > sensorThreshold && !isRecording) { 
    isRecording = true; // Set the recording flag to true
    eventData = ""; // Clear the eventData string
    eventNumber++; // Increment the event number
    doc.clear(); // Clear the JSON document
    doc["eventStart"] = true; // Signal that the event has started
    doc["eventNumber"] = eventNumber; // Add the event number to the JSON document
    doc["startTime"] = currentTime; // Add the start time to the JSON document
    JsonArray samples = doc.createNestedArray("samples"); // Initialize a JSON array for samples

    Serial.println("eventStart");
  } 

  // Check if recording is in progress
  if (isRecording) {
    if (currentTime - lastSampleTime >= interval) { // Check if the interval has elapsed
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
      Serial.println("eventEnd");
      doc["endTime"] = currentTime; // Add the end time to the JSON document
      doc["eventEnd"] = true; // Signal that the event has ended

      serializeJson(doc, eventData); // Serialize the JSON data
      Serial.println(eventData); // Send JSON string via Serial
      // myDataChar.writeValue(eventData); // Send JSON string via BLE

      // Print the size of the JSON document to the serial monitor
      Serial.print("Size of JSON document: ");
      Serial.print(eventData.length()); // Print the size of the JSON document
      Serial.println(" bytes");

      sendBLEData(eventData); // Send the JSON data over BLE

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

// Function to split JSON into manageable BLE sizes <512-bytes and send the JSON data over BLE
void sendBLEData(const String& jsonData) { 
  if (!BLE.connected()) {
    Serial.println("BLE Disconnected. Stopping data transmission.");
    return;
  }

  int len = jsonData.length();
  int pos = 0;
  char buffer[257]; // Static buffer to hold the substring

  while (pos < len) {
    int sendSize = min(256, len - pos);
    jsonData.substring(pos, pos + sendSize).toCharArray(buffer, 257); // Ensure the buffer is large enough
    buffer[sendSize] = '\0'; // Manually null-terminate the string

    if (myDataChar.writeValue(buffer)) { // Use the version that accepts a const char*
      Serial.print("Sent: ");
      Serial.println(buffer); // Debug output
    } else {
      Serial.println("Failed to send data.");
      break; // Stop sending if a transmission fails
    }
    pos += sendSize;
    delay(200); // Adjusted delay
  }
}