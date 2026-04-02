#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MAX31855.h>
#include <ESP32Servo.h>

// ======================================================
// ManualGSESystemsTest
// ======================================================
// Features:
// 1. Reads 4 pressure transducers from ADS1115 channels AIN0-AIN3
// 2. Converts PT voltages (0.5V to 4.5V) into PSI
// 3. Reads 2 thermocouples using MAX31855 over SPI
// 4. Controls up to 3 servos from Serial using:
//      @S[num] [degree]
//    Example:
//      @S2 50
// 5. Input degree range is 0..90, internally scaled so:
//      0  -> 0 servo degrees
//      90 -> 180 servo degrees
// ======================================================

// =========================
// USER-SET CONSTANTS
// =========================

// ---------- I2C for ADS1115 ----------
const int I2C_SDA_PIN = 33;
const int I2C_SCL_PIN = 34;
const uint8_t ADS1115_ADDRESS = 0x48;   // change if your ADDR pin is different

// ---------- Pressure transducer config ----------
// PT output is 0.5V to 4.5V
const float PT_MIN_VOLTAGE = 0.5f;
const float PT_MAX_VOLTAGE = 4.5f;

// Set default PT range here. Can also be changed at runtime with:
// @PRANGE 1000
// @PRANGE 1600
float ptFullScalePsi = 1000.0f;

// ---------- MAX31855 shared SPI pins ----------
const int TC_SCK_PIN  = 18;
const int TC_MISO_PIN = 16;

// IMPORTANT: Replace these with the real chip-select pins for your board
const int TC1_CS_PIN = 17;   // TODO: replace with real CS pin for thermocouple 1
const int TC2_CS_PIN = 21;   // TODO: replace with real CS pin for thermocouple 2

// ---------- Servo pins ----------
// IMPORTANT: Replace these with the real servo GPIO pins on your board.
// I only know one earlier servo test used pin 41, so that is preserved for Servo 1.
const int NUM_SERVOS = 3;
const int servoPins[NUM_SERVOS] = {
  41,   // Servo 1
  42,   // TODO: replace if different
  45,   // TODO: replace if different
};

// Servo pulse range
const int SERVO_MIN_US = 1000;
const int SERVO_MAX_US = 2000;
const int SERVO_FREQ_HZ = 50;

// User input range and physical servo mapping
const int USER_MIN_DEG = 0;
const int USER_MAX_DEG = 90;
const int SERVO_OPEN_DEG = 0;
const int SERVO_CLOSED_DEG = 180;

// ---------- Telemetry timing ----------
const unsigned long TELEMETRY_INTERVAL_MS = 500;

// =========================
// GLOBAL OBJECTS
// =========================
Adafruit_ADS1115 ads;

// MAX31855 uses SCK, CS, MISO
Adafruit_MAX31855 thermocouple1(TC_SCK_PIN, TC1_CS_PIN, TC_MISO_PIN);
Adafruit_MAX31855 thermocouple2(TC_SCK_PIN, TC2_CS_PIN, TC_MISO_PIN);

Servo servos[NUM_SERVOS];
int lastUserServoCommand[NUM_SERVOS] = {0, 0, 0};   // stores last user-entered 0..90 values

unsigned long lastTelemetryTime = 0;

// =========================
// HELPER FUNCTIONS
// =========================

// Convert PT voltage into PSI using:
// 0.5V -> 0 psi
// 4.5V -> full-scale psi
float voltageToPsi(float voltage, float fullScalePsi) {
  // Clamp to prevent noise from making negative or overrange values
  if (voltage < PT_MIN_VOLTAGE) voltage = PT_MIN_VOLTAGE;
  if (voltage > PT_MAX_VOLTAGE) voltage = PT_MAX_VOLTAGE;

  float psi = ((voltage - PT_MIN_VOLTAGE) / (PT_MAX_VOLTAGE - PT_MIN_VOLTAGE)) * fullScalePsi;
  return psi;
}

// Map user input 0..90 to actual servo angle 15..180
int mapUserInputToServoAngle(int userDeg) {
  userDeg = constrain(userDeg, USER_MIN_DEG, USER_MAX_DEG);

  float fraction = float(userDeg - USER_MIN_DEG) / float(USER_MAX_DEG - USER_MIN_DEG);
  float servoAngle = SERVO_OPEN_DEG + fraction * float(SERVO_CLOSED_DEG - SERVO_OPEN_DEG);

  return int(servoAngle + 0.5f); // rounded
}

// Print a help menu
void printHelp() {
  Serial.println();
  Serial.println("========== ManualGSESystemsTest Commands ==========");
  Serial.println("@S1 45        -> Move servo 1 using 0..90 input");
  Serial.println("@S2 50        -> Move servo 2 using 0..90 input");
  Serial.println("@S3 10        -> Move servo 3 using 0..90 input");
  Serial.println("@PRANGE 1000  -> Set PT full-scale range to 1000 psi");
  Serial.println("@PRANGE 1600  -> Set PT full-scale range to 1600 psi");
  Serial.println("@STATUS       -> Print one telemetry snapshot now");
  Serial.println("@HELP         -> Show this help menu");
  Serial.println("==================================================");
  Serial.println();
}

// Read one ADS1115 channel and return voltage
float readPressureVoltage(int channel) {
  int16_t raw = ads.readADC_SingleEnded(channel);
  return ads.computeVolts(raw);
}

// Read one ADS1115 channel and return PSI
float readPressurePsi(int channel) {
  float voltage = readPressureVoltage(channel);
  return voltageToPsi(voltage, ptFullScalePsi);
}

// Print one telemetry snapshot
void printTelemetry() {
  float ptVoltage[4];
  float ptPsi[4];

  for (int i = 0; i < 4; i++) {
    ptVoltage[i] = readPressureVoltage(i);
    ptPsi[i] = voltageToPsi(ptVoltage[i], ptFullScalePsi);
  }

  double tc1 = thermocouple1.readCelsius();
  double tc2 = thermocouple2.readCelsius();

  Serial.println("------------------------------------------------------------");
  Serial.println("ManualGSESystemsTest");
  Serial.print("PT Range Setting: ");
  Serial.print(ptFullScalePsi, 0);
  Serial.println(" psi");

  Serial.println("PRESSURE TRANSDUCERS:");
  Serial.println("PT1(AIN0)\tPT2(AIN1)\tPT3(AIN2)\tPT4(AIN3)");
  Serial.print(ptPsi[0], 1); Serial.print(" psi\t");
  Serial.print(ptPsi[1], 1); Serial.print(" psi\t");
  Serial.print(ptPsi[2], 1); Serial.print(" psi\t");
  Serial.print(ptPsi[3], 1); Serial.println(" psi");

  Serial.println("PT VOLTAGES:");
  Serial.print(ptVoltage[0], 3); Serial.print(" V\t");
  Serial.print(ptVoltage[1], 3); Serial.print(" V\t");
  Serial.print(ptVoltage[2], 3); Serial.print(" V\t");
  Serial.print(ptVoltage[3], 3); Serial.println(" V");

  Serial.println("THERMOCOUPLES:");
  Serial.print("TC1: ");
  if (isnan(tc1)) Serial.print("FAULT");
  else Serial.print(tc1, 2);
  Serial.print(" C\t");

  Serial.print("TC2: ");
  if (isnan(tc2)) Serial.print("FAULT");
  else Serial.print(tc2, 2);
  Serial.println(" C");

  Serial.println("SERVO COMMAND STATE (user scale 0..90):");
  Serial.print("S1="); Serial.print(lastUserServoCommand[0]); Serial.print("\t");
  Serial.print("S2="); Serial.print(lastUserServoCommand[1]); Serial.print("\t");
  Serial.print("S3="); Serial.print(lastUserServoCommand[2]); Serial.print("\t");

  Serial.println("------------------------------------------------------------");
}

// Parse commands like:
// @S2 50
// @PRANGE 1600
// @STATUS
// @HELP
void handleCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) return;

  if (cmd.equalsIgnoreCase("@HELP")) {
    printHelp();
    return;
  }

  if (cmd.equalsIgnoreCase("@STATUS")) {
    printTelemetry();
    return;
  }

  if (cmd.startsWith("@PRANGE")) {
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex < 0) {
      Serial.println("ERROR: Use @PRANGE 1000 or @PRANGE 1600");
      return;
    }

    String valueStr = cmd.substring(spaceIndex + 1);
    valueStr.trim();
    int value = valueStr.toInt();

    if (value == 1000 || value == 1600) {
      ptFullScalePsi = float(value);
      Serial.print("PT full-scale range set to ");
      Serial.print(ptFullScalePsi, 0);
      Serial.println(" psi");
    } else {
      Serial.println("ERROR: PT range must be 1000 or 1600");
    }
    return;
  }

  // Servo command format: @S[num] [degree]
  // Examples:
  // @S1 0
  // @S2 50
  // @S4 90
  if (cmd.startsWith("@S")) {
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex < 0) {
      Serial.println("ERROR: Servo command format is @S[num] [degree]");
      return;
    }

    String servoPart = cmd.substring(2, spaceIndex);   // characters after @S up to space
    String degreePart = cmd.substring(spaceIndex + 1);

    servoPart.trim();
    degreePart.trim();

    int servoNum = servoPart.toInt();    // expects 1..4
    int userDeg = degreePart.toInt();    // expects 0..90

    if (servoNum < 1 || servoNum > NUM_SERVOS) {
      Serial.println("ERROR: Servo number out of range. Use 1 to 4.");
      return;
    }

    if (userDeg < USER_MIN_DEG || userDeg > USER_MAX_DEG) {
      Serial.println("ERROR: Servo degree must be between 0 and 90.");
      return;
    }

    int servoIndex = servoNum - 1;
    int actualServoAngle = mapUserInputToServoAngle(userDeg);

    servos[servoIndex].write(actualServoAngle);
    lastUserServoCommand[servoIndex] = userDeg;

    Serial.print("Servo ");
    Serial.print(servoNum);
    Serial.print(" set to user input ");
    Serial.print(userDeg);
    Serial.print(" deg -> actual servo angle ");
    Serial.print(actualServoAngle);
    Serial.println(" deg");
    return;
  }

  Serial.println("ERROR: Unknown command. Type @HELP");
}

// Read serial line-by-line
void checkSerialCommands() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);

    // flush any leftover CR/LF chars
    while (Serial.available() > 0) {
      char c = Serial.peek();
      if (c == '\r' || c == '\n') Serial.read();
      else break;
    }
  }
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("Starting ManualGSESystemsTest...");

  // ---------- I2C init ----------
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: ADS1115 not found.");
    Serial.println("Check power, GND, SDA/SCL, and I2C address.");
    while (1) {
      delay(1000);
    }
  }

  // Gain selection:
  // GAIN_ONE = +/-4.096V full scale
  // This is close to your 0.5V..4.5V PT range.
  // NOTE: if your PT truly reaches 4.5V, readings near the top may clip slightly.
  ads.setGain(GAIN_ONE);

  Serial.println("ADS1115 initialized.");

  // ---------- Servo init ----------
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].setPeriodHertz(SERVO_FREQ_HZ);
    servos[i].attach(servoPins[i], SERVO_MIN_US, SERVO_MAX_US);

    // Start at user input 0 => actual 15 degrees
    lastUserServoCommand[i] = 0;
    servos[i].write(mapUserInputToServoAngle(0));
  }

  Serial.println("Servos initialized.");

  // ---------- Thermocouple init ----------
  // MAX31855 doesn't have much explicit begin/init in this library style,
  // so we just read later. If CS pins are wrong, you'll get FAULT / NaN.
  Serial.println("MAX31855 objects created.");

  printHelp();
  printTelemetry();
}

// =========================
// LOOP
// =========================
void loop() {
  checkSerialCommands();

  unsigned long now = millis();
  if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryTime = now;
    printTelemetry();
  }
}