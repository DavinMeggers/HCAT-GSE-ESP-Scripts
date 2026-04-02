#include <ESP32Servo.h>

// ======================================================
// RC Servo Test for GSE Board
// ======================================================
// Reads 2 standard RC PWM inputs and drives 2 servos.
//
// RC receiver inputs:
//   CH1 -> GPIO1
//   CH2 -> GPIO2
//
// Servo outputs:
//   Servo 1 -> GPIO41
//   Servo 2 -> GPIO42
//
// Failsafe behavior:
//   If a valid RC pulse is lost, servo goes CLOSED
//
// User scale mapping:
//   user 0   -> servo 15 deg
//   user 90  -> servo 180 deg
//
// RC pulse mapping:
//   1000 us -> user 0
//   2000 us -> user 90
// ======================================================

// -----------------------------
// Pin assignments
// -----------------------------
const int RC1_INPUT_PIN = 1;   // IO1 on expansion connector
const int RC2_INPUT_PIN = 2;   // IO2 on expansion connector

const int SERVO1_PIN = 41;      // Servo 1 output
const int SERVO2_PIN = 42;      // Servo 2 output

// -----------------------------
// Servo settings
// -----------------------------
const int SERVO_MIN_US = 1000;
const int SERVO_MAX_US = 2000;
const int SERVO_FREQ_HZ = 50;

// -----------------------------
// Your required mapping
// -----------------------------
const int USER_MIN_DEG = 0;
const int USER_MAX_DEG = 90;

const int SERVO_OPEN_DEG = 15;
const int SERVO_CLOSED_DEG = 180;

// -----------------------------
// Standard RC PWM range
// -----------------------------
const int RC_MIN_US = 1000;
const int RC_MAX_US = 2000;

// Timeout for pulseIn()
// If no pulse is seen within this time, treat as signal loss
const unsigned long RC_TIMEOUT_US = 30000;  // 30 ms

Servo servo1;
Servo servo2;

// ------------------------------------------------------
// Map user command 0..90 to physical servo angle 15..180
// ------------------------------------------------------
int mapUserInputToServoAngle(int userDeg) {
  userDeg = constrain(userDeg, USER_MIN_DEG, USER_MAX_DEG);

  float fraction = float(userDeg - USER_MIN_DEG) / float(USER_MAX_DEG - USER_MIN_DEG);
  float servoAngle = SERVO_OPEN_DEG + fraction * float(SERVO_CLOSED_DEG - SERVO_OPEN_DEG);

  return int(servoAngle + 0.5f);
}

// ------------------------------------------------------
// Map RC pulse width 1000..2000 us to user scale 0..90
// ------------------------------------------------------
int pulseWidthToUserDeg(unsigned long pulseWidthUs) {
  pulseWidthUs = constrain(pulseWidthUs, RC_MIN_US, RC_MAX_US);

  float fraction = float(pulseWidthUs - RC_MIN_US) / float(RC_MAX_US - RC_MIN_US);
  float userDeg = USER_MIN_DEG + fraction * float(USER_MAX_DEG - USER_MIN_DEG);

  return int(userDeg + 0.5f);
}

// ------------------------------------------------------
// Read one RC PWM pulse in microseconds
// Returns 0 if no pulse is found before timeout
// ------------------------------------------------------
unsigned long readRcPulseUs(int pin) {
  return pulseIn(pin, HIGH, RC_TIMEOUT_US);
}

// ------------------------------------------------------
// Move servo to CLOSED position for failsafe
// ------------------------------------------------------
void setServoClosed(Servo &servo) {
  servo.write(SERVO_CLOSED_DEG);
}

// ------------------------------------------------------
// Setup
// ------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting 2-Channel RC Servo Test...");

  // Set RC input pins
  pinMode(RC1_INPUT_PIN, INPUT);
  pinMode(RC2_INPUT_PIN, INPUT);

  // Initialize servos
  servo1.setPeriodHertz(SERVO_FREQ_HZ);
  servo2.setPeriodHertz(SERVO_FREQ_HZ);

  servo1.attach(SERVO1_PIN, SERVO_MIN_US, SERVO_MAX_US);
  servo2.attach(SERVO2_PIN, SERVO_MIN_US, SERVO_MAX_US);

  // Startup failsafe: both closed
  setServoClosed(servo1);
  setServoClosed(servo2);

  Serial.print("RC1 input pin: ");
  Serial.println(RC1_INPUT_PIN);
  Serial.print("RC2 input pin: ");
  Serial.println(RC2_INPUT_PIN);
  Serial.print("Servo1 output pin: ");
  Serial.println(SERVO1_PIN);
  Serial.print("Servo2 output pin: ");
  Serial.println(SERVO2_PIN);

  Serial.println("Failsafe: CLOSED if signal is lost");
  Serial.println("1000 us -> user 0 -> servo 15 deg");
  Serial.println("2000 us -> user 90 -> servo 180 deg");
  Serial.println();
  Serial.println("rc1_us\trc2_us\tuser1\tuser2\tservo1_deg\tservo2_deg");
}

// ------------------------------------------------------
// Main loop
// ------------------------------------------------------
void loop() {
  unsigned long rc1Pulse = readRcPulseUs(RC1_INPUT_PIN);
  unsigned long rc2Pulse = readRcPulseUs(RC2_INPUT_PIN);

  int user1Deg = -1;
  int user2Deg = -1;
  int servo1Deg = SERVO_CLOSED_DEG;
  int servo2Deg = SERVO_CLOSED_DEG;

  // -------------------------
  // Channel 1 -> Servo 1
  // -------------------------
  if (rc1Pulse >= 900 && rc1Pulse <= 2200) {
    user1Deg = pulseWidthToUserDeg(rc1Pulse);
    servo1Deg = mapUserInputToServoAngle(user1Deg);
    servo1.write(servo1Deg);
  } else {
    setServoClosed(servo1);
  }

  // -------------------------
  // Channel 2 -> Servo 2
  // -------------------------
  if (rc2Pulse >= 900 && rc2Pulse <= 2200) {
    user2Deg = pulseWidthToUserDeg(rc2Pulse);
    servo2Deg = mapUserInputToServoAngle(user2Deg);
    servo2.write(servo2Deg);
  } else {
    setServoClosed(servo2);
  }

  // Debug output
  Serial.print(rc1Pulse);
  Serial.print('\t');
  Serial.print(rc2Pulse);
  Serial.print('\t');
  Serial.print(user1Deg);
  Serial.print('\t');
  Serial.print(user2Deg);
  Serial.print('\t');
  Serial.print(servo1Deg);
  Serial.print('\t');
  Serial.println(servo2Deg);

  delay(50);
}