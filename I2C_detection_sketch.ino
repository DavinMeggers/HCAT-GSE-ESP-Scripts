#include <Wire.h>

// --------------------------------------------------
// Pin definitions for your ESP32-S2 board
// --------------------------------------------------
const int I2C_SDA_PIN = 33;
const int I2C_SCL_PIN = 34;

// --------------------------------------------------
// setup() runs once when the board powers up or resets
// --------------------------------------------------
void setup() {
  // Start the serial monitor connection
  // Make sure Serial Monitor is also set to 115200 baud
  Serial.begin(115200);

  // Small delay so the serial monitor has time to connect
  delay(1500);

  Serial.println();
  Serial.println("ESP32-S2 I2C Scanner Starting...");
  Serial.print("Using SDA pin: ");
  Serial.println(I2C_SDA_PIN);
  Serial.print("Using SCL pin: ");
  Serial.println(I2C_SCL_PIN);

  // Start the I2C bus on the chosen SDA and SCL pins
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Optional: set I2C clock speed to standard 100 kHz
  Wire.setClock(100000);

  Serial.println("Scanning I2C bus for devices...");
  Serial.println("--------------------------------");
  
  int devicesFound = 0;

  // I2C addresses go from 0x08 to 0x77 in normal use
  for (uint8_t address = 8; address <= 119; address++) {
    Wire.beginTransmission(address);

    // endTransmission() returns:
    // 0 = success, meaning a device answered
    // anything else = no device or an error
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at I2C address: 0x");

      // Print leading zero for addresses less than 0x10
      if (address < 16) {
        Serial.print("0");
      }

      Serial.println(address, HEX);
      devicesFound++;
    }
  }

  Serial.println("--------------------------------");

  if (devicesFound == 0) {
    Serial.println("No I2C devices were found.");
    Serial.println("Check wiring, pull-up resistors, and ADC power.");
  } else {
    Serial.print("Scan complete. Number of devices found: ");
    Serial.println(devicesFound);
  }
}

// --------------------------------------------------
// loop() runs repeatedly after setup()
// We leave it empty because we only want to scan once
// --------------------------------------------------
void loop() {
  // Nothing here
}