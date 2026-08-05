# Hybrid RC Car — Project Context

RC-scale hybrid vehicle: a small four-stroke IC engine with a parallel electric
assist motor (MGU-K style), custom chassis and aero, and an ESP32-based telemetry
and energy management system.

Goal is a **portfolio piece** demonstrating hybrid powertrain integration,
embedded control, and vehicle dynamics understanding. It is explicitly *not*
optimized for outright speed. When there's a tradeoff between "faster" and
"better demonstrates the engineering," choose the latter.

This file is the governing project context. The three HTML planning docs
(01/02/03) and BUILD-LOG.md hold the detail; where they conflict with this
file, this file wins.

## Powertrain architecture

- **Engine:** SEMTO ST-NF2 (a.k.a. TOYAN / OTTO MOTOR FS-L200AC) — 7.0cc SOHC
  inline twin, four-stroke, air-cooled. 0.6ps, 4,000–16,000 rpm.
  535g, 11.2 x 9 x 9.2 cm bare. Ordered with starter kit + gas/CDI conversion kit.
- **Hybrid layout:** parallel MGU-K, crank-mounted, no turbo. Chosen to mirror
  current F1 regulations as closely as is feasible at this scale.
- **v1:** crank-mounted MGU-K with a standard centrifugal clutch. Accepts limited
  brake regen as a known compromise.
- **MGU-K coupling — open (decided 2026-08-05 to stay open):** belt drive off
  the engine's existing starter-belt interface (per docs 1/3) vs direct coaxial
  mount on the crank nose. Undecided until the vendor's dimensioned spec sheet
  arrives. Motor sizing proceeds either way; mount design waits.
- **MGU-K controller:** VESC-class four-quadrant unit. "Current sensing" means
  the VESC's own UART-reported motor/battery currents — no separate current
  sensor hardware.
- **v2 (later):** locked-clutch full F1-style setup with proper regen.
- **Explicitly rejected:** P3 / downstream-motor layout. Don't re-propose it.

## Control and telemetry

ESP32-based. Scope covers:

- Assist / regen state machine (the core control logic)
- Energy management and pack state tracking
- MGU-K electrical data via the VESC UART link (RPM, phase current, battery V/I)
- Engine sensing per the planning docs: per-cylinder head temp (MAX31855),
  RPM (A3144 Hall + SmCo magnet), vibration baseline (MPU-6050)
- Aero and dynamics sensing: pitot, ride height, load cells
- Data logging pipeline (microSD ground truth + WiFi live), coast-down test support

Architecture ground rules from the planning docs, still in force:

- The ignition world and the electronics world never share power or wiring.
  Anything crossing does so through an optocoupler. Electronics on their own
  battery, star grounding, shielded sensor runs, ferrites at receiving ends.
- The ESP32's ADC is never used: every sensor is a hardware pulse or a digital
  SPI/I2C breakout. New sensor picks (ride height, load cells, anything
  current-related) must be digital-path parts — INA-class monitors, ToF or
  digital ride-height sensing, HX711-class bridge amps — never raw analog.

## Deferred / stretch (decided 2026-08-05)

- **Custom ignition-timing ECU** — deferred out of the core plan, parked
  alongside the v2 locked-clutch work. The engine runs on the stock
  fixed-timing CDI indefinitely; success criteria no longer require a mapped
  ignition curve. Consequences: the third ESP32 and the second tuned buck
  revert to true spares, and the Phase 2 opto/trigger BOM lines plus the
  Phase 1 crank-trigger-capture task are shelved with it.
- **v2 locked clutch + full brake regen** — as before, only after full success.

## Chassis and body

- 3D-printed monocoque-style chassis in a durable material, with lighter
  detachable aero parts mounted to it
- Body and aero surfaces designed from scratch in SolidWorks — no purchased body
  kit, for full control over the aerodynamics
- Printer may be capable of polycarbonate (material and access still to confirm)

## Current status: staged build, engine on backorder

The engine is delayed roughly 3–4 weeks (late August 2026 expected). A vendor
swap to a CISON L4 (~$969) was offered and **declined** — it would have consumed
the entire budget on the least novel component. The plan is to build everything
around the engine and slot the engine in on arrival.

### Purchasing during the wait (decided 2026-08-05)

- **Authorized now, no per-item check-in:** the bench set — MGU-K motor,
  VESC-class controller, bench surrogate motor (+ its drive), gearbox, and the
  traction pack (+ BMS per sizing) the rig needs to run.
- **Design/sizing only until separately approved:** steering hardware (servo,
  linkage), aero/dynamics sensors, everything else.
- Log every order and price in BUILD-LOG.md as usual; flag cost implications
  before recommending additions.

### Unblocked — work these now

- **Steering:** servo, linkage, uprights, Ackermann geometry, bump steer
- **MGU-K electrical:** motor sizing, VESC selection, pack and BMS sizing,
  wiring (per the EMI ground rules above)
- **Firmware:** telemetry, logging pipeline, assist/regen state machine
- **Transmission (partial):** ratio math off the 4,000–16,000 rpm range; gearbox
  can be ordered now
- **Chassis packaging:** using a dummy block at the 11.2 x 9 x 9.2 cm / 535g
  envelope as a stand-in

### Bench surrogate approach

Develop the full control loop without the engine: mount a second brushless motor
as a crank stand-in, spin the MGU-K against it, and validate the state machine and
telemetry end to end on the bench. When the engine arrives it should be a driver
swap, not a rewrite. Keep the engine interface behind an abstraction so the
surrogate and the real engine are interchangeable.

### Blocked until the engine (or a dimensioned drawing) arrives

- Crank interface: shaft diameter, length, thread pitch and direction, keyway/flat
- Mounting boss hole pattern relative to crank centerline
- Crank centerline height above the mounting face
- Real torque curve and vibration signature (needed for engagement tuning)

A dimensioned spec sheet has been requested from the vendor.

### Engine-arrival sequence (unchanged from the planning docs)

"Slot-in-able" refers to the chassis and powertrain interfaces — it does not
skip the bench sequence. On arrival: unboxing inspection checklist (doc 2 §2),
glow/nitro break-in and tuning BEFORE the CDI conversion, high-voltage ground
strap installed BEFORE the CDI kit goes on (known kit defect that otherwise
destroys the Hall sensor and ignition module), cam box greased before first
start, short starting bursts, outdoor running only. The deferred engine-arrival
purchases (washer thermocouples, starter/CDI battery, balance charger,
consumables, bench materials, ~$120–160 + the ~$7 MP1584 3-pack) still trigger
at unboxing.

### Design-for-slop rule

Kit drawings are approximate and unit-to-unit variation is real. Slot motor mount
holes rather than drilling to size, plan a shim stack at the crank interface, and
leave clearance around anything non-structural. Final-fit on arrival.

## Working notes

- Starting from near-zero on engine experience — this project is for learning.
  Explain mechanical reasoning rather than just handing over answers.
- Budget is tight and self-funded for phase 1. Flag cost implications of
  suggestions.
- Prefer approaches that keep the engine slot-in-able and the bench surrogate
  viable.

## Repo conventions

- BUILD-LOG.md is the running log and the portfolio deliverable — append a
  dated entry (progress, problems, resolutions) whenever project work happens.
- The repo is public (github.com/Adamg159/powerunit-research): no secrets;
  WiFi credentials go in a gitignored `secrets.h` once firmware needs them.
- Sync between desktop and laptop: pull before working, commit + push after.

## Open questions (as of 2026-08-05)

- Which gearbox (make/model/ratio) — is one already chosen, or does the order
  wait on the ratio math?
- Did the starter kit, CDI conversion kit, and SmCo magnets ship separately, or
  are they held with the backordered engine? (Magnets gate the last open bench
  test — Hall sensor breadboard; the harness connector gates the starter-battery
  buy.)
- Bench surrogate motor: owned or a purchase, and what spec (kv / power / shaft)?
- Printer and chassis material: is polycarbonate access confirmed, and what is
  the design-fallback material if not?
- Steering geometry inputs: wheelbase, track, wheel/tire package.
- Radio architecture: receiver → ESP32 → actuators, or conventional
  receiver-direct with the ESP32 listening?
- Traction pack charging: the owned USB 2S charger won't cover a regen-capable
  pack — what charger, and where does the rig live/how is it guarded?
