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
- **Drivetrain:** rear-wheel drive (made explicit 2026-08-05). Front wheels
  steer only — no front driveshafts, which keeps the custom uprights and
  Ackermann work simpler.
- **v1:** crank-mounted MGU-K with a standard centrifugal clutch. Accepts limited
  brake regen as a known compromise.
- **Transmission: single-speed for v1 (decided 2026-08-05).** The regen case
  for a two-speed doesn't survive scrutiny: kinetic energy scales with v², so
  most recoverable energy sits above the clutch drop-out speed even with
  one gear, and off-the-shelf 1/10 nitro two-speeds drive first gear through a
  one-way bearing — the wheels can't backdrive the input in exactly the gear
  that was supposed to widen the regen window. **Target: 40 km/h at 16,000 rpm
  (decided 2026-08-07)** ⇒ total ratio R ≈ 4.9–5.0 on 63–65 mm touring tires
  (scale by tire diameter if the wheel pick changes). v2 locked-clutch check
  passes: engine firing floor lands at 10 km/h. Final tooth counts wait on the
  tire pick (chassis packaging) and the bench-measured clutch engagement RPM.
- **MGU-K coupling — open until the spec sheet:** belt drive off the existing
  starter-belt interface (per docs 1/3) vs direct coaxial mount on the crank
  nose. THE deciding check: whether the starter-belt interface freewheels
  (one-way bearing — starters crank engines, engines never drive starters). A
  one-way there means the crank can never drive the MGU-K: zero brake regen and
  zero engine-driven charging, in v1 AND v2, regardless of gearing — which
  would force the direct crank mount. Ask EngineDIY alongside the spec-sheet
  request. Motor sizing proceeds either way; mount design waits.
- **MGU-K controller:** VESC-class four-quadrant unit. "Current sensing" means
  the VESC's own UART-reported motor/battery currents — no separate current
  sensor hardware.
- **v2 (later):** locked-clutch full F1-style setup with proper regen.
- **Explicitly rejected:** P3 / downstream-motor layout. Don't re-propose it.

## MGU-K electrical system (sized 2026-08-07)

Two independent sizing studies plus an adversarial verification pass converged
on this envelope. Full trail in BUILD-LOG (2026-08-07 entry).

- **3S (11.1 V) traction system.** Sensored motors only exist as a mainstream
  class in high-kv RC-car form, so 6S/4S have no motor to buy; 2S pushes ~61 A.
  Full-assist battery burst ~41 A. Never move this motor class to 4S — the
  2–3S rating is explicit and it unlocks nothing.
- **Motor: Hobbywing QuicRun 3650SD G2 17.5T, 2170 kv, sensored, 3.175 mm
  shaft (~$50).** The 1900 kv alternative loses too much to back-EMF + I·R at
  the top of the band on a sagged pack (~180–280 W deliverable at 16k rpm);
  the 17.5T delivers ~390–470 W there. Its end-bell timing must be set to the
  zero mark before any four-quadrant/regen use. 450 W bursts are far above
  this class's continuous rating — assist is strictly burst-duty, with a
  firmware duty timer and motor-NTC temp foldback via the VESC.
- **Controller — decision pending (ladder corrected 2026-08-07).** A 4-corner
  market sweep (~40 products, all voltage floors verified from vendor pages)
  disproved the earlier "no mid-tier exists" claim. The real ladder, all
  3S-capable (floor ≤ 9.9 V), all VESC-firmware/VescUart:
  1. **Flipsky Mini FSESC4.20 50A** — $71.99 Amazon in stock (B08725X8CT,
     ships from Amazon, 30-day returns) or $56 flipsky.net (China, 1–3 wk).
     DRV8302 gate driver (the known-fragile die) but 6.6-class FETs. ~350 W cap.
  2. **Makerbase VESC MINI V6.7 Pro** — $92 direct / ~$136 Newegg. True
     6-class architecture (STM32F405, same ON-Semi FETs as genuine VESC 6 —
     but batch-dependent substitutions), 8–60 V verified in 4 sources, 50 A
     cont / 240 A peak. Gate driver IC undisclosed; mixed brand QC; no US
     stock. The one 6-class clone WITHOUT the 14 V floor.
  3. **Trampa VESC 6 EDU** (complete kit £100 ≈ $120–175 shipped UK) —
     genuine Vedder hardware, discrete gate drivers (no DRV8302 single point
     of failure), 6 V floor, 25 A cont / 50 A burst. The earlier "undersized"
     verdict was a continuous-duty argument; this project's duty (2–5 s
     assist bursts at ~50–54 A phase, 2–5 A harvest) sits exactly at its
     burst spec — same ~350 W cap as the Flipsky, zero margin, but failure
     mode is graceful temp foldback, not gate-driver death. Stock unverified
     (Trampa pages don't scrape) — confirm before counting on it.
  4. **Trampa VESC 6 MkVI** ~$270 (UK) — 80 A cont / 120 A burst, the only
     unit with full-450 W margin.
  Everything else verified-out: all other Flipsky 6/75/FT-series (14 V+
  floors, newest FT line isn't even VESC), Maytech (12.6 V floor + cost-cut
  gate driver), Spintend (12 V, $279), Holybro (20 A), Stormcore/Little
  FOCer/Cheap FOCer (15–22 V floors, sold out), and every non-VESC route
  (moteus 10 V floor + CAN-FD rewrite; SOLO UNO passes 8 V floor but caps
  32–45 A at Trampa money; ODrive 12 V). Adam's Flipsky-first probe strategy
  was conditional on "no alternatives" — re-confirm against this ladder.
- **No discrete BMS — deliberate, not an omission.** A port-style BMS that
  opens under regen load-dumps an inductive bus into the VESC (known killer);
  one that never opens adds nothing. Instead: VESC regen cap −10 A, charge
  ceiling 4.15 V/cell, discharge cutoffs 10.2 V soft / 9.9 V hard, every-cycle
  balance charging, 50 A MAXI blade fuse in the pack positive (protects wiring
  only), and a standalone balance-lead low-voltage buzzer during bench runs.
  All limits set AND unplug-tested before the first regen event.
- **Pack:** Zeee 3S 5200 mAh 80C hard-case, XT60, 12 AWG leads (~380–420 g —
  feed into the chassis mass budget). A matched pair lets one pack rest between
  regen sessions. Label 2S vs 3S packs on arrival — they share XT60, and 3S
  into the surrogate ESC over-revs the coupled MGU-K.
- **Surrogate rig:** GoolRC/Surpass 3650 3900 kv sensorless + 60 A car-ESC
  combo (~$37, active proportional brake, independently long-term reviewed),
  1:1 coupling, powered by its OWN 2S pack — never the bench PSU (car-ESC
  braking back-feeds its supply) and never the traction pack. Commanded by a
  ~$10 servo tester until the radio-gear question resolves.
- **Sensor path:** the motor's JST-ZH 6-pin hall harness and the VESC's
  PH-style sensor port don't mate. Plan (decided 2026-08-07): splice an
  adapter from the cables included in the motor and ESC boxes — map by
  FUNCTION not position (5 V/GND/temp per both manuals, multimeter-verify
  5 V and GND before the motor side connects; the three hall wires can land
  in any order — VESC detection sorts them). Keep the temp wire: it feeds
  the motor-NTC foldback. Fallback if the Flipsky box has no sensor pigtail:
  ~$9 pre-crimped JST-PH 2.0 kit (Amazon B08T89ZK2Q) spliced to the motor
  cable. Strain-relieve; shielded run + ferrite once the CDI engine is near.
  The XT60→JST-RCY charge adapter is likewise soldered from owned pigtail
  stock, not purchased.

## Control and telemetry

ESP32-based. Scope covers:

- Assist / regen state machine (the core control logic)
- Energy management and pack state tracking
- MGU-K electrical data via the VESC UART link (RPM, phase current, battery V/I)
- Engine sensing per the planning docs: per-cylinder head temp (MAX31855),
  RPM (A3144 Hall + SmCo magnet), vibration baseline (MPU-6050)
- Aero and dynamics sensing: pitot, ride height, load cells. (Pitot note
  2026-08-07: dynamic pressure at 35–40 km/h is only ~60–76 Pa — spec an
  SDP3x-class ±500 Pa digital differential-pressure sensor on I2C, per the
  no-ADC rule; a 1 psi part would waste its whole range.)
- Data logging pipeline (microSD ground truth + WiFi live), coast-down test support

Architecture ground rules from the planning docs, still in force:

- The ignition world and the electronics world never share power or wiring.
  Anything crossing does so through an optocoupler. Electronics on their own
  battery, star grounding, shielded sensor runs, ferrites at receiving ends.
- The ESP32's ADC is never used: every sensor is a hardware pulse or a digital
  SPI/I2C breakout. New sensor picks (ride height, load cells, anything
  current-related) must be digital-path parts — INA-class monitors, ToF or
  digital ride-height sensing, HX711-class bridge amps — never raw analog.

Driver-command architecture (adopted 2026-08-05, phased):

- **Steering servo is always RX-direct** — a firmware crash must never cost
  steering authority.
- **Phase A (bench + first drives):** engine throttle servo also RX-direct
  (on its own receiver PWM port — the selected FS-R11P outputs PWM and the
  serial stream simultaneously, so no Y-lead is needed); the ESP32 passively
  reads driver demand from the serial stream and commands only the VESC.
  Assist/harvest still runs under software control; a crash costs nothing.
  Added latency is a non-issue either way — carb, combustion, and servo
  mechanics dominate throttle response.
- **Phase B (after real bench hours on the firmware):** throttle servo moves
  behind the ESP32 for full engine+electric blending, only with the failsafe
  kit: carb return spring biased to idle; throttle pin initialized to idle
  first thing in boot, on a non-strapping GPIO (not 0/2/12/15/TX0); task + RTC
  watchdogs; SBUS failsafe-flag parsing plus a ~100 ms frame timeout forcing
  idle/zero-assist; VESC command timeout set to 200–300 ms and proven by an
  unplug test; and post-CDI-conversion, an opto-isolated ignition-kill line on
  its own receiver channel as the ESP32-independent hard stop.
- **Open decision — braking:** crank-side MGU-K + centrifugal clutch means no
  regen braking below engagement speed. Either document coast-down-only braking
  as an accepted v1 limitation, or fit a conventional servo-actuated disc brake
  (standard nitro two-servo layout) as an ESP32-independent stopping path.

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
- Printer may be capable of polycarbonate (material and access still to
  confirm — neither has been looked into yet). Until confirmed, design to
  conservative PETG/ASA-class properties (wall thickness, ribs, inserts) so a
  material downgrade never forces a redesign; doc 1 budgeted PA-CF for the
  structure as the reference point.

## Current status: staged build, engine on backorder

The engine is delayed roughly 3–4 weeks (late August 2026 expected). The starter
kit and gas/CDI conversion kit ship WITH the engine, so harness-connector ID and
the starter-battery buy stay blocked. The SmCo magnets ship separately and land
~2 weeks before it (~mid-August) — the Hall breadboard test and the full
engine-telemetry breadboard unblock then. A vendor swap to a CISON L4 (~$969)
was offered and **declined** — it would have consumed the entire budget on the
least novel component. The plan is to build everything around the engine and
slot the engine in on arrival.

### Purchasing during the wait (decided 2026-08-05)

- **Authorized now, one combined order as selections land:** the bench set —
  MGU-K motor, VESC-class controller, surrogate motor + brake-capable RC-car
  ESC + coupling (~$50–70), single-speed drivetrain parts per the ratio math,
  traction pack + BMS per sizing, plus a real balance charger and LiPo safety
  bag (the owned USB 2S unit can't service a regen-capable pack). Ride the ~$7
  MP1584 3-pack along.
- **Design/sizing only until separately approved:** steering hardware (servo,
  linkage), aero/dynamics sensors, everything else.
- Log every order and price in BUILD-LOG.md as usual; flag cost implications
  before recommending additions.
- **Sourcing rule (2026-08-07):** availability is high-priority — a
  recommended part must be verified in stock (live page check, not cached
  search results) at the time it's listed, and every pick carries a named
  in-stock backup. Commodity RC listings churn in days; re-verify at order
  time.

### Unblocked — work these now

- **Chassis packaging:** dummy block at the 11.2 x 9 x 9.2 cm / 535g envelope;
  outputs wheelbase/track/wheel-tire picks (feeds steering) and the
  telemetry-board envelope (feeds the PCB). Run this ahead of steering.
- **Steering:** Ackermann geometry, bump steer, upright/linkage design — needs
  the wheelbase/track/tire picks from chassis packaging first. Servo purchase
  waits.
- **MGU-K electrical: sized 2026-08-07** (see "MGU-K electrical system" above).
  Remaining: the VESC tier decision, then place the combined bench order and
  wire it per the EMI ground rules.
- **Firmware:** telemetry, logging pipeline, assist/regen state machine (Phase A
  command architecture)
- **Transmission:** single-speed ratio math off the 4,000–16,000 rpm band and
  the bench-measured clutch engagement point, sanity-checked against the v2
  locked-clutch case; then select and order the unit
- **Telemetry PCB (lab fab opportunity):** breadboard the full engine-telemetry
  chain when the magnets land; board outline and mounting location come from
  chassis packaging; a bench perfboard version can come any time, but the
  vehicle PCB is fabbed only after packaging freezes post-engine
  (design-for-slop applies to the board, too).

### Bench surrogate approach (staged 2026-08-05)

Develop the full control loop without the engine; arrival is a driver swap, not
a rewrite. Keep the engine interface behind an abstraction so the surrogate and
the real engine are interchangeable.

- **Stage 1 — no surrogate needed:** VESC + MGU-K free-spinning exercises the
  UART link, telemetry pipeline, logging, and every state-machine transition as
  *logic*. It does NOT validate the charge power path — free-spin regen is over
  in under a second and moves a few watts.
- **Stage 2 — surrogate rig:** a second brushless motor (cheap outrunner +
  brake-capable RC-car ESC — an airplane ESC can only drive, halving the rig)
  spins the MGU-K as a controllable crank stand-in. The first SUSTAINED
  regen/charge test happens here, never during engine break-in:
  pack-accepts-current, BMS-stays-closed, bus-voltage-in-bounds are exactly the
  failures to find on a $50 rig at tens of watts instead of stacked on a
  temperamental first-run engine.
- Also bench-measure the clutch engagement RPM here (spring-tunable; the ratio
  and regen-window math both need the real number). **Spring choice is the
  regen lever, not gearing:** the window fraction is engagement/16,000 rpm
  regardless of ratio — 6k engagement ⇒ regen over the top ~62% of the speed
  range, 9k ⇒ ~44%. Bias springs toward 6–7k if idle stability allows (twin
  idles ~2.5–4k; keep clear margin). Also measure crank-line moment of
  inertia (spin-down test) — reflected through R²/r² it adds ~10–20%
  effective mass, so accel/energy predictions should use ~3.4–3.7 kg.
  Firmware must cap regen torque below the RPM-dependent clutch capacity and
  cut regen on detected slip (VESC RPM vs wheel-derived RPM divergence) —
  centrifugal clutch slip is self-reinforcing and glazes the shoes.
- **Bench safety rule:** never run brake/regen commands with only a bench PSU
  on the DC bus — regenerated current back-feeds a non-bidirectional supply.
  Keep the traction pack (or a braking resistor) connected.

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

## Open questions (as of 2026-08-05, consolidated for research)

Ask EngineDIY (with the pending spec-sheet request):

- **Does the starter-belt interface freewheel (one-way bearing)?** Decides the
  MGU-K coupling — a one-way forces the direct crank mount for any regen at all.
- Chase the dimensioned spec sheet itself: crank shaft diameter/length, thread
  pitch and direction, keyway/flat; mounting boss hole pattern; crank
  centerline height above the mounting face.
- **Which centrifugal clutch kit fits the ST-NF2**, and are engagement springs
  available/tunable? The clutch is the heart of the v1 driveline and its
  engagement RPM feeds the ratio math.

Ask the lab:

- **Printer:** which machine, which materials (is polycarbonate real), and can
  it be used for a personal project?
- **PCB fab:** what process and constraints — max board size, layers, minimum
  trace/space, accepted design files (KiCad?), turnaround. The telemetry board
  gets designed to these limits.

Check what's on hand / decide:

- **Radio gear: selected 2026-08-07, purchase awaiting approval (~$86).**
  Flysky FS-G7P+ + FS-R11P (11 PWM ports + dedicated serial port,
  simultaneous by hardware design; documented SBUS failsafe flags). Full
  wiring map, day-one failsafe ritual, and bench acceptance tests in
  `docs/radio-setup.md`. Backup system: Radiolink RC6GS V3 + R7FG (~$75).
- ~~Target top speed~~ **decided 2026-08-07: 40 km/h** — see the Transmission
  bullet in Powertrain architecture; math trail in BUILD-LOG (2026-08-07).
- **Brake:** accept coast-down-only braking as a documented v1 limitation, or
  fit a servo-actuated disc as an ESP32-independent stopping path?
- **Bench rig location and guarding:** where do two coupled motors at speed
  live, and behind what?
- Optional but useful: a comfort number for the combined bench-set order — it
  steers component tier (e.g. VESC-clone vs genuine).

Owed by the work itself (not research): wheelbase / track / wheel-tire package
from chassis packaging; pack voltage and capacity from MGU-K sizing.
