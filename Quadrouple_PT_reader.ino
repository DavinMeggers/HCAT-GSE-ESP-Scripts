#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// =====================================================
// USER SETTINGS
// =====================================================

// Correct I2C pins for your board
const int I2C_SDA_PIN = 33;
const int I2C_SCL_PIN = 34;

// Set this to either 1000.0 or 1600.0 depending on your PT
const float PT_FULL_SCALE_PSI = 1600.0;

// PT output range:
// 0.5 V = 0 psi
// 4.5 V = full-scale psi
const float PT_MIN_VOLTAGE = 0.5;
const float PT_MAX_VOLTAGE = 4.5;

// ADS1115 default I2C address
const uint8_t ADS1115_ADDRESS = 0x48;

// =====================================================
// GLOBALS
// =====================================================

Adafruit_ADS1115 ads;

// =====================================================
// HELPER FUNCTION
// Converts voltage to pressure
// =====================================================
float voltageToPressure(float voltage, float fullScalePsi) {
  // Clamp voltage so noise or slight overshoot does not create nonsense values
  if (voltage < PT_MIN_VOLTAGE) voltage = PT_MIN_VOLTAGE;
  if (voltage > PT_MAX_VOLTAGE) voltage = PT_MAX_VOLTAGE;

  // Linear scaling:
  // 0.5 V -> 0 psi
  // 4.5 V -> fullScalePsi
  float pressure = ((voltage - PT_MIN_VOLTAGE) / (PT_MAX_VOLTAGE - PT_MIN_VOLTAGE)) * fullScalePsi;

  return pressure;
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("ESP32-S2 + ADS1115 Pressure Transducer Reader");
  Serial.println("Starting...");

  // Set custom I2C pins, then start the I2C bus
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  Wire.setClock(100000);  // 100 kHz standard I2C speed

  Serial.print("I2C SDA pin: ");
  Serial.println(I2C_SDA_PIN);
  Serial.print("I2C SCL pin: ");
  Serial.println(I2C_SCL_PIN);

  // Start ADS1115
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: ADS1115 not found.");
    Serial.println("Check power, ground, SDA/SCL wiring, and address.");
    while (1) {
      delay(1000);
    }
  }

  Serial.print("ADS1115 found at address 0x");
  Serial.println(ADS1115_ADDRESS, HEX);

  // Gain setting:
  // GAIN_ONE = +/-4.096 V full-scale in the library
  // This is a good fit for PT outputs near 0.5-4.5 V
  ads.setGain(GAIN_ONE);

  Serial.print("Pressure range set to 0-");
  Serial.print(PT_FULL_SCALE_PSI);
  Serial.println(" psi");

  Serial.println();
  Serial.println("Pressure (psi):");
  Serial.println("PT0\tPT1\tPT2\tPT3");
}

// =====================================================
// LOOP
// =====================================================
void loop() {
  // Read all 4 ADS1115 channels
  int16_t raw0 = ads.readADC_SingleEnded(0);
  int16_t raw1 = ads.readADC_SingleEnded(1);
  int16_t raw2 = ads.readADC_SingleEnded(2);
  int16_t raw3 = ads.readADC_SingleEnded(3);

  // Convert raw ADC values to volts
  float v0 = ads.computeVolts(raw0);
  float v1 = ads.computeVolts(raw1);
  float v2 = ads.computeVolts(raw2);
  float v3 = ads.computeVolts(raw3);

  // Convert volts to pressure
  float p0 = voltageToPressure(v0, PT_FULL_SCALE_PSI);
  float p1 = voltageToPressure(v1, PT_FULL_SCALE_PSI);
  float p2 = voltageToPressure(v2, PT_FULL_SCALE_PSI);
  float p3 = voltageToPressure(v3, PT_FULL_SCALE_PSI);

  // Print pressures as tab-separated columns
  Serial.print(p0, 1);
  Serial.print('\t');
  Serial.print(p1, 1);
  Serial.print('\t');
  Serial.print(p2, 1);
  Serial.print('\t');
  Serial.println(p3, 1);

  delay(250);
}