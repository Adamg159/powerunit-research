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
- **MGU-K coupling — DECIDED 2026-08-11: direct crank-nose mount.** The
  starter-belt route is dead. The one-way bearing lives inside the
  crank-mounted start belt pulley (factory BOM item 37, "Start belt pulley
  component" — the only start-drive part designated a sub-assembly; the motor
  side is item 52, a plain "Ten-tooth adapter" with no bearing). The crank
  therefore cannot drive that belt: **zero brake regen and zero engine-driven
  charging through the starter path, v1 AND v2, regardless of gearing.**
  Confirmed by an adversarial verification pass; full evidence chain in
  BUILD-LOG (2026-08-11).
  - The belt route fails a second, independent test even if the one-way were
    pressed out: the drive is ~7:1 (70-tooth crank pulley, 10-tooth motor
    adapter), so a 16,000 rpm crank asks ~112,000 rpm of a motor that tops out
    near 24,000. Converting it means replacing both pulleys and the belt.
    **Do not re-propose the belt route.**
  - Precedent to copy: Toyan's own 12 V generator kit for this engine takes
    drive from a dedicated pulley added at the CRANK NOSE outboard of the
    flywheel, and does not tap the starter pulley.
  - **The clutch kit's belt-pulley variants are NOT an MGU-K drive.** The
    "with Single-V / Double-V Groove Belt Pulley" and "Synchronous Pulley
    Clutch" bells put their pulley DOWNSTREAM of the shoes, so drive vanishes
    the moment the clutch disengages — the MGU-K must be upstream, on the crank.
    They look like a free fix for the crank-nose stack-up and are not one.
    **DECIDED 2026-08-14: buy the plain Single-gear Clutch, $27.99 — order
    unblocked, no further vendor answers needed.** All six kit variants share
    identical internals (pin-disc flywheel, blue shoe carrier, hex adapter,
    bearings) and differ only in the bell, so the video's compatibility demo
    (which filmed the V-groove variant) transfers to the gear variant. The
    V-groove recommendation from EngineDIY was re-derived and rejected on our
    own requirements: a V-belt slips under torque and belt slip is
    indistinguishable from clutch slip in the regen slip-cut's RPM comparison;
    the vendor's advice fits their marine/generator customers, not a car with
    a regen control loop. Gear pinion is ~15–16T, module unpublished —
    self-serve with calipers on arrival (mod = OD/(N+2)) before ordering the
    spur, which is gated on the tire pick and engagement RPM anyway. Bore
    number likewise demoted to measure-on-arrival: the factory video shows the
    kit hand-fitting the real engine. Full reasoning in BUILD-LOG 2026-08-14
    (later).
  - **Packaging conflict to solve:** the centrifugal clutch also replaces the
    stock flywheel at the crank nose. Clutch and MGU-K drive want the same real
    estate. Mount design must resolve that stack-up (nose is only ~21 mm long).
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
- **Motor manual data (received 2026-08-16, PN 30404306):** 2170 kv, **2 POLES
  (= 1 pole pair, so VESC ERPM equals mechanical RPM — see Driveline.h)**,
  R = 0.0488 Ω, no-load 1.6 A, Ø36 × 52.8 mm, shaft Ø3.17 × 15 mm, **187 g**,
  2–3S. **Motor can must never exceed 90 °C** — above that the magnets
  demagnetise and the coils can melt, so the NTC foldback starts at 70–75 °C
  and is fully cut by ~85 °C. Phase colours: **A = blue, B = yellow,
  C = orange; wire A-A, B-B, C-C** — with a sensored ESC the phase order is
  NOT free to swap. Bearing spare: R2ZZ 3.175 × 9.525 × 3.967.
- **VESC manual data (same day):** hardware V4.20, 8–60 V, **3–13S — 3S is the
  MINIMUM**, so the pack choice sits at the bottom edge of the window (this is
  why 2S had no controller). BEC 5 V @ 1.5 A. 67 × 39 × 18.3 mm with heatsink,
  **80 g**. Box contents confirm a VESC sensor wire is included.
  SENSE port pinout: `GND | H3 | H2 | H1 | TMP | 5V` (5 V and GND at OPPOSITE
  ENDS — a flipped cable swaps both rails). COMM port, for the ESP32 UART:
  `5V | 3.3V | GND | ADC | TX | RX | ADC2`.
- **Motor: Hobbywing QuicRun 3650SD G2 17.5T, 2170 kv, sensored, 3.175 mm
  shaft (~$50).** The 1900 kv alternative loses too much to back-EMF + I·R at
  the top of the band on a sagged pack (~180–280 W deliverable at 16k rpm);
  the 17.5T delivers ~390–470 W there. **The end-bell timing task is CLOSED as
  N/A (2026-08-16): this motor has FIXED timing** — inspection found no scale,
  no rotatable ring and no clamp screws, only the three M2.5 × 47.8 mm
  through-screws, and the manual sets timing in the ESC ("Zero Timing Mode"),
  not the motor. Harmless: VESC FOC hall detection measures the real sensor
  angles and compensates a static offset in BOTH directions of rotation. The
  drive/brake asymmetry argument applies to trapezoidal ESCs, not FOC.
  **Do not re-propose adjusting it.** 450 W bursts are far above
  this class's continuous rating — assist is strictly burst-duty, with a
  firmware duty timer and motor-NTC temp foldback via the VESC.
- **Controller — DECIDED and ORDERED 2026-08-07: Flipsky Mini FSESC4.20 50A,
  $71.99 (Amazon FBA, B08725X8CT), delivered Aug 10.** Adam's call: probe the
  cheapest 3S-capable unit first and upgrade only if it dies. The ladder below
  is retained as the upgrade path, not an open decision — if the DRV8302 gate
  driver fails, step to the Trampa VESC 6 EDU or MkVI rather than re-running
  the market sweep. Ladder, all 3S-capable (floor ≤ 9.9 V), all
  VESC-firmware/VescUart:
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
- **Sensor path — RESOLVED 2026-08-16: NO SPLICE NEEDED.** The premise of the
  2026-08-07 plan (that the motor's hall harness and the VESC's sensor port
  don't mate) was wrong: the Flipsky box shipped a sensor cable whose 6-pin
  connector mates the Hobbywing ribbon directly — motor end-bell → black
  6-conductor ribbon → white 6-pin joint → coloured pigtail → VESC sensor
  port. Confirmed by photo. **Do not order the ~$9 pre-crimped JST-PH kit
  (B08T89ZK2Q); do not solder an adapter.**
  - **The verification survives even though the soldering doesn't.** Mating is
    not matching: identical housings hide different pin orders, and reversed
    5 V/GND kills the hall ICs instantly and silently. With the pair unmated
    and the VESC on USB only, confirm one pigtail pin at ~5 V and one at 0 V,
    then confirm the motor side puts Vcc/GND on those same positions. Hall
    wires can land in any order — VESC detection sorts them.
  - **Open question the photos raised: does the motor actually have an NTC?**
    The 6th conductor is nominally the temp line, but sensored RC motors often
    leave it unpopulated behind a fully-pinned connector. Measure temp-pin to
    GND at the motor: ~10 kΩ at room temp = fitted, open = absent. If absent,
    the motor-temp foldback needs a separate NTC epoxied to the can — and that
    foldback is what makes burst-duty assist an enforced limit rather than an
    intention, so it is not optional.
  - Strain-relieve; shielded run + ferrite once the CDI engine is near.
  - The XT60→JST-RCY charge adapter is still soldered from owned pigtail
    stock, not purchased.

## Control and telemetry

ESP32-based. Scope covers:

- Assist / regen state machine (the core control logic)
- Energy management and pack state tracking
- MGU-K electrical data via the VESC UART link (RPM, phase current, battery V/I)
- Engine sensing per the planning docs: per-cylinder head temp (MAX31855),
  RPM (A3144 Hall + SmCo magnet), vibration baseline (MPU-6050)
- **Wheel speed — added 2026-08-11 (was a gap), part picked same day.** Needed
  three ways over: the mandated regen-slip cut compares VESC RPM against a
  wheel-derived RPM, but with the MGU-K crank-mounted the VESC *is* the crank
  tach, so the comparison has no second term without a wheel-side pickup; true
  vehicle speed needs it; and so do the coast-down tests.
  - **Two sensors on different axles.** FRONT (undriven) = true ground speed,
    the reference. REAR (driven) = driven speed. Crank RPM vs rear × ratio
    gives CLUTCH slip (what firmware must cut regen on); rear vs front gives
    TIRE slip. One sensor gives neither cleanly.
  - **Part: bare A3144 / AH3144E unipolar Hall switch** — same family as the
    engine RPM pickup, so one part to stock and one breadboard technique.
    Hardware pulse, satisfies the no-ADC rule.
  - **WIRING GOTCHA — power at 5 V, pull up to 3.3 V.** The A3144 needs 4.5 V
    minimum so it cannot run at 3.3 V, but its output is OPEN-COLLECTOR, so a
    pull-up to 3.3 V makes the output swing 0–3.3 V and no level shifter is
    needed. Do NOT use a KY-003-style breakout at 5 V — its onboard pull-up
    goes to its own VCC and would put 5 V on an ESP32 pin. Bare chip + own
    10 kΩ pull-up is the clean build.
  - **USE THE PARTS ALREADY ON HAND FIRST.** There are leftover Hall switches
    and magnets in stock; buying is only justified if a bench test says they
    won't do the job. Three checks, no purchase needed: (i) bare chips are
    ideal, KY-003 breakouts are bench-only and must be run at 3.3 V, never 5 V,
    or their onboard pull-up puts 5 V on a GPIO; (ii) **field strength at the
    real air gap is the only genuine risk** — slide a magnet in until the
    sensor switches reliably, then design the mount at HALF that distance to
    cover vibration, drift and wheel runout; (iii) count what's in the drawer,
    since pulse rate is magnets x wheel rpm.
  - **If topping up: 3 x 2 mm N52 neodymium discs** (~$8–10/50) — NOT the SmCo
    magnets, which are specified and priced for the crank's heat. Four per
    wheel, **all with the same pole facing out** (A3144 is unipolar and only
    responds to one pole). ~220 pulses/s at 40 km/h on 63 mm tires; ~20 at
    walking pace, still fast enough for slip detection. One magnet per wheel
    still works at a quarter the resolution.
  - **Prefer SMALL magnets on the wheels.** A large magnet on a rim adds
    rotating imbalance at 3,400 rpm that will show up in the ride-height and
    load-cell channels later. Save large units for the crank end, where the
    flywheel dominates.
  - **Retention matters:** ~260 g of centrifugal acceleration at 3,400 rpm.
    The load is small (~0.3 N) but oil-soaked CA glue fails in service —
    pocket the magnets in the printed hub with a retaining lip and use epoxy.
- Aero and dynamics sensing: pitot, ride height, load cells. (Pitot note
  2026-08-07: dynamic pressure at 35–40 km/h is only ~60–76 Pa — spec an
  SDP3x-class ±500 Pa digital differential-pressure sensor on I2C, per the
  no-ADC rule; a 1 psi part would waste its whole range.)
- Data logging pipeline (microSD ground truth + WiFi live), coast-down test support

RPM-sensing constraints (established 2026-08-11 from the factory manuals):

- **NEVER put the RPM magnet on the start-belt pulley, its belt, or the
  starter-side pulley.** The start pulley (BOM item 37) contains the one-way
  bearing: once the engine fires the crank outruns it and it trails on bearing
  drag, so anything sensing it reads garbage. It is otherwise the most
  attractive-looking real estate on the engine — this trap would only have
  surfaced on the bench.
- **VESC RPM is now a genuine crank tachometer** and should be the primary
  source: the direct crank-nose mount is rigid, the motor is sensored, so it
  is accurate to standstill at high resolution over a UART link already being
  built. **The A3144 + SmCo path stays, but its job changes to the independent
  cross-check** — a hardware pulse on its own pin survives a firmware fault or
  a VESC/UART failure, which the VESC path cannot.
- **Flywheel conflict RESOLVED 2026-08-14 — EngineDIY answered by video.** The
  clutch is BUILT ON the CDI magnet flywheel: magnet flywheel (rim magnet
  visible, deep cup clearing the start pulley) mounts to the crank nose, a
  backing disc and the blue shoe carrier index onto two drive pins on its face
  (pin-driven, not friction-driven), and the bell rides a bearing on a central
  stub, retained by a nose-end screw. One flywheel serves CDI trigger AND
  clutch carrier; stock flywheel is simply removed. Demonstrated frame-by-frame
  in the vendor video (BUILD-LOG 2026-08-14; video kept local as `111.mp4`).
  The clutch stack lengthens the nose — the only crank-speed face left exposed
  at the end is the small bell-retaining screw, which the MGU-K mount design
  must work around.
- **Free reference geometry:** BadgerJed's CC-BY collection ([thing:6020386](https://www.thingiverse.com/thing:6020386))
  includes four Hall-effect sensor mount STLs for this exact engine, mounted
  with M3 slot-headed screws. Pull these before designing our own — they
  answer "where does a Hall sensor physically fit on an ST-NF2." Useful the
  moment the SmCo magnets land (~mid-August, ~2 weeks before the engine).
- **Packaging reality:** the crank nose is ~21 mm long and already carries the
  start pulley, the flywheel-or-clutch, and the M6 nut. The MGU-K drive, the
  clutch, and any crank-mounted sensor target all compete for it. Draw that
  stack-up as ONE problem — do not solve it three times independently.

### MCU: move to ESP32-S3 (decided 2026-08-11 — pin budget forced it)

The full sensor set does not fit a WROOM-32. Budget: I2C 2, SPI bus 3, chip
selects 3, VESC UART 2, SBUS in 1, pulse inputs 3 (engine RPM + 2 wheel),
throttle servo + ignition kill 2, ToF XSHUT 2, HX711 load cells 5, status
LED + buzzer 2 — **~25 pins, of which ~22 must be output-capable.** A WROOM-32
offers roughly 20 output-capable GPIO once GPIO 6–11 (flash) and TX0/RX0 are
removed and the strapping pins are treated carefully, plus 4 input-only
(34/35/36/39). The shape is wrong, not just the count.

- **Board picked 2026-08-11: Hosyond 3-pack ESP32-S3, N16R8, dual Type-C**
  (Amazon B0F5QCK6X5) — the only one of three candidates whose listing claims a
  genuine Espressif ESP32-S3-WROOM-1 module rather than a clone module, which
  means a certified radio and known flash/PSRAM parts. **Backup: DORHEA 3-pack
  B0CKXJLP4B**, same spec, take it if materially cheaper. (DORHEA 2-pack
  B0CKXJKQ1F is the same board in a smaller pack — no reason to prefer it.)
  Prices not verified; compare at order time.
- **Three boards, not two:** vehicle + bench + spare. A spare ESP32 has already
  justified itself once on this project. Three is what lets the whole sensor
  set go on breadboards at once when the magnets land.
- **Usable pin math on N16R8:** 34 broken out, minus 3 consumed by the octal
  PSRAM (GPIO 35/36/37) = **31**. Flash and debug through the **UART** Type-C
  port rather than the native-USB one to keep GPIO 19/20 free; native USB costs
  those two. 29–31 against a 25-pin budget. Avoid strapping pins 0, 45, 46 for
  critical outputs.
- The S3's different ADC is irrelevant here because of the no-ADC rule.
- Owned WROOM-32s stay useful as bench units and for single-sensor bring-up.
- Regardless of MCU: put the three pulse inputs on input-only pins where the
  part has one (they are inputs, and an open-collector Hall needs an external
  pull-up anyway, so the missing internal pull-up costs nothing). If a build
  ever gets tight again, an MCP23017 I2C expander (~$3) absorbs the slow
  outputs — status LED, buzzer, ToF XSHUT — before any MCU change is needed.

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
- **Braking — DECIDED 2026-08-11: fit a mechanical disc brake on a DEDICATED
  servo.** Crank-side MGU-K + centrifugal clutch means no regen braking below
  engagement, so a friction brake is the only stopping authority at low speed.
  - **NOT the standard nitro two-servo layout.** That layout puts throttle and
    brake on ONE servo through a combined linkage — which in Phase B, when the
    ESP32 takes the throttle servo, would hand it the brake as well. A firmware
    fault would then cost throttle and braking together, the exact failure the
    architecture exists to prevent.
  - **Three servos: steering (RX-direct), throttle (RX-direct in Phase A, ESP32
    in Phase B), brake (RX-direct permanently, its own receiver channel).** The
    FS-R11P has 11 PWM ports, so channels are not a constraint. This makes the
    brake a genuine ESP32-independent stopping path alongside steering.
  - **Costs zero ESP32 pins.** The ESP32 already parses the SBUS stream, so it
    reads brake demand for logging and for blending regen against friction
    braking without a wire to the brake at all.
  - **Architecture: one inboard disc on the driveline**, not per-wheel discs —
    standard nitro practice, brakes both rear wheels through the diff, and far
    simpler on a custom chassis. The disc/pad/cam hardware must be chosen
    TOGETHER WITH the single-speed transmission, since they share a shaft.
    Reference part class: HSP 02044 disc brake set (2 discs, 4 pads, screws,
    ~$5–10) or the 02044-S metal variant — cheap and widely stocked, but do not
    order until the transmission is picked.

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
  **Hard constraint found 2026-08-11: the flywheel hangs 9–10 mm BELOW the
  engine's mounting plane** (~50 mm flywheel on a crank axis only ~15 mm up).
  The dummy block must model that overhang, and any chassis plate or rail needs
  a cutout at the flywheel / start-pulley end — the official base plate has
  exactly such a cutout. Mounting: four M4, 38 mm across the crank axis;
  slot every hole (see design-for-slop).
- **Steering:** Ackermann geometry, bump steer, upright/linkage design — needs
  the wheelbase/track/tire picks from chassis packaging first. Servo purchase
  waits.
- **MGU-K electrical: sized 2026-08-07, bench set ORDERED and DELIVERED Aug 10**
  (see "MGU-K electrical system" above). Remaining is bench work, not
  decisions — and two of them evaporated on 2026-08-16: the hall adapter needs
  NO splice (the shipped cable mates directly) and the end-bell timing is N/A
  (fixed-timing motor). What remains: wire per the EMI ground rules, and set
  and unplug-test every VESC limit BEFORE the first regen event.
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
- Also bench-measure the clutch engagement RPM here (the ratio and
  regen-window math both need the real number). **Spring choice is the
  regen lever, not gearing:** the window fraction is engagement/16,000 rpm
  regardless of ratio — 6k engagement ⇒ regen over the top ~62% of the speed
  range, 9k ⇒ ~44%. Bias springs toward 6–7k if idle stability allows (twin
  idles ~2.5–4k; keep clear margin).
  **CAVEAT 2026-08-11 — the lever may not actually be adjustable.** The ST-NF2
  clutch kit ships with shoes and springs matched to four-stroke rpm, but no
  vendor sells the springs separately or advertises them as tunable, and none
  publishes an engagement RPM. Treat engagement as a number we RECEIVE and
  bench-measure early, not one we dial in. If it lands high, the fallback is
  sourcing generic 1/10 nitro clutch springs and checking fitment — not
  ordering a tuning set, because none is sold. Also measure crank-line moment of
  inertia (spin-down test) — reflected through R²/r² it adds ~10–20%
  effective mass, so accel/energy predictions should use ~3.4–3.7 kg.
  Firmware must cap regen torque below the RPM-dependent clutch capacity and
  cut regen on detected slip (VESC RPM vs wheel-derived RPM divergence) —
  centrifugal clutch slip is self-reinforcing and glazes the shoes.
- **Bench safety rule:** never run brake/regen commands with only a bench PSU
  on the DC bus — regenerated current back-feeds a non-bidirectional supply.
  Keep the traction pack (or a braking resistor) connected.

**Bench rig location and safety rules (settled 2026-08-11).** Location is a
wall-anchored wooden desk — rigid, so coupler runout won't walk the rig.

- **Speeds are higher than they feel.** Free-spinning, the surrogate is the
  fast one: 3900 kv on a fresh 2S ≈ 32,000 rpm no-load; the MGU-K at 2170 kv
  on 3S ≈ 27,000. Coupled 1:1, that is a rigid aluminium coupler at ~30,000
  rpm about arm's length from your face.
- **The hazard is NOT fragments.** A coupler letting go carries roughly half a
  joule — a dropped coin. The two things that actually injure people at a rig
  like this are **entanglement** (a multimeter probe lead, sleeve, or cable tail
  drawn into the coupler) and **the LiPo** (a 3S 5200 mAh pack holds ~58 Wh,
  which is the genuinely dangerous energy on that desk).
- **Guard requirements: anchored, and reach-blocking.** It must not be propped
  or hand-held — a guard that can fall into the rig is worse than none. And it
  must stop you *reaching in*, not merely seeing in; a flat sheet you can lean
  around blocks fragments but not the actual hazard. A clear storage tote
  inverted over the rig with a notch for wiring satisfies both at zero cost.
- **Decide where the probe leads live BEFORE spin-up**, and anchor every
  cable away from the shaft line. Nothing draped.
- **Disconnect must be reachable without crossing the rotating parts** — site
  the pack's XT60 to the side, never behind the coupler.
- **NON-COMBUSTIBLE SURFACE under all charging and pack rest.** The desk is
  wood. LiPo bags contain a venting pack but run very hot. Ceramic tile, steel
  tray or paving slab (~$5–10). Same surface for packs resting between regen
  sessions.
- **Charge rate is power-limited anyway.** The B6ACneo is ~50 W on AC; a 3S
  pack charges at 12.6 V, so it caps near 4 A regardless of setting — about
  0.75C on a 5200 mAh pack. 2.6 A (0.5C, ~2 h) if being gentler; nothing here
  is time-pressured on charge rate.
- **Still to buy: ABC dry-chemical extinguisher (~$25) plus a tub of sand.**
  Deferred since July and now the last open safety item. The goal is not to
  extinguish the cell — you won't — but to stop it igniting a desk that is
  bolted to the wall. Have it before the first 5200 mAh charge.

### Blocked until the engine (or a dimensioned drawing) arrives

**Much of this was answered on 2026-08-11 from the factory manuals + a web
recon pass — see BUILD-LOG. The manuals are public:** [SEMTO ST-NF2](https://cdn.shopify.com/s/files/1/0175/0718/8800/files/SEMTO-ST-NF2.pdf?v=1697620359)
and [OTTO FS-L200AC-OT](https://rc24.mycashflow.fi/files/manuals/Toyan/FS-L200AC-OT_EN.pdf) — the same document under two brands,
60-line BOM and exploded views. **Standing lesson: before opening a vendor
ticket, search for a public manual under EVERY brand name the part is sold
under.**

Answered (all "do not machine to these — physical measurement on arrival
governs"; the verifier widened every band the digger proposed):

- Crank output nose: **8 mm parallel shank** (not tapered) stepping to an M6
  thread. **Drive is a ROUND PIN in a longitudinal round-ended groove** — BOM
  item 04 "Round pin (φ2X12)", qty 2 — not a flat, not a spline. Pin-driven
  AND nut-clamped. Exposed nose ≈ 21 mm total, thread ≈ 8.5 mm.
- Crank centerline **≈ 15 mm ±1.5** above the mounting face.
- Mounting: **four M4 blind holes**, **38 mm across the crank axis** (solid),
  ~40–42 mm along it (medium confidence).
- **The flywheel hangs 9–10 mm BELOW the mounting plane** (~50 mm flywheel on a
  ~15 mm axis) — confirmed three ways. **Chassis plates and rails need a cutout
  at the flywheel / start-pulley end.** Feeds chassis packaging directly.
- There is no second shaft end: start pulley and flywheel share the OUTPUT end;
  the far end is the timing/fan end.

Still genuinely unknown:

- **M6 thread pitch (×1.0 vs ×0.75) and hand (LH/RH)** — unpublished anywhere;
  the CAD thread is decorative (~0.32 mm pitch) so it cannot be read off the
  drawings. A left-hand nose would wreck a purchased nut or hub.
- **Mounting hole pattern TOPOLOGY** — rhombus (two holes on the crank
  centerline fore/aft, two at mid-length across) per a 3D scan, vs rectangle
  38 × 40 per a printed mount and the official base photo. **Unbroken tie. Do
  not order machined parts against either reading.**
- Whether both φ2 pins are on the output nose, and their angular/axial layout.
- Real torque curve and vibration signature (needed for engagement tuning).

**Design-for-slop is now vendor-endorsed:** the manual says *"If the existing
holes on the engine bracket cannot be aligned perfectly with the engine
mounting holes, do not force the installation... it is recommended to purchase
adjustment pads"*, and an ST-NF2 owner reports the screws not lining up on the
official base. Slot every hole.

**Dead idea — do not resurrect:** the 8 mm ↔ 3.175 mm rigid clamp coupler.
The nose is pin-driven with only ~21 mm exposed, most of it occupied by the
pulley, flywheel/clutch and nut. Attachment is a face drive, not a shank clamp.

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

**Cut down hard on 2026-08-11** — the web recon answered the freewheel
question, the clutch-kit question, the drive feature, the centerline height and
most of the mounting pattern. Do NOT re-ask those; don't spend vendor goodwill
re-establishing what the manuals already settle. Four items remain, and only
the first two can cost machined parts:

- **M6 thread pitch and hand.** "Is the crankshaft output thread M6 × 1.0 or
  M6 × 0.75, and is it right-hand or left-hand?" Genuinely unpublished — not in
  either manual, no BOM line, no listing, and the CAD thread is decorative so
  it can't be measured off the drawings. A left-hand nose would wreck a
  purchased nut or hub.
- **Mounting hole pattern topology.** Phrase it so a shop hand can answer
  without a drawing: "Looking at the bottom of the crankcase, are the four M4
  holes at the corners of a rectangle, or are two on the crankshaft centerline
  (front and rear) with the other two out at the sides at mid-length? A phone
  photo of the bare engine's underside would answer this completely." A 3D scan
  says rhombus, a printed mount and the official base photo say rectangle
  38 × 40 — unbroken tie.
- ~~Clutch bore~~ **CLOSED as an order-blocker 2026-08-14.** Keyway concern
  died 2026-08-13 (retention is shaft/screw, not key). The bore number was
  never given, but the vendor video shows the kit hand-assembling onto a real
  ST-NF2, which retires the mismatch risk; caliper on arrival governs
  (design-for-slop). Torque path answered visually: pin-driven shoes off the
  flywheel face, set-screwed + nut-clamped flywheel — not a friction-only M6
  clamp. Nothing left to ask.
- ~~Clutch vs CDI trigger magnet~~ **CLOSED 2026-08-14 — answered by vendor
  video.** The clutch mounts ON the CDI magnet flywheel (pin-driven shoes off
  the flywheel face, bell on a bearing); one flywheel does both jobs. See
  BUILD-LOG 2026-08-14. Compatibility no longer blocks the clutch order.
  **Still open from that round: the bell gear tooth count and module** — the
  bell in the video shows no visible gear or V-groove, so also ask WHICH
  variant was filmed.
- **One-way bearing identification.** "Part 37, the Start belt pulley
  component: what is the designation and size (bore × OD × width) of the
  one-way bearing pressed into it?" Only matters if we ever want to defeat or
  service it; it would also independently confirm the 8 mm shank.
- Also worth asking: are BOTH φ2 × 12 round pins (BOM item 04) on the output
  nose, and what is their layout?

Ask the lab:

- **Printer:** which machine, which materials (is polycarbonate real), and can
  it be used for a personal project?
- **PCB fab:** what process and constraints — max board size, layers, minimum
  trace/space, accepted design files (KiCad?), turnaround. The telemetry board
  gets designed to these limits.

Check what's on hand / decide:

- ~~Radio gear~~ **CLOSED — ordered 2026-08-07 ($92.11 incl. tax), delivered
  Aug 10.** Flysky FS-G7P+ + FS-R11P (11 PWM ports + dedicated serial port,
  simultaneous by hardware design; documented SBUS failsafe flags). Wiring map,
  day-one failsafe ritual, and bench acceptance tests in `docs/radio-setup.md`.
  No longer a question — it is a bench task: run the failsafe ritual.
- ~~Target top speed~~ **decided 2026-08-07: 40 km/h** — see the Transmission
  bullet in Powertrain architecture; math trail in BUILD-LOG (2026-08-07).
- ~~Brake~~ **CLOSED 2026-08-11: active braking, dedicated servo, inboard
  disc.** See the braking bullet under Driver-command architecture. Remaining
  is design work, not a question: pick the disc/pad hardware alongside the
  transmission, and size the brake servo.
- ~~Bench rig location and guarding~~ **CLOSED 2026-08-11.** Wall-anchored
  wooden desk — rigid enough that a coupler-runout imbalance won't walk the
  rig. Bench rules now documented under "Bench surrogate approach"; one open
  purchase remains (ABC extinguisher, ~$25, still unbought since July).
- Optional but useful: a comfort number for the combined bench-set order — it
  steers component tier (e.g. VESC-clone vs genuine).

Owed by the work itself (not research): wheelbase / track / wheel-tire package
from chassis packaging; pack voltage and capacity from MGU-K sizing.
