#include <SPI.h> // for communicating with the SD card
#include <SD.h> // for reading and writing to the SD card
#include <ArduinoJson.h> // for parsing JSON data

String eventData; // String to store the event data
DynamicJsonDocument doc(1024); // JSON document to store the event data

const int switchPin = 7; // digital pin for the switch

int sensorValue = 0; // variable to store the value coming from the sensor
int sensorThreshold = 300; // threshold value to start recording
bool isRecording = false; // flag to indicate if recording is in progress

unsigned long lastTimeAboveThreshold = 0; // variable to store the last time the sensor value was above the threshold
unsigned long interval = 100; // interval set to 100 milliseconds
unsigned long lastSampleTime = 0; // variable to store the last time a sample was taken

int eventNumber = 0; // variable to store the event number
File dataFile; // file to store the sensor data

void setup() {
  pinMode(switchPin, INPUT); // Setup the switch pin as an input
  Serial.begin(115200); // start serial communication at 115200 bps

  while (!Serial) {
    ;  // Wait for Serial Monitor to open
  
  }

  Serial.println("Simulated MyoWare Sensor Data Logging");

  dataFile = SD.open("data.csv", FILE_WRITE); // Open the data file
  if (dataFile) { // If the file opened successfully
    if (dataFile.size() == 0) { // If the file is empty
      dataFile.println("EventID,CurrentTime,SensorValue"); // Write the header to the file
    }
    dataFile.close();  // Close the file
  } else { // If the file failed to open
    Serial.println("Error opening data.csv"); // Print an error message
  }
}

/*
  The loop function reads the state of the switch and generates random sensor values based on the switch state.
  If the sensor value is above the threshold, it starts recording the sensor data.
  The sensor data is stored in a JSON object and written to the SD card.
  The JSON object is then serialized and sent via Serial.
*/
void loop() {
  unsigned long currentTime = millis(); // Get the current time
  bool switchState = digitalRead(switchPin); // Read the state of the switch

  // Generate random sensor values based on switch state
  sensorValue = switchState == HIGH ? random(301, 700) : random(0, 299); 

//   // Print the sensor value to the serial monitor
//   Serial.print(eventNumber); // Print the event number
//   Serial.print(", ");
//   Serial.print(currentTime); // Print the current time
//   Serial.print(", ");
//   Serial.println(sensorValue); // Print the sensor value

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
    JsonObject sample = samples.createNestedObject(); // Add first sample
    sample["time"] = currentTime;
    sample["value"] = sensorValue;

    Serial.println("eventStart");
    // Print the sensor value to the serial monitor
    // NOTE: This needs to be converted to SD card writing when the code is integrated with the SD card
    Serial.print(eventNumber); // Print the event number
    Serial.print(", ");
    Serial.print(currentTime); // Print the current time
    Serial.print(", ");
    Serial.println(sensorValue); // Print the sensor value
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

      dataFile = SD.open("data.csv", FILE_WRITE); // Open the data file
      if (dataFile) { // If the file opened successfully
        dataFile.print(eventNumber);  // Write the event number to the file
        dataFile.print(",");
        dataFile.print(currentTime);  // Write the current time to the file
        dataFile.print(",");
        dataFile.println(sensorValue); // Write the sensor value to the file
        dataFile.close();
      }
      lastSampleTime = currentTime;   // Update the last sample time
    }

    // Check if the sensor value is below the threshold and the recording has been going on for more than 1 second
    if (sensorValue > sensorThreshold) { // If the sensor value is above the threshold
      lastTimeAboveThreshold = currentTime; // Update the last time the sensor value was above the threshold
    } else if (currentTime - lastTimeAboveThreshold > 1000) { // If the sensor value is below the threshold and the recording has been going on for more than 1 second
      isRecording = false; // Set the recording flag to false
      doc["eventEnd"] = true; // Signal that the event has ended
        Serial.println("eventEnd");
      doc["endTime"] = currentTime; // Add the end time to the JSON document

      serializeJson(doc, eventData); // Serialize the JSON data
      Serial.println(eventData); // Send JSON string via Serial
      eventData = ""; // Reset eventData string
      Serial.println(); // End the line for serial monitor clarity
    }
  } 
  
  delay(50); // Delay for 50 milliseconds
}
