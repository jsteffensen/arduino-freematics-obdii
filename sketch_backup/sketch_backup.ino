#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <OBD2UART.h>

// LCD and SD card settings
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD's I2C address
const int chipSelectSD = 4;
const int chipSelectEthernet = 10; // Not used in this example, but required for initialization

// OBD2 UART
COBD obd;

// Time tracking for loop actions
unsigned long lastTime = 0;
const long interval = 10000; // Interval for loop actions (10000 ms or 10 seconds)

// Define the size of each CAN frame (max 8 data bytes + overhead)
#define CAN_FRAME_SIZE 16 // Adjusted for overhead
#define BUFFER_SIZE 256 // Number of frames in the buffer

// Create a buffer to hold the CAN frames
byte canBuffer[BUFFER_SIZE][CAN_FRAME_SIZE];
int bufferIndex = 0;

void setup() {

  // Use an unconnected analog pin to seed the random number generator
  randomSeed(analogRead(0));

  // Initialize serial communications
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Freematics OBD-2");

  // Initialize SD card
  pinMode(chipSelectSD, OUTPUT);
  digitalWrite(chipSelectSD, HIGH);
  if (!SD.begin(chipSelectSD)) {
    lcd.setCursor(0, 1);
    lcd.print("SD Fail");
    Serial.println("SD initialization failed!");
    while (1); // Stop everything if SD fails
  }
  lcd.setCursor(0, 1);
  lcd.print("SD OK    ");
  overwriteData("Startup complete");

  // Initialize OBD
  Serial1.begin(115200); // OBD-II on Serial1
  if (!obd.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("OBD Fail ");
    Serial.println("OBD initialization failed!");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("OBD OK   ");
  }
}

void loop() {

  // Periodic actions every interval
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    // do stuff
  }

  if (receiveCANFrame(canBuffer[bufferIndex])) {
    bufferIndex++;
    if (bufferIndex >= BUFFER_SIZE) {
        writeToSDCard(canBuffer, BUFFER_SIZE);
        bufferIndex = 0; // Reset buffer index
    }
  }
}

String generateFileName() {
  unsigned long currentTime = millis();
  int randomNum = random(9999); // Add some randomness
  String fileName = "Data_" + String(randomNum) + ".txt";
  return fileName;
}

bool receiveCANFrame(byte *frame) {
    // Logic to receive a CAN frame from the bus
    // Return true if a frame is received
    return false;
}

void overwriteData(String dataLine) {
  
  // Open the file in write mode, which creates it if it doesn't exist
  String fileName = generateFileName();
  Serial.println(String("Writing to file->") + fileName.c_str());
  File dataFile = SD.open(fileName.c_str(), FILE_WRITE);
  if (dataFile) {
    dataFile.close(); // Close the file immediately to clear the contents

    // Reopen the file to write the data
    dataFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataLine); // Write the new data
      dataFile.close(); // Close the file to save changes
      Serial.println("Data overwritten: " + dataLine);
    } else {
      Serial.println("Error reopening " + fileName);
    }
  } else {
    Serial.println("Error opening " + fileName + " for clearing");
  }
}

void writeToSDCard(byte buffer[][CAN_FRAME_SIZE], int numFrames) {
    // Write the buffer to the SD card
    File dataFile = SD.open("can_data.txt", FILE_WRITE);
    if (dataFile) {
        for (int i = 0; i < numFrames; i++) {
            dataFile.write(buffer[i], CAN_FRAME_SIZE);
        }
        dataFile.close();
    }
}
