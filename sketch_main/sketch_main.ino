// For Arduino Mega 2650 with 16x2 I2C LCD and W5100 Ethernet shield with SD card adapter
#include <Wire.h>  // Standard library
#include <LiquidCrystal_I2C.h>  //In libraries folder

// connect SDA (green)-> pin 20 and SCL (yellow)-> pin 21.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD's I2C

#include <OBD2UART.h> //In libraries folder

COBD obd;

void setup() {
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Freematics OBD-2");
  
  Serial1.begin(115200); // Arduino pin 18->white wire and pin 19->green wire for OBD communication
  while (!Serial1); // Wait for Serial port to connect
  
  Serial.begin(115200); // Keep Serial for debugging to the computer

  // Check for the presence of the OBD-II adapter and loop until detected
  for (;;) {
    
    delay(2000);
    byte version = obd.begin();

    if (version > 0) {
      lcd.setCursor(0,1);
      lcd.print("OBD-2 adapter OK");

      Serial.println("OBD-2 adapter OK");
      break;
    } else {

      lcd.setCursor(0,1);
      lcd.print("                ");
      delay(150);
      lcd.setCursor(0,1);
      lcd.print("No OBD-2 adapter");

      Serial.println("No OBD-2 adapter");
      Serial.println(version);
    }
  }

  // Initialize MEMS with sensor fusion enabled
  bool hasMEMS = obd.memsInit(true);

  lcd.setCursor(0,1);
  lcd.print("                ");
  delay(150);

  if (hasMEMS) {
    lcd.setCursor(0,1);
    lcd.print("Motion rdy");
    Serial.println("Motion rdy");
  }
  
  // If no MEMS sensor, hang the setup
  if (!hasMEMS) {
    lcd.setCursor(0,1);
    lcd.print("Motion rdy");
    Serial.println("No motion");

    for (;;) delay(1000);
  }
}

void loop() {
  int16_t acc[3] = {0};
  int16_t gyro[3] = {0};
  int16_t mag[3] = {0};

  // Read MEMS sensor data, skip loop iteration if read fails
  if (!obd.memsRead(acc, gyro, mag)) return;
  
  // Print accelerometer data
  Serial.print("ACC:");
  Serial.print(acc[0]);
  Serial.print('/');
  Serial.print(acc[1]);
  Serial.print('/');
  Serial.print(acc[2]);

  lcd.setCursor(0,1);
  lcd.print("                ");
  delay(100);
  lcd.print(String(acc[0]) + "/" + String(acc[1]) + "/" + String(acc[2]));

  // Print gyroscope data
  Serial.print(" GYRO:");
  Serial.print(gyro[0]);
  Serial.print('/');
  Serial.print(gyro[1]);
  Serial.print('/');
  Serial.print(gyro[2]);

  // Print magnetometer data
  Serial.print(" MAG:");
  Serial.print(mag[0]);
  Serial.print('/');
  Serial.print(mag[1]);
  Serial.print('/');
  Serial.print(mag[2]);
  Serial.println();

  // Compute and print orientation data if available
  float yaw, pitch, roll;
  if (obd.memsOrientation(yaw, pitch, roll)) {
    Serial.print("Orientation: ");
    Serial.print(yaw, 2);
    Serial.print(' ');
    Serial.print(pitch, 2);
    Serial.print(' ');
    Serial.println(roll, 2);
  }

  delay(100);
}