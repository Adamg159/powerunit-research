// A3144 Hall sensor arrival test — breadboard, no soldering needed.
// Counts pulses with an interrupt and mirrors the sensor on the onboard LED.
//
// Wiring (ESP32 DevKit + breadboard):
//   A3144 pin 1 (VCC, flat face toward you, left)  -> VIN (5 V)
//   A3144 pin 2 (GND, middle)                      -> GND
//   A3144 pin 3 (OUT, right)                       -> GPIO 4
//   10 k resistor (owned stock) from GPIO 4        -> 3V3   <-- pull-up; the A3144 is
//                                                       open-collector and only pulls LOW
//
// The A3144 is UNIPOLAR: only one magnetic pole on the branded face triggers it.
// Test each magnet BOTH ways; the working face is the one that lights the LED.
// Works with any magnet for the sensor test itself (fridge magnet, speaker magnet);
// when the SmCo discs arrive, use this rig to find and paint-mark each disc's working face.
//
// PASS looks like: LED on + "DETECT" while the right pole is near, clean single count
// per approach (no bounce storms), count increments reliably.

const int HALL_PIN = 4;
const int LED_PIN = 2;

volatile uint32_t pulses = 0;
void IRAM_ATTR onPulse() { pulses++; }

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(HALL_PIN, INPUT);      // external 10 k pull-up to 3.3 V per the wiring above
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(HALL_PIN, onPulse, FALLING);   // A3144 pulls low on detect
  Serial.println("\n=== A3144 Hall sensor arrival test ===");
  Serial.println("Bring a magnet near the branded face; try both poles.");
}

void loop() {
  bool detect = digitalRead(HALL_PIN) == LOW;
  digitalWrite(LED_PIN, detect);
  static uint32_t last = 0;
  static bool lastDetect = false;
  if (detect != lastDetect || millis() - last > 2000) {
    Serial.printf("%s  pulses=%lu\n", detect ? "DETECT" : "clear ", pulses);
    lastDetect = detect;
    last = millis();
  }
  delay(20);
}
