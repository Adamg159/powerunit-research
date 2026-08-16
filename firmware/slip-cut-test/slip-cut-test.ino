// Regen slip-cut bench test — runs the Driveline logic on real hardware.
//
// Two modes, both in this one sketch:
//
//   1. BOOT SELF-TEST (no hardware needed). Runs the sign-convention and
//      fail-closed scenarios and prints PASS/FAIL. Flash this to a bare board
//      with nothing attached and it still tells you the logic is sound.
//
//   2. LIVE MODE. Reads the two wheel Hall pickups and lets you type a crank
//      RPM over serial to stand in for the VESC until the UART link exists.
//      Spin a magnet-equipped wheel by hand, type a crank speed, and watch the
//      verdict change. This is how the thresholds get sanity-checked before
//      any current flows.
//
// Wiring (per the A3144 rules — power at 5 V, pull up to 3.3 V):
//   A3144 pin 1 (VCC) -> 5 V        A3144 pin 2 (GND) -> GND
//   FRONT sensor OUT  -> GPIO 4     10 k from GPIO 4  -> 3V3
//   REAR  sensor OUT  -> GPIO 5     10 k from GPIO 5  -> 3V3
// The A3144 is open-collector: the external pull-up to 3V3 is what keeps the
// swing at 0-3.3 V. Do NOT use a KY-003 breakout at 5 V — its onboard pull-up
// goes to its own VCC and would put 5 V on a GPIO.
//
// Build (Driveline.h lives outside the sketch folder, so pass it explicitly):
//   arduino-cli compile --fqbn esp32:esp32:esp32s3 \
//     --library firmware/libraries/Driveline firmware/slip-cut-test
//
// Serial: 115200. Type a number to set the simulated crank RPM; "t" re-runs
// the self-test.

#include <Driveline.h>

using namespace driveline;

const int FRONT_PIN = 4;
const int REAR_PIN = 5;

PulseTach frontTach(WHEEL_MAGNETS);
PulseTach rearTach(WHEEL_MAGNETS);
SlipMonitor slip;

void IRAM_ATTR onFront() { frontTach.onPulse(micros()); }
void IRAM_ATTR onRear() { rearTach.onPulse(micros()); }

float simCrankRpm = 0.0f;

// --- boot self-test --------------------------------------------------------
// Mirrors the desktop test's critical cases. Kept short deliberately: this is
// the subset where being wrong is expensive, not the full suite.

static int selfTestFailures = 0;

static void expect(bool ok, const char* what) {
  Serial.printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok) selfTestFailures++;
}

static float rearForCrank(float crank) { return crank / FINAL_DRIVE; }

static void runSelfTest() {
  selfTestFailures = 0;
  Serial.println("\n=== Driveline self-test ===");

  expect(fabsf(crankRpmFromErpm(32000.0f) - 16000.0f) < 0.01f,
         "32000 ERPM -> 16000 crank rpm (pole pairs = 2)");

  SlipMonitor s;
  SlipInputs in;
  in.rear_valid = in.front_valid = true;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;

  // Crank leading = engine driving = normal.
  in.crank_rpm = 14000.0f;
  uint32_t t = 10000;
  RegenVerdict v = s.update(t, in);
  for (int i = 0; i < 20 && v != RegenVerdict::Allow; ++i) { t += 50; v = s.update(t, in); }
  expect(v == RegenVerdict::Allow, "crank LEADING is not a cut");

  // Crank lagging = clutch slipping under regen = the mandated cut.
  in.crank_rpm = 10000.0f;
  expect(s.update(t + 10, in) == RegenVerdict::CutClutchSlip, "crank LAGGING cuts regen");

  // Recovery is not instant.
  in.crank_rpm = 12000.0f;
  expect(s.update(t + 20, in) == RegenVerdict::CutCooldown, "does not re-allow immediately");

  // Fail closed on a dead tach at speed.
  SlipMonitor f;
  SlipInputs g;
  g.crank_rpm = 12000.0f;
  g.rear_wheel_rpm = rearForCrank(12000.0f);
  g.front_wheel_rpm = g.rear_wheel_rpm;
  g.rear_valid = false; g.front_valid = true;
  expect(f.update(10000, g) == RegenVerdict::CutNoData, "dead tach at speed = CUT");

  Serial.printf("=== %s ===\n\n", selfTestFailures == 0 ? "PASS" : "FAIL");
}

// --- setup / loop ----------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(FRONT_PIN, INPUT);   // external 10 k pull-up to 3.3 V
  pinMode(REAR_PIN, INPUT);
  attachInterrupt(FRONT_PIN, onFront, FALLING);   // A3144 pulls low on detect
  attachInterrupt(REAR_PIN, onRear, FALLING);

  runSelfTest();

  Serial.println("Live mode. Type a crank RPM (e.g. 12000) then Enter; 't' re-runs self-test.");
  Serial.printf("Config: final drive %.2f, engagement %.0f rpm, %u magnets/wheel\n",
                FINAL_DRIVE, slip.config().engagement_rpm, (unsigned)WHEEL_MAGNETS);
}

void loop() {
  // Serial input: a number sets the simulated crank speed, 't' re-tests.
  static String buf;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      buf.trim();
      if (buf == "t") runSelfTest();
      else if (buf.length()) simCrankRpm = buf.toFloat();
      buf = "";
    } else if (buf.length() < 12) {
      buf += c;
    }
  }

  uint32_t us = micros();
  SlipInputs in;
  in.crank_rpm = simCrankRpm;              // replace with crankRpmFromErpm(vesc.erpm)
  in.front_wheel_rpm = frontTach.rpm(us);
  in.rear_wheel_rpm = rearTach.rpm(us);
  in.front_valid = frontTach.valid(us);
  in.rear_valid = rearTach.valid(us);

  RegenVerdict v = slip.update(millis(), in);

  static uint32_t last = 0;
  if (millis() - last > 250) {
    last = millis();
    Serial.printf("crank %6.0f | rear %6.0f (expect crank %6.0f) | front %6.0f | "
                  "clutch slip %+7.0f | tire %+5.1f%% | %s\n",
                  in.crank_rpm, in.rear_wheel_rpm, slip.expectedCrankRpm(),
                  in.front_wheel_rpm, slip.clutchSlipRpm(),
                  slip.tireSlipRatio() * 100.0f, verdictName(v));
  }
  delay(10);
}
