#include <Arduino.h>
#include <FS.h>     // File system support
#include <SD.h>     // SD card library
#include <SPI.h>    // SPI library

// Change this to your actual CS pin
#define chipSelect 5

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("SD Card Test Starting...");

  // 1) Initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("ERROR: SD card initialization failed!");
    while (true) {
      delay(1000); // Stop here if SD fails
    }
  }
  Serial.println("SD card initialized successfully.");

  // 2) Write a test file
  writeFile(SD, "/testFile.txt", "Hi from ESP32\n");

  // 3) Append to the same file
  appendFile(SD, "/testFile.txt", "Appending some data.\n");

  // 4) Read back the file
  readFile(SD, "/testFile.txt");

}

void loop() {
  // The main loop can remain empty for a basic SD test
  delay(1000);
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing to file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("ERROR: Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File write successful");
  } else {
    Serial.println("File write failed");
  }
  file.close();
}


void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path, FILE_READ);
  if(!file) {
    Serial.println("ERROR: Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");
  while(file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("ERROR: Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("File append successful");
  } else {
    Serial.println("File append failed");
  }
  file.close();
}
