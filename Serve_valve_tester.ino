#include <ESP32Servo.h>

// --------------------------------------
// Servo settings
// --------------------------------------
const int SERVO_PIN = 45;

Servo myServo;

int inputAngle = 90;   // default starting position

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("Servo Serial Control Ready");
  Serial.println("Enter angle between 0 and 180:");

  // Configure servo
  myServo.setPeriodHertz(50);        // Standard servo frequency
  myServo.attach(SERVO_PIN, 1000, 2000);

  // Start centered
  myServo.write(inputAngle);
}

void loop() {

  // Check if user typed something
  if (Serial.available() > 0) {

    // Read the angle as an integer
    int angle = Serial.parseInt();

    // Make sure angle is valid
    inputAngle = angle;
    myServo.write(inputAngle);

    Serial.print("Servo moved to: ");
    Serial.print(inputAngle);
    Serial.println(" degrees");


    // Clear leftover serial characters
    while (Serial.available()) {
      Serial.read();
    }
  }
}