// Desktop self-test for Driveline.h — no hardware, no Arduino, no clock.
//
// Build and run:
//   g++ -std=c++17 -Wall -Wextra -I.. -o test_driveline test_driveline.cpp && ./test_driveline
//
// The same scenario table runs on the ESP32 via firmware/slip-cut-test.
// Every case is written as "what the vehicle is physically doing" first,
// because the failure being guarded against is a sign error, and sign errors
// look perfectly reasonable in code.

#include "../Driveline.h"

#include <cstdio>
#include <cmath>
#include <initializer_list>

using namespace driveline;

static int failures = 0;
static int checks = 0;

static void check(bool ok, const char* what) {
  ++checks;
  if (!ok) { ++failures; printf("  FAIL  %s\n", what); }
  else printf("  ok    %s\n", what);
}

static void checkNear(float got, float want, float tol, const char* what) {
  ++checks;
  if (std::fabs(got - want) > tol) {
    ++failures;
    printf("  FAIL  %s (got %.2f, want %.2f +/- %.2f)\n", what, got, want, tol);
  } else {
    printf("  ok    %s (%.2f)\n", what, got);
  }
}

// ---------------------------------------------------------------------------

static void testErpmConversion() {
  printf("\n[ERPM conversion]\n");
  // QuicRun 3650SD G2 is a 2-POLE motor (manual spec table) = 1 pole pair,
  // so ERPM == mechanical RPM on this motor.
  checkNear(MOTOR_POLE_PAIRS, 1.0f, 0.001f, "2 poles => 1 pole pair");
  checkNear(crankRpmFromErpm(16000.0f), 16000.0f, 0.01f, "16000 ERPM -> 16000 crank rpm");
  checkNear(erpmFromCrankRpm(16000.0f), 16000.0f, 0.01f, "round-trips");

  // Guard the general case so a future motor swap cannot silently break this:
  // whatever the pole count, the two conversions must be exact inverses.
  for (float rpm : {1000.0f, 7500.0f, 16000.0f}) {
    checkNear(crankRpmFromErpm(erpmFromCrankRpm(rpm)), rpm, 0.01f,
              "conversion round-trips exactly");
  }
}

static void testSpeed() {
  printf("\n[wheel speed]\n");
  // 40 km/h = 11.11 m/s on a 63 mm tire => 3369 wheel rpm.
  checkNear(wheelRpmToSpeedMps(3369.0f), 11.11f, 0.05f, "3369 wheel rpm -> 11.1 m/s");
  // And the ratio should put the crank near 16,000 there.
  checkNear(3369.0f * FINAL_DRIVE, 16677.0f, 200.0f, "target ratio lands near 16k crank");
}

static void testPulseTach() {
  printf("\n[PulseTach]\n");
  PulseTach t(4);
  // 4 magnets, 4500 us between pulses => 1 rev per 18 ms => 3333 rpm.
  uint32_t now = 1000;
  for (int i = 0; i < 10; ++i) { t.onPulse(now); now += 4500; }
  checkNear(t.rpm(now), 3333.0f, 30.0f, "steady train reads correct rpm");
  check(t.valid(now), "fresh train is valid");

  // DECAY: pulses stop. The reading must fall, not hold.
  float held = t.rpm(now + 50000);
  check(held < 1000.0f, "reading decays when pulses stop (no stale high value)");
  check(!t.valid(now + 300000), "goes invalid after the stale timeout");

  // NOISE REJECTION: a spurious edge 100 us after a real one must be ignored.
  PulseTach n(4);
  uint32_t m = 1000;
  for (int i = 0; i < 5; ++i) { n.onPulse(m); m += 4500; }
  float before = n.rpm(m);
  n.onPulse(m - 4400);          // glitch, 100 us after the last real edge
  checkNear(n.rpm(m), before, 50.0f, "EMI glitch does not spike the reading");
  check(n.rejected() == 1, "glitch counted as rejected");
}

// Convenience: a rear wheel rpm that implies a given crank speed.
static float rearForCrank(float crank) { return crank / FINAL_DRIVE; }

static void testSlipHappyPath() {
  printf("\n[slip: engaged and clean]\n");
  SlipMonitor s;
  SlipInputs in;
  in.crank_rpm = 12000.0f;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;
  in.rear_valid = in.front_valid = true;

  // First call after standstill: recovery timers have to run before ALLOW.
  uint32_t t = 10000;
  RegenVerdict v = s.update(t, in);
  for (int i = 0; i < 20 && v != RegenVerdict::Allow; ++i) { t += 50; v = s.update(t, in); }
  check(v == RegenVerdict::Allow, "matched speeds eventually allow regen");
  checkNear(s.clutchSlipRpm(), 0.0f, 1.0f, "no clutch slip reported");
}

static void testSlipSignConvention() {
  printf("\n[slip: sign convention — the whole point]\n");
  SlipConfig cfg;
  SlipMonitor s(cfg);
  SlipInputs in;
  in.rear_valid = in.front_valid = true;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;

  // ENGINE DRIVING: crank LEADS the wheels by a lot. This is normal.
  // A magnitude-only slip test would wrongly cut here.
  in.crank_rpm = 14000.0f;
  uint32_t t = 10000;
  RegenVerdict v = s.update(t, in);
  for (int i = 0; i < 20 && v != RegenVerdict::Allow; ++i) { t += 50; v = s.update(t, in); }
  check(v == RegenVerdict::Allow, "crank LEADING (engine driving) is not a cut");
  check(s.clutchSlipRpm() > 0.0f, "leading slip is positive");

  // REGEN SLIP: crank LAGS the wheels. This is the failure.
  in.crank_rpm = 10000.0f;   // ~17 % behind, well past the 8 % trip
  v = s.update(t + 10, in);
  check(v == RegenVerdict::CutClutchSlip, "crank LAGGING (clutch slipping) cuts regen");
  check(s.clutchSlipRpm() < 0.0f, "lagging slip is negative");
}

static void testSlipCutIsImmediateAndRecoveryIsSlow() {
  printf("\n[slip: cut fast, recover slow]\n");
  SlipMonitor s;
  SlipInputs in;
  in.rear_valid = in.front_valid = true;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;

  uint32_t t = 10000;
  in.crank_rpm = 12000.0f;
  RegenVerdict v = s.update(t, in);
  for (int i = 0; i < 20 && v != RegenVerdict::Allow; ++i) { t += 50; v = s.update(t, in); }
  check(v == RegenVerdict::Allow, "starts allowed");

  // One single bad sample cuts immediately — no persistence filter.
  in.crank_rpm = 10000.0f;
  check(s.update(t + 10, in) == RegenVerdict::CutClutchSlip, "single sample cuts");

  // Slip clears instantly, but regen must NOT come back instantly.
  in.crank_rpm = 12000.0f;
  check(s.update(t + 20, in) == RegenVerdict::CutCooldown, "does not re-allow immediately");
  check(s.update(t + 400, in) == RegenVerdict::CutCooldown, "still held at 380 ms (cooldown 500)");

  // After cooldown AND a clean hold, it comes back.
  uint32_t u = t + 20;
  RegenVerdict w = RegenVerdict::CutCooldown;
  for (int i = 0; i < 40 && w != RegenVerdict::Allow; ++i) { u += 50; w = s.update(u, in); }
  check(w == RegenVerdict::Allow, "re-allows after cooldown + clean hold");
  check(s.cutEvents() == 1, "counted exactly one cut event");
}

static void testFailClosed() {
  printf("\n[slip: fail closed]\n");
  SlipMonitor s;
  SlipInputs in;
  in.crank_rpm = 12000.0f;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;

  // Rear tach dies while the vehicle is clearly moving.
  in.rear_valid = false; in.front_valid = true;
  check(s.update(10000, in) == RegenVerdict::CutNoData, "dead rear tach at speed = CUT");

  in.rear_valid = true; in.front_valid = false;
  check(s.update(10010, in) == RegenVerdict::CutNoData, "dead front tach at speed = CUT");

  // Same missing data at standstill is NOT a fault — it is just parked.
  in.crank_rpm = 0.0f;
  in.rear_valid = in.front_valid = false;
  check(s.update(10020, in) == RegenVerdict::BelowEngagement,
        "no tach data at standstill is not reported as a fault");
}

static void testBelowEngagement() {
  printf("\n[slip: below clutch engagement]\n");
  SlipMonitor s;
  SlipInputs in;
  in.rear_valid = in.front_valid = true;
  in.crank_rpm = 4000.0f;                       // idle-ish, clutch open
  in.rear_wheel_rpm = 0.0f;                     // and the car is not moving
  in.front_wheel_rpm = 0.0f;
  check(s.update(10000, in) == RegenVerdict::BelowEngagement,
        "open clutch reports below-engagement, not clutch slip");
}

static void testTireSlip() {
  printf("\n[slip: tire]\n");
  SlipMonitor s;
  SlipInputs in;
  in.rear_valid = in.front_valid = true;
  in.crank_rpm = 12000.0f;
  in.rear_wheel_rpm = rearForCrank(12000.0f);
  in.front_wheel_rpm = in.rear_wheel_rpm;

  uint32_t t = 10000;
  RegenVerdict v = s.update(t, in);
  for (int i = 0; i < 20 && v != RegenVerdict::Allow; ++i) { t += 50; v = s.update(t, in); }
  check(v == RegenVerdict::Allow, "starts allowed");

  // Rear axle locking under regen: rear 25 % slower than the undriven front.
  // Crank follows the rear, so clutch slip stays clean — only the front
  // comparison catches this. This is the case one sensor cannot see.
  in.rear_wheel_rpm = in.front_wheel_rpm * 0.75f;
  in.crank_rpm = in.rear_wheel_rpm * FINAL_DRIVE;
  check(s.update(t + 10, in) == RegenVerdict::CutTireSlip, "rear locking cuts regen");
  check(s.tireSlipRatio() < -0.2f, "tire slip reported negative (rear slower)");
}

int main() {
  printf("=== Driveline self-test ===\n");
  testErpmConversion();
  testSpeed();
  testPulseTach();
  testSlipHappyPath();
  testSlipSignConvention();
  testSlipCutIsImmediateAndRecoveryIsSlow();
  testFailClosed();
  testBelowEngagement();
  testTireSlip();
  printf("\n%d checks, %d failures -> %s\n", checks, failures,
         failures == 0 ? "PASS" : "FAIL");
  return failures == 0 ? 0 : 1;
}
