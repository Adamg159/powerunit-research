// Driveline.h — crank/wheel speed conversion and the regen slip-cut.
//
// Header-only, no Arduino dependency: every function takes time as an argument
// instead of calling millis()/micros(). That is deliberate — it lets the whole
// module be exercised by a self-test with no hardware and no clock, which is
// the only practical way to prove control logic whose failure mode is
// "glazed clutch shoes."
//
// This file is the SINGLE PLACE the ERPM -> mechanical RPM conversion lives.
// See crankRpmFromErpm() for why that matters more here than in a normal build.
//
// Usage sketch (vehicle firmware):
//   driveline::PulseTach rearTach(WHEEL_MAGNETS), frontTach(WHEEL_MAGNETS);
//   void IRAM_ATTR rearISR()  { rearTach.onPulse(micros()); }
//   ...
//   driveline::SlipMonitor slip;
//   auto v = slip.update(millis(), {crankRpmFromErpm(vesc.erpm),
//                                   rearTach.rpm(micros()), frontTach.rpm(micros()),
//                                   rearTach.valid(micros()), frontTach.valid(micros())});
//   if (v != driveline::RegenVerdict::Allow) commandRegen(0);

#pragma once

#include <stdint.h>
#include <math.h>

namespace driveline {

// ---------------------------------------------------------------------------
// Vehicle constants — single source of truth. Update here, nowhere else.
// ---------------------------------------------------------------------------

// Motor POLES, not pole pairs — this is the number the Hobbywing manual prints
// and the number VESC Tool asks for, so keeping the same units in both places
// removes a whole class of factor-of-two mistake.
//
// QuicRun 3650SD G2 (PN 30404306, 17.5T): the manual's spec table gives
// **Pole = 2** for every motor in the family. RC 3650 inrunners use a 2-pole
// rotor, i.e. ONE pole pair. So ERPM and mechanical RPM are the SAME NUMBER
// on this motor.
//
// This corrects an earlier assumption of 2 pole pairs (2026-08-16). It was
// wrong in the harmless direction — the crank would have read half speed,
// looked permanently behind the wheels, and latched the slip-cut so regen
// never engaged — but wrong is wrong, and a permanently-cut regen would have
// been maddening to debug on the rig.
//
// STILL VERIFY ON THE BENCH (task A8): turn the shaft one known revolution by
// hand and confirm the reported ERPM change matches 1:1. Cheap, and the manual
// is not the motor.
constexpr float MOTOR_POLES = 2.0f;
constexpr float MOTOR_POLE_PAIRS = MOTOR_POLES / 2.0f;

// Crank revolutions per wheel revolution (target 40 km/h @ 16,000 rpm).
// Provisional until the tire pick and the measured clutch engagement RPM land.
constexpr float FINAL_DRIVE = 4.95f;

// Magnets per wheel (plan: 4, all with the same pole facing out — the A3144 is
// unipolar). One magnet per wheel also works, at a quarter the resolution.
constexpr uint8_t WHEEL_MAGNETS = 4;

// Rolling diameter, metres. 63 mm touring tire.
constexpr float TIRE_DIAMETER_M = 0.063f;

// ---------------------------------------------------------------------------
// THE conversion.
// ---------------------------------------------------------------------------
//
// The VESC reports ERPM (electrical RPM), not mechanical RPM. In a normal
// build that is a cosmetic detail in a telemetry readout. Here it is a safety
// item, because the MGU-K is mounted directly on the crank nose, so the VESC
// *is* the crank tachometer, and the slip-cut below compares that number
// against a wheel-derived crank speed.
//
// On THIS motor the ratio happens to be 1:1 (2 poles), so the conversion is
// currently an identity. Keep calling it anyway. The moment the motor changes
// — a 4-pole outrunner for v2, say — every call site is already correct, and
// the alternative is hunting for bare `erpm` uses under a machine that bites
// silently: get the factor wrong high and the crank looks fast, which reads as
// permanent positive slip and the cut never fires in the regen direction; get
// it wrong low and regen never engages at all. Centrifugal clutch slip does
// not announce itself, it just glazes the shoes.
//
// So: one function, one place, and nobody scales ERPM anywhere else.

inline float crankRpmFromErpm(float erpm) { return erpm / MOTOR_POLE_PAIRS; }
inline float erpmFromCrankRpm(float rpm) { return rpm * MOTOR_POLE_PAIRS; }

// Wheel RPM -> ground speed, m/s.
inline float wheelRpmToSpeedMps(float wheel_rpm, float diameter_m = TIRE_DIAMETER_M) {
  return (wheel_rpm / 60.0f) * 3.14159265f * diameter_m;
}

// ---------------------------------------------------------------------------
// PulseTach — Hall pulses to RPM.
// ---------------------------------------------------------------------------
//
// Period-based, not count-based: at walking pace a count-per-window scheme
// either has terrible resolution or terrible latency, and the slip-cut needs
// both. Measures the interval between pulses instead.
//
// Two behaviours worth knowing about:
//
//  - DECAY. If pulses stop arriving, rpm() does not keep reporting the last
//    interval. It reports the slower of (last interval) and (time since last
//    pulse), so a wheel that stops braking-hard decays toward zero instead of
//    holding a stale high reading. Holding a stale high rear-wheel reading
//    during a lock-up is precisely the reading that would hide a slip event.
//
//  - REJECTION. Intervals shorter than min_interval_us are discarded as noise.
//    A CDI ignition a few centimetres away is an EMI source, and one spurious
//    edge otherwise reads as a momentary 10x overspeed.
//
// ISR safety: onPulse() is ISR context; rpm()/valid() are main-loop context.
// The pair is published under a sequence counter, so the reader retries if it
// catches a torn update. Assumes the ISR cannot be preempted by the reader,
// which holds on ESP32 for a single tach's pin.

class PulseTach {
 public:
  explicit PulseTach(uint8_t magnets = WHEEL_MAGNETS,
                     uint32_t stale_us = 250000,   // ~1 rev/s floor at 4 magnets
                     uint32_t min_interval_us = 400)
      : magnets_(magnets ? magnets : 1),
        stale_us_(stale_us),
        min_interval_us_(min_interval_us) {}

  // ISR context. `now_us` is a free-running microsecond clock; unsigned
  // subtraction makes the 71-minute wraparound a non-event.
  void onPulse(uint32_t now_us) {
    if (have_) {
      uint32_t dt = now_us - last_us_;
      if (dt < min_interval_us_) { rejected_++; return; }
      interval_us_ = dt;
    }
    last_us_ = now_us;
    have_ = true;
    count_++;
    seq_++;  // publish last
  }

  float rpm(uint32_t now_us) const {
    uint32_t last, iv;
    if (!snapshot(last, iv) || iv == 0) return 0.0f;
    uint32_t since = now_us - last;
    uint32_t eff = (iv > since) ? iv : since;   // decay — see note above
    if (eff == 0) return 0.0f;
    return 60000000.0f / ((float)eff * (float)magnets_);
  }

  bool valid(uint32_t now_us) const {
    uint32_t last, iv;
    if (!snapshot(last, iv) || iv == 0) return false;
    return (uint32_t)(now_us - last) < stale_us_;
  }

  uint32_t pulses() const { return count_; }
  uint32_t rejected() const { return rejected_; }

  // Test seam: inject a pulse train without an interrupt controller.
  void resetForTest() { have_ = false; interval_us_ = 0; last_us_ = 0; count_ = 0; rejected_ = 0; seq_ = 0; }

 private:
  bool snapshot(uint32_t& last, uint32_t& iv) const {
    for (int tries = 0; tries < 4; ++tries) {
      uint32_t s1 = seq_;
      last = last_us_;
      iv = interval_us_;
      if (s1 == seq_) return have_;
    }
    return false;   // contended beyond reason; report no data (fail closed)
  }

  uint8_t magnets_;
  uint32_t stale_us_;
  uint32_t min_interval_us_;

  volatile uint32_t last_us_ = 0;
  volatile uint32_t interval_us_ = 0;
  volatile uint32_t count_ = 0;
  volatile uint32_t rejected_ = 0;
  volatile uint32_t seq_ = 0;
  volatile bool have_ = false;
};

// ---------------------------------------------------------------------------
// SlipMonitor — decides whether regen is allowed to happen at all.
// ---------------------------------------------------------------------------
//
// Two independent slips, two sensors on two different axles:
//
//   CLUTCH slip = crank RPM  vs  rear-wheel RPM x final drive.
//     This is the one the plan mandates cutting on. During regen the MGU-K
//     brakes the crank, so if the clutch is slipping the crank falls BEHIND
//     what the rear wheels say it should be doing. Sign matters: crank
//     LEADING is normal under engine drive, crank LAGGING under regen is the
//     failure. A magnitude-only test would fire constantly under acceleration.
//
//   TIRE slip = rear wheel vs front (undriven) wheel.
//     Rear locking under regen braking is a stability problem, not a clutch
//     problem, but the response is the same: stop regenerating.
//
// Bias throughout is FAIL CLOSED. Losing a wheel sensor means no regen, not
// unguarded regen: without the wheel-side pickup the crank-mounted VESC has no
// second term to compare against, and the comparison is the whole safety case.
//
// Cut is immediate; re-allow is slow and deliberate. Cutting one cycle early
// costs a few joules of harvest. Cutting one cycle late glazes the shoes,
// which is a parts order and a teardown.

enum class RegenVerdict : uint8_t {
  Allow,               // regen permitted
  BelowEngagement,     // normal, not a fault: clutch is open, nothing to harvest
  CutClutchSlip,       // crank lagging the rear wheels — the mandated cut
  CutTireSlip,         // rear axle speed diverging from ground truth
  CutNoData,           // a wheel tach is stale while the vehicle is moving
  CutCooldown,         // slip has cleared but the hold-off has not expired
};

const char* verdictName(RegenVerdict v);

struct SlipConfig {
  float final_drive = FINAL_DRIVE;

  // Regen is pointless below clutch engagement. BENCH-MEASURED NUMBER —
  // this placeholder is a guess until the surrogate rig produces the real one.
  float engagement_rpm = 6500.0f;

  // Clutch slip trip: whichever of these is larger at the current speed.
  float clutch_slip_frac = 0.08f;        // 8 % of expected crank speed
  float clutch_slip_floor_rpm = 300.0f;  // absolute floor, kills low-speed noise

  // Tire slip trip, as a fraction of front (ground truth) speed.
  float tire_slip_frac = 0.15f;
  float min_front_rpm_for_tire_slip = 60.0f;  // below this the ratio is noise

  // Re-allow discipline.
  float recover_slip_frac = 0.03f;   // must come back well inside the trip point
  uint32_t cooldown_ms = 500;        // minimum time cut, regardless of recovery
  uint32_t recover_hold_ms = 300;    // and must look clean for this long
};

struct SlipInputs {
  float crank_rpm = 0.0f;        // MECHANICAL — pass crankRpmFromErpm(erpm)
  float rear_wheel_rpm = 0.0f;
  float front_wheel_rpm = 0.0f;
  bool rear_valid = false;
  bool front_valid = false;
};

class SlipMonitor {
 public:
  explicit SlipMonitor(const SlipConfig& cfg = SlipConfig()) : cfg_(cfg) {}

  RegenVerdict update(uint32_t now_ms, const SlipInputs& in) {
    // Expected crank speed implied by the driven axle.
    expected_crank_rpm_ = in.rear_wheel_rpm * cfg_.final_drive;
    clutch_slip_rpm_ = in.crank_rpm - expected_crank_rpm_;   // negative = crank lagging
    tire_slip_ratio_ = 0.0f;

    const bool moving = in.crank_rpm >= cfg_.engagement_rpm;

    // --- fail closed on missing data -------------------------------------
    if (!in.rear_valid || !in.front_valid) {
      // Standstill is not a fault; a stale tach while the crank is spinning is.
      return latchIfCut(now_ms, moving ? RegenVerdict::CutNoData
                                       : RegenVerdict::BelowEngagement);
    }

    if (!moving) return latchIfCut(now_ms, RegenVerdict::BelowEngagement);

    // --- clutch slip ------------------------------------------------------
    float trip = cfg_.clutch_slip_frac * expected_crank_rpm_;
    if (trip < cfg_.clutch_slip_floor_rpm) trip = cfg_.clutch_slip_floor_rpm;
    const bool clutch_bad = clutch_slip_rpm_ < -trip;

    // --- tire slip --------------------------------------------------------
    bool tire_bad = false;
    if (in.front_wheel_rpm >= cfg_.min_front_rpm_for_tire_slip) {
      tire_slip_ratio_ = (in.rear_wheel_rpm - in.front_wheel_rpm) / in.front_wheel_rpm;
      tire_bad = fabsf(tire_slip_ratio_) > cfg_.tire_slip_frac;
    }

    if (clutch_bad) return latchIfCut(now_ms, RegenVerdict::CutClutchSlip);
    if (tire_bad) return latchIfCut(now_ms, RegenVerdict::CutTireSlip);

    // --- clean: earn the way back in --------------------------------------
    float recover = cfg_.recover_slip_frac * expected_crank_rpm_;
    if (recover < cfg_.clutch_slip_floor_rpm) recover = cfg_.clutch_slip_floor_rpm;
    const bool clean = clutch_slip_rpm_ > -recover;

    if (!clean) { clean_since_ms_ = 0; return latchIfCut(now_ms, last_cut_); }
    if (clean_since_ms_ == 0) clean_since_ms_ = now_ms ? now_ms : 1;

    if (cut_) {
      const bool cooled = (uint32_t)(now_ms - cut_at_ms_) >= cfg_.cooldown_ms;
      const bool held = (uint32_t)(now_ms - clean_since_ms_) >= cfg_.recover_hold_ms;
      if (!(cooled && held)) return RegenVerdict::CutCooldown;
      cut_ = false;
    }
    return RegenVerdict::Allow;
  }

  // Telemetry — log these, they are how the thresholds get tuned.
  float clutchSlipRpm() const { return clutch_slip_rpm_; }
  float expectedCrankRpm() const { return expected_crank_rpm_; }
  float tireSlipRatio() const { return tire_slip_ratio_; }
  bool isCut() const { return cut_; }
  uint32_t cutEvents() const { return cut_events_; }

  const SlipConfig& config() const { return cfg_; }
  void setConfig(const SlipConfig& c) { cfg_ = c; }

 private:
  RegenVerdict latchIfCut(uint32_t now_ms, RegenVerdict v) {
    if (v == RegenVerdict::BelowEngagement) {
      // Not a fault, but it does reset the recovery timer: the clutch has been
      // open, so nothing has been proven about its grip.
      clean_since_ms_ = 0;
      cut_ = false;
      return v;
    }
    if (!cut_) { cut_ = true; cut_events_++; }
    cut_at_ms_ = now_ms;
    clean_since_ms_ = 0;
    last_cut_ = v;
    return v;
  }

  SlipConfig cfg_;
  bool cut_ = false;
  uint32_t cut_at_ms_ = 0;
  uint32_t clean_since_ms_ = 0;
  uint32_t cut_events_ = 0;
  RegenVerdict last_cut_ = RegenVerdict::CutNoData;

  float clutch_slip_rpm_ = 0.0f;
  float expected_crank_rpm_ = 0.0f;
  float tire_slip_ratio_ = 0.0f;
};

inline const char* verdictName(RegenVerdict v) {
  switch (v) {
    case RegenVerdict::Allow: return "ALLOW";
    case RegenVerdict::BelowEngagement: return "below-engagement";
    case RegenVerdict::CutClutchSlip: return "CUT clutch-slip";
    case RegenVerdict::CutTireSlip: return "CUT tire-slip";
    case RegenVerdict::CutNoData: return "CUT no-data";
    case RegenVerdict::CutCooldown: return "CUT cooldown";
  }
  return "?";
}

}  // namespace driveline
