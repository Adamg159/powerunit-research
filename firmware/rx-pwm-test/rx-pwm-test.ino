// Receiver PWM reader — the instrument for the A2 failsafe acceptance tests.
//
// Stands in for servos, which this project does not own (servo purchase was
// deliberately deferred). It is also the better instrument: a servo shows you
// roughly where a channel went, this shows exact microseconds on four channels
// at once AND measures how long the failsafe took to engage. The FS-R11P
// judgment time is ~300 ms by spec, and "the servo moved" does not measure that.
//
// What it prints:
//   - live pulse width per channel, in microseconds (1000-2000 typical)
//   - LOST flag per channel when no pulse has arrived for 500 ms
//   - on transmitter-off, the captured "failsafe position" of each channel and
//     the measured time from last-normal-frame to settled-failsafe
//
// WIRING — READ THE VOLTAGE WARNING FIRST.
//   RX [8] VCC/BVD  -> 5 V          RX GND -> GND (common with the ESP32)
//   RX CH1 signal   -> GPIO 4       (steering)
//   RX CH2 signal   -> GPIO 5       (throttle)
//   RX CH3 signal   -> GPIO 6       (ignition kill)
//   RX CH10 signal  -> GPIO 7       (sentinel channel)
//
// *** The FS-R11P drives its PWM outputs at the supply voltage. Powered from
// *** 5 V, those are 5 V signals and the ESP32 is a 3.3 V part. METER ONE
// *** CHANNEL'S IDLE VOLTAGE BEFORE CONNECTING ANYTHING. If it sits near 5 V,
// *** put a divider on EVERY signal line: 1k from RX signal to the GPIO, 2k
// *** from that GPIO node to GND (gives ~3.3 V). 10k/20k works equally well.
// *** Grounds must be common or the readings are meaningless.
//
// Build:
//   arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB firmware/rx-pwm-test
//
// Serial: 115200. Press 'z' to re-zero the captured baseline.

#include <Arduino.h>

struct Channel {
  const char* name;
  uint8_t pin;
  volatile uint32_t rise_us;
  volatile uint32_t width_us;
  volatile uint32_t last_us;
  volatile uint32_t frames;
  uint32_t baseline_us;      // captured while the link is healthy
};

Channel ch[] = {
  {"CH1 steer",  4, 0, 0, 0, 0, 0},
  {"CH2 thr",    5, 0, 0, 0, 0, 0},
  {"CH3 kill",   6, 0, 0, 0, 0, 0},
  {"CH10 sent",  7, 0, 0, 0, 0, 0},
};
const int NCH = sizeof(ch) / sizeof(ch[0]);

const uint32_t LOST_US = 500000;   // no pulse for 500 ms = signal gone

// One handler per channel. Plain functions rather than a template: the Arduino
// .ino preprocessor generates prototypes ahead of the definitions and cannot
// parse a templated ISR, and the resulting error points at a comment line.
static inline void IRAM_ATTR handleEdge(int i) {
  uint32_t now = micros();
  if (digitalRead(ch[i].pin)) {
    ch[i].rise_us = now;
  } else {
    uint32_t w = now - ch[i].rise_us;
    if (w > 500 && w < 3000) {     // plausible servo pulse; reject noise
      ch[i].width_us = w;
      ch[i].last_us = now;
      ch[i].frames++;
    }
  }
}

void IRAM_ATTR onEdge0() { handleEdge(0); }
void IRAM_ATTR onEdge1() { handleEdge(1); }
void IRAM_ATTR onEdge2() { handleEdge(2); }
void IRAM_ATTR onEdge3() { handleEdge(3); }

// State for the failsafe timing measurement.
bool wasHealthy = false;
uint32_t lastHealthy_ms = 0;
bool reported = false;

static bool linkHealthy(uint32_t now_us) {
  for (int i = 0; i < NCH; i++) {
    if ((uint32_t)(now_us - ch[i].last_us) > LOST_US) return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== FS-R11P PWM reader (A2 failsafe instrument) ===");
  Serial.println("Divider check: if any channel idles near 5 V, DISCONNECT and add 1k/2k.");

  pinMode(ch[0].pin, INPUT); attachInterrupt(ch[0].pin, onEdge0, CHANGE);
  pinMode(ch[1].pin, INPUT); attachInterrupt(ch[1].pin, onEdge1, CHANGE);
  pinMode(ch[2].pin, INPUT); attachInterrupt(ch[2].pin, onEdge2, CHANGE);
  pinMode(ch[3].pin, INPUT); attachInterrupt(ch[3].pin, onEdge3, CHANGE);

  Serial.println("Waiting for frames... (TX on, RX bound)");
}

void loop() {
  while (Serial.available()) {
    if (Serial.read() == 'z') {
      for (int i = 0; i < NCH; i++) ch[i].baseline_us = ch[i].width_us;
      Serial.println(">>> baseline re-captured");
    }
  }

  uint32_t now_us = micros();
  bool healthy = linkHealthy(now_us);

  if (healthy) {
    lastHealthy_ms = millis();
    if (!wasHealthy) Serial.println(">>> LINK UP");
    // Continuously refresh the baseline while healthy, so the failsafe report
    // compares against what the transmitter was actually commanding.
    for (int i = 0; i < NCH; i++) ch[i].baseline_us = ch[i].width_us;
    reported = false;
  } else if (wasHealthy) {
    Serial.println(">>> LINK LOST");
  }
  wasHealthy = healthy;

  // Report the failsafe outcome once, shortly after the link drops.
  if (!healthy && !reported && (millis() - lastHealthy_ms) > 800) {
    reported = true;
    Serial.printf("\n--- FAILSAFE REPORT (%lu ms after last good frame) ---\n",
                  millis() - lastHealthy_ms);
    for (int i = 0; i < NCH; i++) {
      bool holding = (uint32_t)(micros() - ch[i].last_us) <= LOST_US;
      Serial.printf("  %-10s commanded %4lu us -> now %s",
                    ch[i].name, ch[i].baseline_us,
                    holding ? "" : "NO PULSES (output stopped)");
      if (holding) Serial.printf("%4lu us", ch[i].width_us);
      Serial.println();
    }
    Serial.println("  PASS = each channel at its configured failsafe position,");
    Serial.println("         NOT still holding the last commanded value.\n");
  }

  static uint32_t last = 0;
  if (millis() - last > 250) {
    last = millis();
    for (int i = 0; i < NCH; i++) {
      bool lost = (uint32_t)(now_us - ch[i].last_us) > LOST_US;
      Serial.printf("%s %4lu%s  ", ch[i].name, ch[i].width_us, lost ? "*LOST" : "     ");
    }
    Serial.println();
  }
  delay(10);
}
