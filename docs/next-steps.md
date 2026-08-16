# Next steps — full list, with what each one needs

Snapshot 2026-08-16. Every open thread on the project, in rough dependency
order, each labelled with the **pieces required** — parts, information, or a
prior step. The point of the "needs" column is that most stalls on this
project have been missing *information*, not missing parts.

Legend: **READY** = every piece is in hand, start now · **SOON** = waiting on a
delivery with a known date · **BLOCKED** = waiting on the engine, or on a
measurement that needs the engine.

---

## A. Bench bring-up (the current work)

Detail in [bench-bringup.md](bench-bringup.md). Both safety gates closed
2026-08-16.

| # | Step | Pieces needed | Status |
|---|---|---|---|
| A1 | Label the two packs 3S/2S; log resting cell voltages | Packs, paint pens, multimeter | **READY** |
| A2 | Radio failsafe ritual + acceptance Tests A and B | FS-G7P+, FS-R11P, 1–2 servos, electronics rail, [radio-setup.md](radio-setup.md) | **READY** |
| A3 | Arrival-test all three ESP32-S3 boards; record eFuse MACs | 3× ESP32-S3, USB-C cable (**UART port, not native USB**), arduino-cli | **READY** |
| A4 | Splice JST-ZH → JST-PH sensor adapter; verify 5 V/GND before connecting motor | Motor + VESC pigtails, crimps/heat-shrink, multimeter | **READY** |
| A5 | Set motor end-bell timing to the zero mark | MGU-K, hex key | **READY** |
| A6 | Bolt MGU-K to bracket; clamp bracket to bench | Motor, blue bracket, clamps | **READY** |
| A7 | VESC bring-up: firmware **then** config, FOC + hall detection, verify smooth two-direction start | A4–A6, 3S pack (storage charge fine), VESC Tool, guard | **READY** |
| A8 | Confirm pole pairs = 2 empirically (one hand turn vs reported ERPM) | A7 | **READY** |
| A9 | Sanity-check motor NTC reading at room temperature | A4, A7 | **READY** |
| A10 | Set every limit; **unplug-test** the command timeout; log measured stop time | A7, VESC Tool | **READY** |
| A11 | Couple surrogate 1:1; first sustained regen test | A10, coupler, 2S pack, servo tester, guard, fire kit, non-combustible surface | **READY** |
| A12 | Measure coupled spin-down inertia (feeds the ~10–20 % effective-mass adder) | A11, logging | **READY** |
| A13 | Measure deliverable assist power vs RPM on a sagged 3S pack (checks the 17.5T pick against ~390–470 W) | A11, charged packs, logging | **READY** |

## B. Firmware

| # | Step | Pieces needed | Status |
|---|---|---|---|
| B1 | ~~ERPM conversion + slip-cut module~~ **DONE 2026-08-16** — [Driveline.h](../firmware/libraries/Driveline/Driveline.h), 30 desktop checks passing | — | **DONE** |
| B2 | Flash [slip-cut-test](../firmware/slip-cut-test/slip-cut-test.ino), confirm boot self-test PASSes on real hardware | A3 | **READY** |
| B3 | Breadboard both wheel Hall pickups against B2's live mode; spin by hand, watch verdicts | A3, 2× A3144, 2× 10 k, magnets, breadboard | **READY** (uses on-hand stock first) |
| B4 | VESC UART link: read RPM / phase current / battery V,I; replace the simulated crank RPM in B2 | A7, VescUart, ESP32-S3 | **READY** |
| B5 | SBUS parse: driver demand, failsafe flag bits, sentinel channel, 100 ms frame timeout | A2, A3 | **READY** |
| B6 | Logging pipeline: microSD ground truth + WiFi live | A3, SD module (tested 07-26), `secrets.h` (gitignored) | **READY** |
| B7 | Assist/regen state machine (Phase A: commands VESC only, never the throttle) | B4, B5, B6 | **READY** |
| B8 | Duty timer + motor-NTC foldback for burst assist | A9, A13, B4 | after A13 |
| B9 | Pre-run checklist in firmware/docs, incl. **start regen sessions ≤4.0 V/cell** (no graceful HV regen taper exists) | — | **READY** |
| B10 | Engine RPM pickup as the independent cross-check to the VESC tach | SmCo magnets (~mid-Aug), engine | **BLOCKED** |
| B11 | Tune slip thresholds against real clutch behaviour | Measured engagement RPM ⇒ clutch + engine | **BLOCKED** |

## C. Chassis, packaging, mechanical

| # | Step | Pieces needed | Status |
|---|---|---|---|
| C1 | Dummy engine block at 11.2 × 9 × 9.2 cm / 535 g, **modelling the 9–10 mm flywheel overhang below the mounting plane** | Printer access, [chassis-packaging.md](chassis-packaging.md) | **READY** |
| C2 | Wheel/tire pick ⇒ wheelbase and track | C1 | **READY** |
| C3 | Final tooth counts for the single-speed | C2 (tire diameter) + measured engagement RPM + clutch pinion module | **BLOCKED** |
| C4 | Measure clutch pinion with calipers on arrival: mod = OD/(N+2) | Clutch (ordered $27.99) | **SOON** |
| C5 | Order spur gear to match | C3, C4 | after C3/C4 |
| C6 | Inboard disc brake hardware (HSP 02044 class) — pick **with** the transmission, shared shaft | C3 | after C3 |
| C7 | Brake servo sizing + third servo channel | C6 | after C6 |
| C8 | Crank-nose stack-up: clutch + MGU-K drive + sensor target as **ONE** problem (~21 mm nose) | Engine, clutch, C1 | **BLOCKED** |
| C9 | Steering: Ackermann, bump steer, uprights, linkage | C2 | after C2 |
| C10 | Slot every mounting hole (vendor-endorsed design-for-slop) | applies throughout | — |
| C11 | Telemetry PCB: bench perfboard now, vehicle board after packaging freezes | B-series breadboard, lab PCB answers | perfboard **READY** |

## D. Purchases still open

| # | Item | Approx | Needed for | Status |
|---|---|---|---|---|
| D1 | ~~ABC extinguisher~~ Kidde FA110G | $24.97 | A11 | **DELIVERED** |
| D2 | Sand tub + non-combustible charging surface | $5–10 | A11 | available at bench |
| D3 | Magnets — 3×2 mm N52 discs, only if on-hand stock fails the bench test | $8–10 | B3 | test first |
| D4 | Spur gear | ~$10 | C5 | gated on C3/C4 |
| D5 | Brake disc/pad set | $5–10 | C6 | gated on C3 |
| D6 | Belt + pulleys for the rear power module | $20–30 | power-module build | flagged, not ordered |
| D7 | Engine-arrival consumables: washer thermocouples, starter/CDI battery, fuel, MP1584 3-pack | $120–160 + $7 | engine unboxing | **BLOCKED** |

## E. Questions still owed by other people

| # | Question | Ask who | Why it matters | Status |
|---|---|---|---|---|
| E1 | M6 crank thread pitch (×1.0 vs ×0.75) **and hand** | EngineDIY | A left-hand nose wrecks a purchased nut/hub | open |
| E2 | Mounting hole topology — rectangle 38×40 or rhombus? | EngineDIY (phone photo of the underside answers it) | Unbroken tie; blocks ordering machined mounts | open |
| E3 | Clutch bell gear tooth count + module, and which variant the video filmed | EngineDIY | Would let C5 move before the clutch lands | open (C4 self-serves it) |
| E4 | One-way bearing designation in BOM item 37 | EngineDIY | Only matters if we ever service/defeat it | low priority |
| E5 | Both φ2×12 pins on the output nose? Layout? | EngineDIY | Feeds C8 | low priority |
| E6 | Printer: which machine, which materials, is polycarbonate real, personal use OK? | Lab | Gates C1 material assumptions | open |
| E7 | PCB fab: process, max size, layers, min trace/space, file formats, turnaround | Lab | C11 designs to these limits | open |

## F. Waiting on the engine (late August)

Nothing here can be pulled forward; listed so it isn't rediscovered as new work.

- Unboxing inspection checklist (doc 2 §2)
- Glow/nitro break-in and tuning **before** the CDI conversion
- HV ground strap fitted **before** the CDI kit (known kit defect — otherwise it
  destroys the Hall sensor and ignition module)
- Cam box greased before first start; short starting bursts; outdoor running only
- Bench-measure clutch engagement RPM ⇒ unblocks C3, B11, and the regen-window math
- Harness connector ID ⇒ unblocks the starter-battery buy
- Real torque curve and vibration signature

---

## The critical path, in one line

**C2 (wheel/tire pick) and the measured clutch engagement RPM are the two
numbers the most work is queued behind** — between them they gate the
transmission, the spur, the brake hardware, and the regen-window math.
C2 is READY today and needs nothing but bench time; engagement RPM is
engine-gated. So: **do C1/C2 while the bench work runs.**
