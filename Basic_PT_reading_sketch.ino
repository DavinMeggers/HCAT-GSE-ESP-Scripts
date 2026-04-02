#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// --------------------------------------------------
// ESP32-S2 I2C pin assignments on your board
// --------------------------------------------------
const int I2C_SDA_PIN = 33;
const int I2C_SCL_PIN = 34;

// --------------------------------------------------
// Create an ADS1115 object
// --------------------------------------------------
Adafruit_ADS1115 ads;

// --------------------------------------------------
// ADS1115 I2C address
// Most ADS1115 boards default to 0x48 unless ADDR is tied differently
// --------------------------------------------------
const uint8_t ADS1115_ADDRESS = 0x48;

// --------------------------------------------------
// Pressure transducer is connected to AIN1
// Channels are:
// 0 = AIN0
// 1 = AIN1
// 2 = AIN2
// 3 = AIN3
// --------------------------------------------------
const int PT_CHANNEL = 0;

// --------------------------------------------------
// setup() runs once at startup
// --------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("ESP32-S2 + ADS1115 Pressure Transducer Reader");
  Serial.println("Starting I2C...");

  // Start I2C on your custom pins
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);  // standard I2C speed

  // Try to start the ADS1115
  // If this fails, the chip is not responding at this address
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: ADS1115 not found.");
    Serial.println("Check power, ground, SDA/SCL wiring, and I2C address.");
    while (1) {
      delay(1000);
    }
  }

  Serial.print("ADS1115 found at address 0x");
  Serial.println(ADS1115_ADDRESS, HEX);

  // Gain setting:
  // GAIN_ONE gives a full-scale range of +/-4.096 V
  // This is a good safe starting point for many 0-3.3V signals
  ads.setGain(GAIN_ONE);

  Serial.println("Reading from AIN1...");
  Serial.println("RawCounts\tVoltage(V)");
}

// --------------------------------------------------
// loop() runs over and over
// --------------------------------------------------
void loop() {
  // Read the raw ADC value from single-ended channel 1 (AIN1)
  int16_t raw = ads.readADC_SingleEnded(PT_CHANNEL);

  // Convert raw ADC counts to volts
  // computeVolts() is provided by the Adafruit library
  float voltage = ads.computeVolts(raw);

  // Print both the raw ADC reading and the calculated voltage
  Serial.print(raw);
  Serial.print("\t\t");
  Serial.println(voltage, 6);

  delay(500);
}