// =============================================
// RC Signal Generator with Debug Output
// Pins:
//   D3 -> Channel 1
//   D5 -> Channel 2
// =============================================

const int CH1_PIN = 3;
const int CH2_PIN = 5;

// Pulse widths (microseconds)
const int PULSE_OPEN_US  = 1000;  // "open"
const int PULSE_CLOSE_US = 2000;  // "closed"

// 20 ms period (50 Hz)
const int FRAME_PERIOD_US = 20000;

// Duration for each state (1 second)
const unsigned long STATE_DURATION_MS = 1000;

unsigned long lastSwitchTime = 0;
bool state = false;  // false = CH1 open, CH2 closed

// ----------------------------------------
// Send one PWM pulse on a pin
// ----------------------------------------
void sendPulse(int pin, int pulseWidthUs) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(pulseWidthUs);
  digitalWrite(pin, LOW);

  delayMicroseconds(FRAME_PERIOD_US - pulseWidthUs);
}

void setup() {
  Serial.begin(115200);

  pinMode(CH1_PIN, OUTPUT);
  pinMode(CH2_PIN, OUTPUT);

  digitalWrite(CH1_PIN, LOW);
  digitalWrite(CH2_PIN, LOW);

  Serial.println("RC Signal Generator Started");
}

void loop() {

  // Check if it's time to switch states
  if (millis() - lastSwitchTime >= STATE_DURATION_MS) {
    state = !state;
    lastSwitchTime = millis();

    if (state == false) {
      Serial.println("STATE: Servo 1 OPEN, Servo 2 CLOSED");
    } else {
      Serial.println("STATE: Servo 1 CLOSED, Servo 2 OPEN");
    }
  }

  // Continuously generate PWM signals
  if (state == false) {
    // CH1 OPEN, CH2 CLOSED
    sendPulse(CH1_PIN, PULSE_OPEN_US);
    sendPulse(CH2_PIN, PULSE_CLOSE_US);
  } else {
    // CH1 CLOSED, CH2 OPEN
    sendPulse(CH1_PIN, PULSE_CLOSE_US);
    sendPulse(CH2_PIN, PULSE_OPEN_US);
  }
}