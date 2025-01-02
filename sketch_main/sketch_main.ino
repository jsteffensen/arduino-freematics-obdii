#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
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
const long interval = 10; // Interval for loop actions (10000 ms or 10 seconds)

// Define the size of each CAN frame (max 8 data bytes + overhead)
#define CAN_FRAME_SIZE 16 // Adjusted for overhead
#define BUFFER_SIZE 256 // Number of frames in the buffer

// Create a buffer to hold the CAN frames
byte canBuffer[BUFFER_SIZE][CAN_FRAME_SIZE];
int bufferIndex = 0;

// Global filename
String globalFileName;

String generateFileName() {
    unsigned long currentTime = millis();
    int randomNum = random(9999); // Add some randomness
    String fileName = "data" + String(randomNum) + ".dat";
    return fileName;
}

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

    // Generate a global filename for use in all file operations
    globalFileName = generateFileName();
    // Create an empty file to be appended to by writeToSDCard()
    File dataFile = SD.open(globalFileName.c_str(), FILE_WRITE);
    if (dataFile) {
        dataFile.close();  // Close the file immediately after creating it
        Serial.println(globalFileName + " created successfully.");
    } else {
        Serial.println("Error creating " + globalFileName);
        while (1); // Stop everything if no file
    }

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

bool receiveCANFrame(byte *frame) {
    byte dummyData[11] = {0x01, 0x23, 0x08, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}; // Example frame data
    memcpy(frame, dummyData, sizeof(dummyData));
    return true; // Indicate a frame has been 'received'
}

void writeToSDCard(byte buffer[][CAN_FRAME_SIZE], int numFrames) {
    File dataFile = SD.open(globalFileName.c_str(), FILE_WRITE);
    if (dataFile) {
        for (int i = 0; i < numFrames; i++) {
            dataFile.write(buffer[i], CAN_FRAME_SIZE);
        }
        dataFile.close();
        Serial.println("Data written to " + globalFileName);
    } else {
        Serial.println("Error opening " + globalFileName);
    }
}

void loop() {

    // Periodic actions every interval
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    
    // add frame to buffer
    if (receiveCANFrame(canBuffer[bufferIndex])) {
        bufferIndex++;
        //Serial.println("" + String(bufferIndex) + "<->" + String(BUFFER_SIZE));
        if (bufferIndex >= BUFFER_SIZE) {
            writeToSDCard(canBuffer, BUFFER_SIZE);
            bufferIndex = 0; // Reset buffer index
        }
    }

  }

    
}
