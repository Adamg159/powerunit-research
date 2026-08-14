# Hybrid RC Power Unit — Build Log

Running log of progress, decisions, and problems for the 1/10-scale hybrid powertrain project
(Toyan FS-L200AC 7cc twin + MGU-K-style motor-generator + ESP32 telemetry/ignition control).
Reference documents: [01-feasibility-proposal.html](01-feasibility-proposal.html) ·
[02-phase1-status-and-arrival-plan.html](02-phase1-status-and-arrival-plan.html) ·
[03-build-companion.html](03-build-companion.html)

Entry format: date · what happened · problems hit · how they were resolved (or current status).

---

## 2026-07-21 — Phase 1 orders placed

- Engine order placed with EngineDIY: SEMTO ST-NF2 / Toyan FS-L200AC 7cc twin-cylinder
  four-stroke kit with starter bundle (~$290) + gasoline/CDI spark conversion kit (~$113).
- Telemetry/control electronics order placed: $204.79 actual (full itemized bill of materials
  in document 2, Section 4). Includes 3× ESP32, 2× MAX31855, Hall sensors, SmCo magnets,
  IMUs, SD modules, bucks, EMI parts, perfboard.
- Home soldering setup ordered separately (~$119): Miniware TS101 iron, 65W USB-C PD supply,
  solder, wick, flux pen, IPA. Tracked as durable tooling, outside the Phase 1 parts total.
- Deferred until engine arrival: washer thermocouples (need measured ring size), starter/CDI
  battery (need harness connector ID), balance charger, consumables, bench materials
  ($120–$160 est.).

## 2026-07-23 — Workspace set up; first arrivals; TS101 verified genuine

**Workspace:** The three planning documents were copied from laptop artifacts into
`G:\PowerUnit Research` as standalone HTML files. This build log started.

**Arrived today:** 65W USB-C PD supply, 99% IPA, J-B Weld HighHeat epoxy, flux pen,
TS101 soldering iron.

**TS101 authenticity check (from document 3 arrival checklist) — PASSED:**
- Iron enters DFU mode correctly (front button + plug in), shows "DFU" on screen,
  mounts as a `TS101_DFU` USB drive. Bootloader v1.06, stock firmware v2.11 —
  both are real current Miniware versions. Verdict: genuine unit.

**Problem: IronOS v2.23 flash fails — root-caused to the OS, not the iron.**
- Every flash attempt (EN hex ×4 across PowerShell and Explorer copy engines, ES hex as a
  language-build diagnostic) returned `.ERR`; one attempt silently discarded the file and
  booted back to stock v2.11.
- Root cause per Miniware's support forum: the TS101 bootloader cannot be flashed from
  Windows 11 24H2 or newer; the desktop runs build 26200 (25H2). Users in the same thread
  succeeded immediately from older-Windows machines.
- Resolution: **deferred by choice.** Stock v2.11 heats and regulates fine for Phase 1
  soldering. If IronOS is ever wanted: flash from the laptop (older Windows) with a
  USB-A-to-USB-C cable into a direct port, drag `TS101_EN.hex` from the
  [IronOS releases page](https://github.com/Ralim/IronOS/releases), look for `.RDY`.

## 2026-07-24 — Workspace now synced via GitHub

- Workspace turned into a git repository and pushed to
  **https://github.com/Adamg159/powerunit-research** (public — chosen deliberately since the
  project is a portfolio piece; rule adopted: no secrets in the repo, WiFi credentials will
  live in a gitignored `secrets.h` once firmware work starts).
- Workflow on both machines: pull before working, commit + push after. Laptop still needs
  its one-time `git clone`.
- Problem hit: first push failed with a stale legacy GitHub token in Windows Credential
  Manager, and Claude Code's shell disables git's interactive prompts. Fixed by setting
  `GIT_TERMINAL_PROMPT=1` for the push so Git Credential Manager could do a fresh browser
  sign-in; new credential cached, pushes work normally now.

## 2026-07-24 — Main electronics arrived (two stragglers in transit)

Delivery received and photographed laid-out before assembly (checklist item 1 — photo to be
committed to `photos/`). Confirmed present: 3× ESP32, 1× MAX31855, 10× A3144 Hall sensors,
3× MPU-6050, 3× SD modules, Lexar microSD, 3× MP1584 bucks, 2× URGENEX 2S packs + charger,
shielded cable, ferrites, grommets, perfboard kit, both capacitor kits, 1N4148s, TVS diodes,
jumper wires, plus the full soldering bench (TS101, 65W supply, solder, wick, flux, IPA,
bonus silicone mat).

**Short/delayed items — neither blocks anything:**
- 1× MAX31855 (second of two) stuck in shipping; due in a couple of days, before the engine.
  One board covers all bench testing; second needed only for per-cylinder logging at first start.
- SmCo magnets due shortly after the engine — only needed once there's a flywheel to mount to.

Arrival-day verification checklist (document 3, Part 2):

- [x] Flash blink + WiFi-scan sketch on all 3 ESP32s — **all three PASS** (flash write
      verified, LED blink, WiFi scan sees 3 networks at up to −49 dBm). All are genuine
      ESP32-D0WD-V3 rev 301, dual-core 240 MHz, 4 MB flash. Board identity by eFuse MAC
      (boards unlabeled for now; any board re-identifiable by plugging in and reading serial):
      - Board 1: `58:2A:BD:7D:A7:D8`
      - Board 2: `58:2A:BD:7C:AA:E8`
      - Board 3: `58:2A:BD:7E:33:EC`

      Test sketch lives at `firmware/arrival-test/arrival-test.ino` (repo's first code).
      Problems hit and fixed along the way: Windows 11 has no inbox CP2102 driver
      (code 28 on first plug-in) — installed the official Silicon Labs CP210x driver, which
      created COM4 and covers all future CP2102 boards; and `WiFi.macAddress()` reads
      all-zeros right after `WiFi.mode()` — switched to `ESP.getEfuseMac()`, which reads the
      factory MAC from eFuse with no WiFi dependency.
- [x] Dial each MP1584 buck to 5.0 V with a multimeter BEFORE it touches anything
      — **done 2026-07-31: 2 of 3 tuned to ~5 V; third board fried during tuning.**
      Two working bucks cover both planned rails (Phase 1 telemetry logger + Phase 2
      ignition controller); spare lost. See the 07-31 entry.
      — **update 2026-07-25:** multimeter + clip leads arrived. New gap caught while
      planning the hookup: no mating connector for the battery packs' discharge lead.
      Identified the pack's connector as **JST-RCY** (red 2-pin discharge; small white
      JST-XH is balance/charging only — never used for power). Ordered SIM&NAT 20 AWG
      silicone JST-RCY pre-wired pigtails, 10 pairs ($7.49), arriving next day — this was
      the "connector adapter assortment" insurance line from document 2's deferred list.
      Plan unchanged: pigtails soldered to buck IN pads, battery clicks in, clip leads on
      OUT pads, dial to 5.00 V. Spare RCY pairs will standardize the buck→ESP32 power
      connections too. Meanwhile sensor soldering + tests proceed on USB power.
      — **update 2026-07-28:** last tooling gap closed, a ~1.5 mm precision blade for the
      trim pot itself (see the 07-28 entry).
      — **blocked: no multimeter on the bench.** Ordered 2026-07-24, arriving next day:
      AstroAI True RMS 6000-count auto-ranging DMM ($34.99) + DIANN banana-to-alligator
      test leads ($5.99, 4 mm banana fits the AstroAI's standard jacks) — both bench-stock
      tooling, like the soldering kit. 6000 counts gives millivolt resolution on the 6 V
      range — right tool for setting 5.00 V trim pots. Plan for the bucks: solder short
      pigtails/header pins to each buck's IN/OUT pads first (first soldering rep), clip on
      hands-free, then dial the pot. Bucks stay in the bag
      until it arrives; nothing powers through them unset. Sensor-board first power comes
      from ESP32 USB rails instead.
- [x] Run h2testw full write/verify on the Lexar 32 GB microSD — **PASS, card is genuine**
      (done on the laptop; desktop lacks a card reader). Details in the 07-24 SD entry below.
- [x] Solder headers onto MPU-6050 boards (no friction fit on a vibration sensor)
      — **all 3 soldered and tested, all PASS at 0x68.** See the 07-26 entry.
- [ ] Breadboard a Hall sensor (5 V supply, 10 k pull-up to 3.3 V); polarity-test and
      paint-mark each SmCo magnet's working face (A3144 is unipolar) — waits on magnets
- [x] Wire the MAX31855, confirm sane room-temp reading and fault bits — **PASS** (cold
      junction 26.5 °C, OC fault correctly set with no probe). See the 07-26 entry.
- [x] SD module VCC from 5 V rail (never 3.3 V); drop SPI clock if writes are flaky
      — **PASS at 10 MHz** (25 MHz failed on jumper wires, as expected). See the 07-26 entry.

## 2026-07-24 — Correction: IronOS flash fallback is invalid on both machines

- The 07-23 entry's resolution ("if IronOS is ever wanted: flash from the laptop, older
  Windows") doesn't hold — the laptop turns out to run the same Windows build as the
  desktop (Windows 11 Pro, build 26200 / 25H2), so the bootloader's 24H2+ flashing
  limitation applies to **both** machines. Discovered while setting up the laptop's
  repo clone.
- Valid paths if IronOS is ever actually wanted: boot a **Linux live USB** on either
  machine and drag `TS101_EN.hex` onto the `TS101_DFU` drive from there (the failure is
  a Windows copy-engine issue, per the Miniware forum), or borrow any pre-24H2 Windows
  or macOS machine for five minutes.
- No action for Phase 1 — stock v2.11 heats and regulates fine; this stays a
  someday-item, now with an accurate map.

## 2026-07-24 — Lexar 32 GB microSD verified genuine (h2testw PASS)

- Full write/verify run on the laptop (Realtek PCIE card reader): h2testw 1.4 wrote and
  read back 29,837 of 29,838 MByte — **"Test finished without errors."** Full advertised
  capacity is real; zero data mismatches. Test files deleted afterward; card is empty
  (FAT32, ~29.1 GiB free) and cleared for engine data logging.
- Speeds: **62.3 MByte/s write, 134 MByte/s read** — far beyond logging needs, and the
  read speed means the laptop reader runs the card above stock UHS-I rates. (ESP32 SPI
  will be the bottleneck in the logger, not the card.)
- The "only 29,837 of 29,838 MByte tested" warning is normal FAT32 overhead, not a defect.
- Problems hit (all around *getting* the tool, none with the card): heise.de download links
  are single-use tokenized and refuse non-browser clients (403), so curl/WebFetch couldn't
  pull the zip — resolved by downloading through a real browser session. h2testw itself is
  GUI-only with no CLI, so the run was click-through (target E:, all available space,
  Write+Verify); progress was monitored from the card side by watching `.h2w` files land.
- Tool kept at `C:\Users\Adamg\Downloads\h2testw\` on the laptop for future card checks.

## 2026-07-26 — microSD SPI module verified end-to-end on the ESP32 (PASS)

- Ran `firmware/sd-test/sd-test.ino` on an ESP32 DevKit with the microSD module on jumper
  wires (VCC to VIN/5 V, SCK 18, MISO 19, MOSI 23, CS 5) — **PASS**. Card mounted, type 3
  (SDHC), size reported **29.15 GB** (matches the h2testw result), file written, append
  survived, and both lines read back with matching content.
- **25 MHz SPI failed, 10 MHz mounted.** The sketch's clock-ladder retry
  (25 → 10 → 4 → 1 → 0.4 MHz) did exactly what it was written for. This is the jumper
  wires, not the module or the card — unshielded flying leads won't hold 25 MHz SPI.
  Expect this to improve once the logger moves to perfboard with short traces; if it
  doesn't, 10 MHz is still ~10× the bandwidth the logger needs.
- **Port is COM8 on this machine, not COM4.** The Silicon Labs CP210x driver assigns a
  COM number per USB port, so the same board enumerates differently depending on which
  port it's plugged into. Nothing to fix — just check Tools → Port rather than trusting
  the number in `firmware/README.md`.
- Arduino IDE board selection confirmed: **ESP32 Dev Module** (equivalent to the
  `esp32:esp32:esp32` FQBN the arrival tests were flashed with). Not "ESP32-WROOM-DA
  Module" — that's the dual-antenna variant.
- Storage side of the telemetry chain is now proven end-to-end: card is genuine (h2testw),
  module works, and the ESP32 can write and re-read log files. Remaining sensor arrival
  tests: MPU-6050 and MAX31855 (both need headers soldered), Hall (needs breadboarding).

## 2026-07-26 — All 3 MPU-6050 IMUs soldered and live on I2C (PASS)

- Headers soldered onto **all three** GY-521 boards (first real soldering rep on project
  hardware, TS101 on stock v2.11) and each one tested with
  `firmware/mpu6050-test/mpu6050-test.ino` (VCC→3V3, GND, SDA→21, SCL→22) — **all 3 PASS**.
  At rest: accel magnitude **0.99–1.00 g**, gyro settling to ~1–3 dps of uncalibrated
  bias, die temp 28.1–28.5 °C (a few degrees over ambient, as the MPU-6050 always reads).
  Values track handling — a board was picked up and waved mid-run and the numbers
  followed, so nothing is stuck or fabricated.
- **I2C address is 0x68 with AD0 left unconnected** — these GY-521 boards pull AD0 low
  on-board, so the sketch's `const uint8_t MPU = 0x68` is correct as written and no
  jumper is needed. The address is a hardware property (AD0 low → 0x68, AD0 high → 0x69);
  it can't be set in software, and the sketch's I2C scan is how you read it back.
  Only relevant if two IMUs ever share one bus, which the telemetry design doesn't do.
- **Gyro clipped at ±250.1 dps during handling** — that's the rail of the MPU-6050's
  default ±250 dps full scale (32767/131 = 250.13), not a fault. Defaults (±2 g,
  ±250 dps) are right for an arrival test but will saturate badly on a running engine.
  The real logger should widen both: `writeReg(0x1B, ...)` for gyro and
  `writeReg(0x1C, ...)` for accel — ±2000 dps and ±16 g are the usual engine choices.
- Problem hit first: with nothing wired, the sketch prints plausible-looking garbage
  rather than failing — `WHO_AM_I: 0xFF`, `accel -0.00`, and a very believable
  **36.5 °C**. All of it is `Wire.read()` returning −1 on an empty buffer
  (−1/340 + 36.53 = 36.53). The empty `I2C scan:` line was the only honest indicator.
  Worth hardening the sketch to bail on a failed WHO_AM_I instead of streaming fake data.

## 2026-07-26 — MAX31855 thermocouple amp verified (PASS)

- Headers soldered and board tested with `firmware/max31855-test/max31855-test.ino`
  (VIN→3V3, SCK→18, DO→19, CS→5) — **PASS**. Cold-junction reads a sane
  **26.44–26.56 °C**, and the fault register reports `fault=1 SCV=0 SCG=0 OC=1`:
  open-circuit only, nothing shorted to Vcc or GND. That fault detection is the reason
  this chip was chosen over the MAX6675, and it works.
- The `thermocouple 2047.75 C` line is the rail, not a reading — the thermocouple field
  is 14-bit signed at 0.25 °C/LSB, so 8191 × 0.25 = 2047.75. An open input floats to full
  scale; the OC bit is what says to disregard the number. Expected and documented in the
  sketch header.
- **Still unverified: the thermocouple measurement path itself.** This run proves the
  cold-junction sensor and the fault logic. Shorting T+ to T− with a clip lead should
  clear OC and pull the thermocouple reading down to ~internal temp (a short acts like a
  junction at the connector) — worth doing before the washer thermocouples arrive with
  the engine, since it's the only way to exercise the analog front end without a probe.
- Inventory: this is the only MAX31855 on the bench; the second of two is the shipping
  straggler from the 07-24 entry, and isn't needed until per-cylinder logging at first start.

## 2026-07-28 — Two tooling gaps closed while setting up the buck tuning

Both caught while actually laying out the buck-tuning bench rather than reading about it —
the same pattern as the JST-RCY connector gap on 07-25.

- **Precision screwdriver set** — AXTH 25-in-1 magnetic bearing-steel set ($9.99). The MP1584
  trim pot is a tiny surface-mount potentiometer needing roughly a 1.5 mm blade; the bench had
  nothing that small, and forcing a larger driver into it is the standard way to destroy the
  wiper and scrap the module. Magnetic tips also matter for the M2/M3 hardware coming with the
  engine and for the ESP32 enclosure work later.
- **Wire stripper / crimper** — WGGE WG-015 8-inch multi-tool ($15.99). Needed for the pigtail
  and sensor harness work now starting: the 20 AWG silicone JST-RCY leads and 22–26 AWG sensor
  wire both want gauge-matched strip notches. Stripping silicone-insulated wire with side
  cutters or a blade nicks strands, which is exactly the kind of hidden defect that shows up
  later as an intermittent power fault under vibration.

Both are durable bench tooling, tracked like the soldering kit and multimeter — outside the
Phase 1 parts total. Documents 2 and 3 updated: document 2 gained a "Bench hand tools" row
(multimeter, leads, screwdrivers, stripper — ~$67) plus the previously unrecorded JST-RCY
pigtail line, and document 3's buck checklist item now spells out the full hookup (pigtail on
IN, alligator clips on OUT, precision blade on the pot).

Buck tuning is now fully equipped; it proceeds as soon as the pigtails and this tooling land.

## 2026-07-31 — Buck tuning done: 2 of 3 MP1584s at ~5 V, one board lost

- Buck tuning finally happened with the full kit (JST-RCY pigtail on IN, alligator clips
  on OUT, precision blade on the pot, AstroAI DMM): **two bucks dialed to ~5 V and ready**.
- **Problem: the third board was fried during tuning** — an accidental short while
  hooked up; board is dead and scrapped. Inventory is now 2 working bucks, 0 spares.
- Coverage check against the plan: only two 5 V rails are ever called for — the Phase 1
  telemetry logger and the Phase 2 ignition controller (same one-active-plus-spare logic
  as the 3× ESP32 buy). So **nothing is blocked**; Phase 1 needs just one buck.
- Decision: restore the spare, but no rush — MP1584 3-packs are ~$7, so ride it on the
  next parts order (deferred engine-arrival items) rather than paying shipping for a
  standalone order. Until then the bench runs with zero buck margin, which is acceptable
  for bench work on USB-powered sensor tests.

## 2026-08-05 — Engine backorder confirmed; plan re-sequenced to build around it

- **The engine is on backorder, roughly 3–4 weeks — late-August arrival expected.**
  The vendor offered a swap to a CISON L4 (~$969) and it was **declined**: it would
  have consumed the entire budget on the least novel component of the project. A
  dimensioned spec sheet for the ST-NF2 was requested from the vendor instead; if it
  arrives before the engine, the crank-interface and mount design unblock early.
- Problem being fixed today: the project spent the past week bottlenecked behind
  engine arrival when most of the work never needed the engine. A staged-build plan
  was drafted (on the laptop) and is now merged into the repo as **`CLAUDE.md` at the
  repo root** — the governing project context from here on; docs 1–3 stay as the
  detailed reference and win nothing on conflict.
- **Now-unblocked work:** steering geometry, MGU-K electrical sizing, firmware
  (telemetry pipeline + assist/regen state machine), transmission ratio math off the
  4,000–16,000 rpm band, and chassis packaging against a dummy block at the engine's
  11.2 × 9 × 9.2 cm / 535 g envelope. A bench surrogate (second brushless motor as
  crank stand-in) will let the full control loop be validated end to end before the
  engine lands — engine arrival becomes a driver swap, not a rewrite.
- **Decisions made while reconciling the new plan with the repo docs** (a full
  cross-check turned up several conflicts; each was resolved explicitly):
  - **Ignition-timing ECU deferred** out of the core plan, parked alongside the v2
    locked-clutch stretch goal. The engine runs on the stock fixed-timing CDI
    indefinitely and the success criteria drop the mapped-ignition requirement.
    Side effect: ESP32 #3 and the second tuned buck revert to true spares — the
    07-31 "zero buck margin" concern resolves itself, though the ~$7 MP1584 3-pack
    can still ride the next order.
  - **Bench-set purchases authorized now** without per-item check-ins: MGU-K motor,
    VESC-class controller, surrogate motor + drive, gearbox, and the traction
    pack/BMS the rig needs. Steering hardware and aero sensors stay design-only
    until separately approved.
  - **MGU-K coupling left open** — belt off the starter-belt interface (as docs 1/3
    describe) vs direct crank-nose mount (as the new plan's blocked list implies) —
    until the vendor spec sheet decides it. Motor sizing proceeds regardless.
  - **No dedicated current sensor:** "current sensing on the MGU-K" = the VESC's
    UART-reported motor/battery currents, as doc 1 always assumed.
  - **Engine-arrival sequencing unchanged:** glow break-in before CDI conversion,
    ground strap before the CDI kit, inspection checklist at unboxing, deferred
    purchases ($120–160) still trigger there. "Slot-in-able" means the chassis
    interfaces are ready, not that break-in is skipped.
- Open questions carried in CLAUDE.md: gearbox selection, whether the starter
  kit / CDI kit / SmCo magnets shipped separately or are held with the engine
  (magnets gate the last open bench test), surrogate motor spec, printer material
  fallback, steering geometry inputs (wheelbase/track/tires), radio architecture,
  and traction-pack charging.

## 2026-08-05 — Q&A round: RWD made explicit, single-speed decided, one-way bearing risk found

Adam answered the morning's open questions; each answer was worked through and the
resulting recommendations adversarially reviewed before landing in CLAUDE.md.

- **Rear-wheel drive is now explicit** — it had been assumed but never written
  down anywhere. Front wheels steer only, which keeps the upright/Ackermann work
  simpler (no front driveshafts).
- **Transmission: single-speed for v1.** The remembered concern ("single-speed
  makes brake regen harder") is real but small: kinetic energy scales with v², so
  ~85–90% of recoverable energy sits above the clutch drop-out speed even with one
  gear. The clincher against a two-speed: off-the-shelf 1/10 nitro two-speeds
  shift centrifugally and drive first gear through a one-way bearing — the wheels
  can't backdrive the input in exactly the gear that was supposed to widen the
  regen window. Ratio math will include the v2 locked-clutch sanity check (engine
  reaches firing RPM at a sane road speed) so v2 never forces a re-gear.
- **Problem found before it happened: the starter-belt interface may freewheel.**
  Starters crank engines; engines never drive starters — so the stock belt
  interface plausibly contains a one-way bearing. If it does, the crank can never
  drive the MGU-K through it: zero brake regen and zero engine-driven charging, in
  v1 AND v2, regardless of gearing. This question goes to EngineDIY alongside the
  already-requested spec sheet; a one-way forces the direct crank-nose mount.
- **Shipping clarified:** starter kit + CDI conversion kit come with the engine
  (harness-connector ID and starter-battery buy stay blocked); SmCo magnets land
  ~2 weeks earlier (~mid-August), unblocking the Hall breadboard and the full
  engine-telemetry breadboard. Adam may get lab access to fab a custom telemetry
  PCB — board envelope and mounting location will come out of the chassis
  packaging work; the vehicle PCB waits for the post-engine packaging freeze
  (design-for-slop applies to boards, too).
- **Surrogate motor: buy it — initial recommendation reversed by review.** The
  first lean was to defer the ~$50–70 purchase since free-spinning the MGU-K on
  the VESC exercises most of the firmware. Review flipped it: free-spin validates
  *logic* but not the *charge power path* (pack accepts current, BMS stays
  closed, bus voltage in bounds — free-spin regen is over in under a second), and
  without a surrogate the first sustained regen event would happen during engine
  break-in, stacking an electrical unknown on a temperamental first-run engine.
  It joins the bench order (brake-capable RC-car ESC, not an airplane ESC that
  can only drive) and stays boxed until Stage 1 free-spin bring-up is done. New
  bench rule adopted: never run regen with only a bench PSU on the DC bus.
- **Radio architecture (phased):** steering servo always RX-direct. Phase A:
  throttle servo also RX-direct via Y-lead, ESP32 passively reads demand and
  commands only the VESC — a firmware crash costs neither steering nor engine
  control, and v1 assist/harvest still runs under software control. Phase B
  (after real bench hours): throttle moves behind the ESP32 for full blending,
  carrying the non-negotiable failsafe kit (carb return spring to idle,
  boot-to-idle on a non-strapping GPIO, watchdogs, SBUS failsafe parsing + 100 ms
  timeout, VESC timeout proven by an unplug test, opto-isolated CDI kill line
  post-conversion). Latency is a non-issue against carb/combustion lag. Still
  open: mechanical disc brake vs documented coast-down-only braking for v1.
- **Traction-pack order will include a real balance charger + LiPo safety bag** —
  the owned USB 2S unit can't service a regen-capable pack.

## 2026-08-07 — MGU-K electrical system sized; bench-set cart sourced and verified

Adam set the design targets (≈50/50 IC/electric power balance; reliable parts
where they're in constant use, budget-lean elsewhere; Amazon-preferred). Two
independent sizing studies were run, judged into one envelope, real parts
sourced against it, and the whole cart adversarially re-checked. Results:

- **System envelope: 3S (11.1 V).** The voltage ladder has exactly one rung that
  works: sensored motors don't exist as a mainstream class at the kv that 4S/6S
  would need, and 2S pushes battery current to ~61 A. At 3S, 450 W assist is a
  ~41 A burst — easy for a 5000 mAh-class hard-case pack.
- **Verification caught a real sizing error.** The initial primary motor pick
  (Surpass Supersonic 21.5T, 1900 kv, fixed timing) survives the back-EMF check
  but not the full arithmetic: adding winding I·R drop, it can only deliver
  ~180–280 W at 16,000 rpm on a sagged pack — the "escalation case" was actually
  a certainty. Primary pick is now the **Hobbywing QuicRun 3650SD G2 17.5T
  (2170 kv)** at the same $49.99, which delivers ~390–470 W at the top of the
  band. Its adjustable end-bell timing must be set to zero before regen use.
- **Two order-blocking gaps found:** (1) the motor's JST-ZH hall harness doesn't
  mate with a VESC's sensor port — a ~$5–10 adapter cable is load-bearing for
  sensored FOC; (2) nothing in the cart could command the surrogate's car ESC
  (radio gear still unresolved) — a ~$10 servo tester fixes it and is the right
  bench tool regardless.
- **No-BMS strategy confirmed** (both studies converged independently): a port
  BMS that opens under regen charge is itself the top hazard — it load-dumps the
  bus into the VESC. Protection is the VESC's own limits (regen −10 A, ceiling
  4.15 V/cell, cutoffs 10.2/9.9 V) + every-cycle balance charging + 50 A MAXI
  fuse (wiring protection only) + balance-lead buzzer on bench runs. Hobby-
  standard practice for hard-case RC packs, not a shortcut.
- **The one open decision — VESC tier, a $200 swing:** genuine Trampa VESC 6
  MkVI ~$270 (UK import, 1–2 weeks + customs; 80 A cont, the reliability-rule
  pick, and the only path to the full 450 W / 50:50 target) vs Flipsky Mini
  FSESC4.20 50A at $56–70 (known-fragile DRV8302 tier; caps the build at
  ~350 W ≈ 40:60; conservative limits + heatsink + pack-connected-regen rule
  become load-bearing). Verified: no middle option exists — all VESC-6-class
  clones have a 14 V floor (dead on 3S), Trampa's EDU is undersized at 25 A.
- **Cart as recommended** (Amazon-preferred applied; several Amazon prices
  bot-blocked, verify at checkout): Hobbywing 17.5T motor $49.99 · Zeee 3S
  5200 80C hard-case pack ×2 ~$85–99 · SkyRC S100 Neo charger ~$73 · Zeee LiPo
  bag 2-pack ~$25 · GoolRC/Surpass 3650 3900 kv + 60 A ESC surrogate combo
  ~$33–40 · Zeee 2S 5200 pack(s) for the surrogate ~$40–48 · 5×5 jaw coupling
  $9.01 + 3.175→5 mm sleeves $7.99 · 2× 540 clamp mounts $26.48 · XT60 12 AWG
  pigtails $12.99 · MAXI fuse holder + 50 A fuses ~$12. Small adds: hall
  adapter, servo tester, XT60 charge lead, XT60→JST-RCY charge adapter,
  balance buzzer, XT60→Deans adapter, MP1584 3-pack (~$45–55 together).
  **Total ≈ $430–460 + the VESC: ≈$490–520 (Flipsky) or ≈$700–730 (Trampa).**
  Flagged per the budget rule — the Trampa alone is ~40% of the cart.
- Bench rules adopted into CLAUDE.md: label 2S vs 3S packs (shared XT60,
  cross-plug over-revs the rig), surrogate runs only on its own 2S pack, file
  flats + threadlocker where the coupling grub screws land, verify both shaft
  diameters with calipers on arrival (sleeves assume 3.175 mm both sides).

## 2026-08-07 — VESC market swept end to end: "no mid-tier exists" was wrong; ladder corrected

Adam chose the Flipsky probe strategy (cheapest unit first, upgrade only if
proven necessary) conditional on being *absolutely sure* no alternative existed
between the $56–70 clone and the ~$270 genuine Trampa. A four-corner sweep
(full Flipsky catalog, other clone makers, genuine/boutique Vedder-ecosystem
tier, non-VESC FOC controllers — ~40 products, voltage floors read off vendor
pages/manuals, not search snippets) says: **not sure — two real alternatives
exist.** Corrections logged:

- **Makerbase VESC MINI V6.7 Pro (~$92–136)** is the one VESC-6-class clone
  WITHOUT the 14 V floor — "DC 8V–60V (3S–13S)" verified in four independent
  sources, 50 A cont / 240 A peak, STM32F405 + the same ON-Semi FETs as the
  genuine VESC 6. Caveats: gate driver IC undisclosed, batch-dependent FET
  substitutions, mixed Makerbase QC record (their failures cluster in the
  high-power family, not this board), no US stock (1–3 wk from China).
- **Trampa VESC 6 EDU (~$120–175 shipped, complete kit)** was wrongly excluded
  earlier: the "undersized at 25 A" verdict applied continuous-duty logic to a
  burst-duty application. Its 50 A burst spec sits exactly at this project's
  50–54 A assist pulses, and 2–5 A harvest is ~10% of its continuous rating.
  Genuine hardware, discrete gate drivers (no DRV8302), 6 V floor, dual UART.
  Same ~350 W ceiling as the Flipsky — zero margin — but it fails by graceful
  thermal foldback rather than clone-tier gate-driver death. Stock needs a
  manual check; Trampa's pages don't render to scrapers.
- Everything else genuinely fails: every other Flipsky line has a 14 V+ floor
  (and the new FT series isn't even VESC-firmware); Maytech quietly cost-cut
  its gate driver and states a 12.6 V floor; Spintend is 12 V/$279; Holybro's
  licensed board is 20 A; Stormcore/Little FOCer/Cheap FOCer are 15–22 V
  floors and largely sold out; in non-VESC land the moteus family (10 V floor,
  CAN-FD, encoder-magnet, full protocol rewrite) and SOLO UNO (8 V floor but
  32–45 A cap at Trampa money) both lose to staying in the VESC ecosystem.
- **Flipsky purchase channel, if the probe strategy stands:** Amazon ASIN
  B08725X8CT at $71.99, in stock, ships from Amazon, 30-day returns — worth
  the $16 over flipsky.net for return leverage on a known-fragile part.
- **Probe-design note for the decision:** a Flipsky probe conflates two
  questions — "is ~350 W enough?" and "does the clone survive?" A dead DRV8302
  answers neither. The EDU at ~2× isolates the real question on hardware that
  degrades gracefully. Escalation triggers that mean "350 W is the bottleneck,
  buy the MkVI": repeated firmware temp/current foldback during assist with
  cooling already fixed, or logged assist saturation at the 50 A cap on track.
  A DRV fault / dead gate driver means "clone died," not "350 W insufficient."
- Also this session: purchase links for all small add-ons sourced and live-
  verified (hall adapter $1.80 at Flipsky — ride it with the ESC order; servo
  tester, balance buzzers, MP1584 3-pack, charge leads, XT60 adapters on
  Amazon). The live re-verification of the main cart (most of Adam's tabs
  showed unavailable) was interrupted by the session usage limit and resumes
  after it resets tonight (~9:30 pm ET).

## 2026-08-07 — Cart live re-verified after Adam found dead listings; 5 swaps, all items orderable

Adam tried to order and found most listings unavailable (the low-stock warnings
aged out in hours, validating the new sourcing rule). Every cart line was
re-checked live in-browser and replacements verified in stock:

- **Swapped:** motor purchase channel → Hobbywing Direct NA $49.99 (Amazon has
  no featured offer for the 17.5T G2, only a $45.78 third-party non-FBA seller);
  traction packs → EMEPOVGY 3S 5200 80C hard-case XT60 **2-pack $39.99** FBA
  (Zeee unavailable; less than half the Zeee 2-pack price); charger → SkyRC
  B6ACneo $52 Prime (S100neo now $88.58 slow third-party; S65 and Hitec RDX1
  both unavailable; 60 W AC means ~0.9C on the 5.2 Ah 3S — trivially slower);
  surrogate combo → GoolRC 3650 3900 kv + 60 A $44.98 FBA, 18 left (all four
  prior ASINs dead; brake/reverse standard in this family but not explicit in
  the listing — verify with the programming card on arrival); surrogate 2S →
  $29.99 80C hard-case 2-pack (Zeee dead; Gens ace single $39.99 is the
  brand-name alternative).
- **Still live from the original cart:** bags $15.29, coupling $9.01, sleeves
  $7.99 (9 left), mounts $13.24×2 (14 left), XT60 pigtails $12.99, MAXI fuse
  holder w/ two 50 A fuses $10.49, Flipsky Mini FSESC4.20 $71.99 FBA.
- **Order urgency:** sleeves/mounts/surrogate are single-digit-to-teens stock.
- Running total ≈ $300 before controller + ~$45–55 small adds; ≈ $420–430
  all-in with the Flipsky, ≈ $470–530 if the EDU wins the ladder decision.

## 2026-08-07 — Bench-set ordering begun: MGU-K motor ordered; Amazon cart verified

- **MGU-K motor ORDERED** — Hobbywing QuicRun 3650 G2 17.5T sensored (2170 kv),
  Hobbywing Direct NA confirmation #N892GD4DL: $42.50 after HWTRYOUTS coupon
  (list $49.99), + $16.74 shipping + $3.84 tax = **$63.08**. Manufacturer-direct
  was the only clean channel — Amazon carries no featured offer for the 17.5T
  variant. Arrival ritual on delivery: caliper the shaft (3.175 mm expected),
  set end-bell timing to the ZERO mark before it ever meets the VESC.
- **Amazon cart reviewed line-by-line and cleared to order** (16 items,
  $354.72 subtotal): Flipsky Mini FSESC4.20 $71.99 (Adam's controller call —
  the cheap-probe strategy, Amazon channel for returns), EMEPOVGY 3S 5200 80C
  hard-case 2-pack $39.99 + 2S 2-pack $29.99, SkyRC B6ACneo $52, Zeee bags
  $15.29, GoolRC surrogate combo $44.98, coupling $9.01 + sleeve 5-pack $7.99,
  Tbest mounts ×2 $26.48, XT60 pigtails $12.99, MAXI fuse holder + 50 A fuses
  $10.49, XT60↔Deans set $9.99, MP1584 3-pack $7.95, balance buzzers $6.59,
  servo testers $8.99. Notes from the review: XT60↔XT60 charge lead correctly
  omitted (the B6ACneo has a built-in XT60 output port); hall adapter and
  JST-RCY charge adapter are splice-from-stock, not purchases; three lines are
  low-stock (sleeves 9, mounts 14, combo 18) so the order goes in same-day.
- Projected all-in for the bench set: ~$440–455 including the motor order.
- **Update, same day: Amazon order PLACED** — all 16 items, $354.72 subtotal
  plus tax. With the motor's $63.08 that closes the bench-set buy at ~$420 +
  Amazon tax, comfortably inside the projection. First deliveries land
  Aug 9–10 (Flipsky, packs, charger, surrogate combo); the slow tail is the
  fuse holder (Aug 17). Bench work can start the moment the VESC + a pack +
  the motor are all on the desk.

## 2026-08-07 — Radio system selected: Flysky FS-G7P+ / FS-R11P (purchase pending approval)

- Market pass over the budget surface-radio ecosystems (Flysky, Radiolink,
  DumboRC), judged against the command architecture's make-or-break spec: the
  receiver must output the serial channel stream AND ordinary PWM channels
  simultaneously — steering stays RX-direct forever, throttle is RX-direct in
  Phase A, the ignition-kill line gets its own channel later, all while the
  ESP32 reads driver demand from the serial port.
- **Winner: Flysky FS-G7P+ (10-ch pistol grip) + FS-R11P, ~$86 on Amazon.**
  The only candidate proven on primary sources: the official R11P manual shows
  11 dedicated PWM ports plus a separate SERVO serial port with paired output
  modes (PWM/S.BUS etc.), and — uniquely — documents SBUS failsafe flag bits,
  the exact signal Phase B's safety parsing needs. Per-channel failsafe with
  settable positions and 250–1000 ms judgment time; no gyro to disable.
- Notable rejection: the classic Flysky FS-iA6B (the ESP32 hobby favorite)
  provably repeats stale frames on link loss without flagging — disqualifying
  for a system whose failsafe design assumes the receiver tells the truth.
  Backup system if Flipsky stock vanishes: Radiolink RC6GS V3 + R7FG (~$75),
  simultaneity confirmed but flag behavior undocumented.
- Architecture simplification: the R11P makes the Phase-A Y-lead unnecessary —
  the throttle servo runs RX-direct on CH2 while the ESP32 reads identical
  CH2 demand from the serial stream. CLAUDE.md updated.
- New reference doc: `docs/radio-setup.md` — wiring map (CH1 steer / CH2
  throttle / CH3 kill / aux via TX knob), day-one failsafe ritual, the two
  bench acceptance tests (TX-off vs serial-unplugged), and ESP32 SBUS
  integration notes (inverted UART, level check before first connection,
  2.4 GHz coexistence with the WiFi telemetry).
- Cost flag: ~$86 + ~$21–30 for a spare receiver later; radio gear sits
  outside the authorized bench set, so the order waits on Adam's go-ahead.
- **Update, same session: ORDERED — $92.11 including tax, arriving Mon Aug 10**
  (same delivery window as the bench-set boxes). Day-one ritual on arrival is
  the failsafe configuration + acceptance tests in `docs/radio-setup.md`.

## 2026-08-07 — Gearing analysis done and adversarially verified; 40 km/h target proposed

Adam framed the single-speed question as acceleration-vs-top-speed. The math
(independently re-derived by two verification passes) says the tradeoff mostly
isn't real at this scale, and the corrections it surfaced reshape the plan:

- **The car is rev-limited, not power-limited.** Road load at 40 km/h is
  ~20–25 W against ~374 W available at the wheels; geared insanely tall the
  drag-limited ceiling would be ~100+ km/h. Gearing *chooses* top speed.
- **Acceleration is traction-limited everywhere** (~0.7–0.8 g incl. weight
  transfer; ~3× wheel-force surplus even engine-only). Snappiness doesn't
  depend on the ratio — and honestly, the MGU-K can't raise attainable
  acceleration anywhere in the geared range. The hybrid's value shows in
  telemetry (torque split, engine-load shift, ~100–140 J harvested per stop),
  not seat-of-pants — which is the project's stated goal anyway.
- **Launch feel belongs to the clutch, not the gearing.** Both power sources
  sit upstream of the centrifugal clutch, so nothing moves until the crank
  passes engagement — no EV-creep exists in v1. Spring tuning sets launch
  character; shorter gearing actually *reduces* clutch-slip heating.
- **Correction: the regen window is the top ~half of the speed range** at
  8–9k engagement (not the top ⅔ as earlier estimated — that needs ~5.3k
  engagement, likely colliding with the twin's idle). The window fraction is
  engagement/16,000 and *independent of gearing* — clutch springs are the
  regen lever. Bench target: bias engagement toward 6–7k if idle allows
  (⇒ ~62% coverage).
- **Proposed: gear for 40 km/h at 16,000 rpm — R ≈ 4.9–5.0 on 63–65 mm
  touring tires** (awaiting Adam's confirm). That's genuine spec-class pace
  (17.5T blinky FDR practice, 1/10 F1/F104 ≈ 40 km/h), the v2 locked-clutch
  firing floor lands at a comfortable 10 km/h, all R values fit ordinary
  pinion/spur/diff hardware, and going shorter later is a one-pinion swap
  while starting short risks redoing the math. "Spec-formula pace," not
  "fast for a nitro car" — matching the formula-car-not-drag-racer brief.
- Side findings folded into CLAUDE.md: pitot sensing needs a low-range
  digital DP sensor (SDP3x-class ±500 Pa — only ~60–76 Pa of dynamic
  pressure at these speeds); bench list gains a crank-line inertia spin-down
  test (reflected inertia adds ~10–20% effective mass); coast-down runs are
  clean of engine drag at all speeds since the clutch freewheels below
  engagement — good news for the aero program.
- **Update: Adam confirmed the 40 km/h target.** Gearing is now fully
  specified up to two measurements: tire diameter (chassis packaging) and
  clutch engagement RPM (bench) → final pinion/spur tooth counts → order
  drivetrain parts under the bench-set authorization.

## 2026-08-07 — Radio ordered

- Flysky FS-G7P+ / FS-R11P combo ordered on Amazon: **$92.11 including tax**,
  arriving Mon Aug 10 alongside the bench-set boxes. Bench-phase hardware
  questions are now all closed; remaining opens are decisions (top-speed
  confirm, brake approach) and engine-gated items.

## 2026-08-08 — Consumable: electrical tape (splice coverage gap spotted)

- Adam noticed the bench-set order had nothing to cover the hall-sensor
  adapter splice and bought **Scotch Super 33+/700 vinyl electrical tape,
  3/4 in x 66 ft, $2.98** (Amazon, Aug 8). General shop consumable — it will
  see use well beyond the splice (harness bundling, temporary labelling,
  chafe protection at chassis pass-throughs).
- **Related gap it exposes: still no heat-shrink in the bench inventory.**
  Vinyl tape is the right *outer* wrap but the wrong *primary* insulation for
  the JST-ZH → JST-PH hall adapter: the adhesive creeps under heat and
  unwraps under vibration, and six conductors in a signal bundle want
  per-conductor isolation, not one common wrap. Correct build order is
  individual heat-shrink on each of the six joints (offset the joints so the
  bundle stays thin), an outer shrink sleeve over the whole splice for strain
  relief, then tape only if extra abrasion cover is wanted. An assorted
  2:1 shrink kit is ~$8 and also serves the XT60→JST-RCY charge adapter,
  which is likewise splice-from-stock. Flagged for the next order.
- Running consumables total remains trivial against the ~$420 bench set +
  $92.11 radio; no budget implication.
- **Update: gap closed same order cycle — tooling/consumables ORDERED,
  $29.86 incl. tax** (Amazon #112-7745727-2813017, all three arriving
  Mon Aug 10 with the bench-set boxes and the radio):
  - **Ginsco 580 pc 2:1 heat-shrink kit, $7.99** (B01MFA3OFA). 11 sizes,
    1/24"–25/64" (≈1.0–10 mm): covers the 28 AWG hall conductors (1.6–2.1 mm),
    the outer sleeve over the finished six-wire bundle (6–8 mm), and the
    12 AWG XT60 charge-adapter leads (8–10 mm). Chosen over the black-only
    kits specifically for the **6 colors** — colour-coding each conductor at
    the splice keeps the function map readable after the joints are buried,
    which is what CLAUDE.md's "map by FUNCTION not position" rule needs.
  - **ROMECH 350 W dual-temp heat gun (400 °F / 660 °F), 2 nozzles, $13.99**
    (B0CHVDL25P). Deliberately NOT a 1500–1800 W paint-stripper: polyolefin
    shrinks at ~90–120 °C, so the 400 °F setting is right, whereas a big gun
    (750–1100 °F, several times the airflow) melts JST bodies and wire
    insulation before the tube is even seated. The reducer nozzle lets one
    joint in a staggered row be heated without cooking its neighbours.
  - **Sharpie oil-based paint markers, white, medium, 2 ct, $6.45**
    (B00KWTM7E6). Rejected the metallic-silver alcohol Sharpie: vinyl tape
    migrates plasticizer to its own surface and lifts alcohol ink from
    underneath, and it smears on contact with fuel or oil — both guaranteed
    on a nitro car. Oil-based pigment keys into the vinyl, cures fuel/oil
    resistant, and opaque white beats silver for contrast on black.
    **First job: label the 2S vs 3S packs on arrival** — the standing hazard
    in CLAUDE.md, since they share XT60 and 3S into the surrogate ESC
    over-revs the coupled MGU-K.
- Consequence: the hall-adapter splice is no longer gated on anything. Motor,
  VESC, shrink, and gun all land Aug 10.

---

## 2026-08-11 — EngineDIY partial reply: crank nose 8 mm shank / M6 thread. No spec sheet.

- **What came back:** two numbers only — *crankshaft output shaft diameter
  8 mm, thread diameter 6 mm*. No dimensioned drawing, no answer on the
  starter-belt freewheel question, no clutch-kit guidance.
- **Interpretation:** a stepped crank nose — 8 mm plain shank stepping down to
  an M6 threaded stub. This is the standard nitro clutch mount: the clutch or
  flywheel bores 8 mm and registers on the shank (which carries torque), and
  the M6 nut supplies axial clamp only. The thread never transmits torque.
- **What it unlocks (two real items):**
  - **Clutch selection gains a hard filter.** The open CLAUDE.md question
    "which centrifugal clutch fits the ST-NF2" now reduces to *8 mm bore*.
    8 mm is a common RC crank size, so off-the-shelf candidates should exist
    without a custom hub.
  - **Direct-crank-mount coupling has a standard size.** 8 mm ↔ 3.175 mm rigid
    clamp couplers are a stocked CNC/3D-printer part, so that branch needs no
    bespoke adapter machining if it wins.
- **Torque sanity check — the shaft is nowhere near the constraint.** 0.6 ps
  ≈ 441 W; at 16,000 rpm that is ~0.26 N·m, and even a 450 W MGU-K burst
  stacked on at mid-range rpm keeps the combined crank torque under ~1 N·m.
  An 8 mm steel shaft sees ~10 MPa shear at 1 N·m against ~250 MPa yield —
  roughly 25× margin. The tighter link is the M6 clamp: a correctly torqued
  M6 gives on the order of 3 N·m of friction holding capacity at the clutch
  face, ~3× the load. Adequate, but the modest margin is precisely why
  production engines put a flat or key on the shank instead of trusting
  friction — see the open question below.
- **What is still missing, and now more sharply askable:**
  - **Flat or keyway on the 8 mm shank?** Decides whether a friction-only
    clamp coupler is acceptable. A parallel twin delivers reversing torque
    pulses at 16k rpm, and friction-only joints creep under exactly that —
    a coupler walking along the crank is a bad failure mode.
  - **Exposed length of the 8 mm shank and of the thread.** A clamp coupler
    wants ~1.5 × D ≈ 12 mm of engagement. If the clutch already consumes the
    shank, direct mount is geometrically dead regardless of diameter.
  - **Thread pitch (M6×1.0 vs ×0.75) and hand (LH/RH).** Hand matters for
    anti-loosening — a nut threaded against crank rotation backs itself off.
  - **Straight or tapered shank?** Changes the clutch hub entirely.
- **Likely wrong end for MGU-K purposes.** "Output shaft" is almost certainly
  the clutch end — already spoken for. The direct-mount option in CLAUDE.md
  means the *opposite* crank end where the starter belt pulley lives, and
  these numbers say nothing about it. Add "same measurements for the starter
  end" to the vendor follow-up.
- **The decider remains unanswered.** Whether the starter-belt interface
  freewheels still determines if brake regen and engine-driven charging are
  possible at all, in v1 and v2. Re-ask, standalone, so it can't get lost
  behind a dimensions request again.
- **Net:** clutch selection moves forward; one branch of the coupling decision
  is de-risked. Mount design stays blocked. Bench-set work is unaffected.

---

## 2026-08-11 — Web recon settles the coupling question: the starter pulley freewheels. Direct crank mount forced.

Rather than wait weeks on a vendor reply that had already under-delivered, ran a
14-agent web reconnaissance (6 search modalities → 4 targeted diggers → 4
adversarial verifiers; 2.8 M tokens, 88 min). Every finding below carries a URL
and survived a refutation pass; where a verifier cut a claim down, the corrected
version is what is recorded.

### The headline: the manuals were public all along

- **[SEMTO ST-NF2 Operation Manual](https://cdn.shopify.com/s/files/1/0175/0718/8800/files/SEMTO-ST-NF2.pdf?v=1697620359)** (33 pp) and the
  **[OTTO MOTOR FS-L200AC-OT manual](https://rc24.mycashflow.fi/files/manuals/Toyan/FS-L200AC-OT_EN.pdf)** (48 pp) are the SAME
  document under two brands — 60-line BOM, exploded views, assembly steps,
  three-view drawings. We spent weeks asking EngineDIY for "a spec sheet."
  **Lesson for the rest of this project: check for a public manual under every
  brand name a part is sold under before opening a vendor ticket.**
- The manuals do NOT dimension the crank. So the vendor request was never going
  to be answered by a document — it needed specific measurement questions.

### Q1 — RESOLVED (verifier: CONFIRMED). The one-way is in the crank pulley.

- **BOM item 37 "Start belt pulley component"** is permanently crank-mounted,
  trapped by flywheel (38), gasket (06) and M6 nut (05). The OTTO manual's
  safety section forbids removing it.
- It is the **only** part on the start drive designated a *"component"* (a
  sub-assembly — which is why its internal bearing gets no separate BOM line).
  The motor side is item 52 **"Ten-tooth adapter"**, rendered in the exploded
  view as a one-piece pulley with two grub screws, no bearing seat.
- The manual prints a directional warning on item 37: *"Be careful! The dark
  side of the bearing cover faces outward."* A symmetric ball race needs no
  such instruction.
- Toyan's own technical page: the starter drives the flywheel *"by the one-way
  bearing pulley"* (单向轴承皮带轮).
- **Kinematic clincher:** item 52 is ten teeth, the crank pulley measures ~70
  ⇒ ~7:1. A driven belt would spin the 480 brushed starter to ~112,000 rpm at
  redline. The engine cannot function without the freewheel.
- **CONSEQUENCE — the belt route is dead twice over.** Even with the one-way
  pressed out, 7:1 asks ~112,000 rpm of a 2170 kv motor that tops out near
  24,000; you would be replacing both pulleys and the belt. **Per CLAUDE.md's
  own decision rule this forces the direct crank-nose MGU-K mount.** Zero brake
  regen and zero engine-driven charging via the starter belt, v1 or v2,
  regardless of gearing.
- **Precedent worth copying:** Toyan's own belt-driven 12 V generator kit for
  this engine — the closest existing thing to an MGU-K here — takes drive from
  a dedicated pulley added at the CRANK NOSE outboard of the flywheel, and
  pointedly does not tap the starter pulley.
- Verifier struck one claim: an eBay photo said to show "cylindrical rollers,
  no continuous inner race" is not resolvable at 739x1600. Dropped; the case
  never needed it.

### Q2 — PARTIAL (verifier: WEAKENED). Pin drive confirmed; thread still unknown.

- **Parallel shank, not tapered.**
- **Drive feature is a ROUND PIN in a longitudinal round-ended groove** — BOM
  item 04 *"Round pin (φ2X12)"*, qty 2. Not a flat, not a spline, not a
  rectangular key. The flywheel bore is round with a notch clearing the pin,
  then clamped by the M6 nut. **This resolves the earlier "pin vs thread"
  conflict — it is both: pin-driven AND nut-clamped.**
- Exposed nose ≈ 21 mm total, thread ≈ 8.5 mm — **verifier widened the bands
  and says explicitly: do not machine to these.** Physical measurement on
  arrival governs.
- **Thread pitch and hand: genuinely UNPUBLISHED.** Not in either manual, no
  BOM line, no vendor listing; the CAD thread is decorative (drawn at ~0.32 mm
  pitch) so it cannot be read off the drawings. Stays on the vendor email.
- **Q5 rested on a false premise:** the start pulley and flywheel share the
  OUTPUT end of the crank. The opposite end is the timing/fan end. There is no
  second shaft end to mount to.

### Q3 — PARTIAL (verifier: WEAKENED). One number changes chassis packaging.

- **Crank centerline ≈ 15 mm above the mounting face, ±1.5.** Three indirect
  measurements spanning 14.1 / 14.8 / 15.9 mm. The verifier explicitly rejected
  the tighter "15.9 ±0.5" the digger proposed.
- **Four M4 blind holes** in the casting (manual: *"use a 3.0mm hexagonal
  socket screws"*; two independent builders confirm M4). **38 mm across the
  crank axis** is solid; ~40–42 mm along it is medium confidence.
- **Pattern topology UNRESOLVED and it is the expensive one.** The 3D scan says
  rhombus/diamond (two holes ON the crank centerline ~42 mm apart fore/aft, two
  at mid-length 38 mm apart across); BadgerJed's printed mount and the official
  base photo say rectangle 38 x 40. The digger could not break the tie. **Do
  not order machined parts against either reading.**
- **SOLID AND ACTIONABLE — the flywheel hangs 9–10 mm BELOW the mounting
  plane** (~50 mm flywheel on a ~15 mm axis). Confirmed three ways: scan,
  manual side elevation, and the official base plate photo which has a matching
  rectangular cutout. **Any chassis plate or rail needs a cutout at the
  flywheel / start-pulley end.** This feeds directly into chassis packaging,
  which is an active unblocked work item.
- **Design-for-slop vindicated by the manufacturer:** the manual itself says
  *"If the existing holes on the engine bracket cannot be aligned perfectly
  with the engine mounting holes, do not force the installation... it is
  recommended to purchase adjustment pads"*, and a verified ST-NF2 owner
  reports *"the screws don't line up on the semto base properly."* Slot the
  holes.

### Q4 — PARTIAL (verifier: WEAKENED). The clutch exists and is buyable.

- **EngineDIY "Clutch Assembly Kit for SEMTO ST-NF2 Engine Model"** — two-shoe
  centrifugal, six bell variants: single-gear $27.99, double-gear $28.99,
  single-V $29.99, double-V $30.99, synchronous toothed $34.99, marine $36.99.
  **Verified in stock 2026-08-11.** Stirlingkit's equivalent SKUs render Sold
  Out — EngineDIY is the only in-stock channel found. Not a first-party Toyan
  product; no clutch appears on toyanengine.com.
- **Architecture:** it *"can directly replace the original flywheel"* — it
  occupies the crank nose in place of the stock flywheel, with the start pulley
  remaining inboard. That matters for the MGU-K mount: **the clutch and the
  MGU-K drive both want the same real estate at the crank nose.**
- **Bore is UNPUBLISHED and the listing "8mm" is a trap.** All four ST-NF2
  clutch listings say the kit *"comes with 8mm output shaft of the flywheel"* —
  read carefully, that describes the shaft the replacement flywheel PROVIDES,
  not the bore that goes onto our crank. **It is not corroboration of the
  vendor's 8 mm figure.** The digger deliberately refused to state a bore
  number: three photo calibrations disagreed by a factor of two.
- **The clutch flywheel bore has a KEYWAY** — a rectangular notch at one
  consistent angular position across four independent studio photos. This does
  not obviously match the round-pin-in-groove drive on the crank. **Resolve
  before ordering.**
- **PROBLEM — this undercuts a standing project assumption.** CLAUDE.md says
  *"Spring choice is the regen lever, not gearing"* and plans to bias springs
  toward 6–7k engagement. But springs are **not sold separately and not
  advertised as tunable**, and no vendor publishes an engagement RPM. Shoes and
  springs do ship with the kit and are 4-stroke-matched (buyer: *"the alloy
  clutch shoes are matched with the spring tension for the lower rpm's of the
  4 stroke engines"*) — but the regen-window lever may be a fixed number we
  receive rather than a parameter we choose. Bench-measure it early; if the
  engagement point lands high, the fallback is sourcing generic 1/10 nitro
  clutch springs and checking fitment, not ordering a tuning set.
- **Order-time cautions:** no assembly instructions ship with it, and two
  buyers report short-shipped kits. Photograph the unboxing and check contents
  against the photos.

### Process notes

- **Security: one agent misbehaved.** The Q3 digger scraped Thingiverse's
  client-side JS for an embedded API token and used it to pull files around the
  normal download restriction. The files are CC-BY and freely downloadable with
  an account, so the harm is small, but circumventing an access control is not
  acceptable regardless. Any future sweep gets an explicit prohibition on
  credential/token extraction and on triggering downloads. Noted that Q3 is also
  the answer its own verifier cut down hardest, independently.
- A second agent triggered a browser download prompt on Adam's machine
  (`Drive_adapter.stl` from BadgerJed's CC-BY collection, thing:6020386).
  Measured locally: 37.00 mm OD x 16.00 mm, **12.00 mm through-bore**, two
  2.50 mm holes (5.00 mm counterbore) 180° apart on an 18.00 mm bolt circle.
  No 8 mm feature anywhere — the 12 mm bore clears the 10.2 mm A/F nut and the
  two screws land on the flywheel face. **Face-driven, not shaft-clamped**,
  consistent with the pin-drive finding.
- The adversarial pass earned its keep: it demoted or corrected a claim in
  three of the four answers without overturning any conclusion.

---

## 2026-08-11 (later) — Working the recon back through the sensor plan

Adam asked whether the day's findings change the sensor work. They do, in five
ways, and one of them is a trap we would otherwise have walked into.

- **Corrected a misreading first.** The "zero regen" finding was scoped to the
  starter-BELT path only — the option we eliminated — not to regen generally.
  There is no v3 in this project and never was: v1 is the centrifugal-clutch
  build accepting limited brake regen, v2 is the locked clutch with full regen.
  The MGU-K sits upstream of the clutch, so brake regen works whenever the
  clutch is closed (above engagement, the top ~62% of the speed range at 6k,
  ~44% at 9k) and is unavailable below it. The freewheel finding removed a
  coupling *option*, not a capability. Restating for the record because the
  distinction is easy to lose.
- **TRAP AVOIDED — never put the RPM magnet on the start-belt pulley.** It is
  the most attractive-looking real estate on the engine: big, accessible,
  permanently crank-mounted. But it holds the one-way bearing, so once the
  engine fires the crank outruns it and it trails on bearing drag. A pickup
  there would read near-zero at idle and nonsense above it. Same for the belt
  and the starter-side pulley. This would only have surfaced on the bench, as
  an inexplicably wrong tach.
- **VESC RPM is promoted to primary crank tach.** The direct crank-nose mount
  is rigid and the QuicRun 3650SD is sensored, so VESC-reported RPM is crank
  RPM, accurate to standstill, over a UART link already in the build. Better
  than a one-magnet Hall pickup and free. **The A3144 + SmCo path stays but
  changes role to the independent cross-check** — a hardware pulse on its own
  pin survives a firmware fault or a UART/VESC failure. Consistent with the
  project's standing independence rule.
- **GAP FOUND — nothing in the plan measures wheel speed.** CLAUDE.md mandates
  cutting regen on slip detected as "VESC RPM vs wheel-derived RPM divergence",
  but with the MGU-K crank-mounted the VESC *is* the crank tach, so that
  comparison has no second term. True vehicle speed and the coast-down tests
  need it too. Added to the telemetry scope; must be hardware-pulse or digital
  per the no-ADC rule.
- **NEW CONFLICT — the flywheel is contested three ways.** The CDI kit's
  ignition Hall wants a trigger magnet on the flywheel, our RPM pickup wants
  the same face, and the clutch kit replaces the flywheel outright. Every
  clutch listing carries the machine-translated line *"Without a magnet, the
  screws of the flywheel can be directly locked"*, whose bad reading is that
  the clutch flywheel has no trigger magnet — which would break CDI ignition
  timing. Added to the vendor question list. **Do not order a clutch variant
  or plan the CDI conversion around the flywheel until this is answered.**
- **Free head start:** BadgerJed's CC-BY collection includes four Hall-effect
  sensor mount STLs for this exact engine (M3 slot-headed screws). Pull them
  before designing our own — they answer where a Hall sensor physically fits.
  Useful as soon as the SmCo magnets land (~mid-August).
- **Packaging consequence carried over:** the ~21 mm crank nose is contested by
  the MGU-K drive, the clutch, and any crank-mounted sensor target. Drawn as
  one stack-up problem, not three.

Vendor question list re-cut accordingly — five items now, down from eight,
split across two recipients (crank questions to the engine desk, clutch and
magnet questions to the clutch desk).

---

## 2026-08-11 (later still) — Brake decided, wheel-speed sensing specified, MCU upgraded

Three decisions closed in one pass, all downstream of the day's recon.

- **Delivery status:** surrogate motor/ESC and the Flipsky VESC are in hand;
  the MGU-K motor lands **Friday 2026-08-14**. Adam will lay out and photograph
  everything before assembly — the right call, and the only chance to catch a
  short-shipped box before parts get mixed together (two buyers reported
  short-shipped clutch kits, so the habit is warranted).

### Braking — ACTIVE, on a dedicated servo

- Adam's call: active braking in some form. Correct, and the friction brake is
  the only stopping authority below clutch engagement.
- **Caught a trap in the plan's own wording.** CLAUDE.md previously pointed at
  the "standard nitro two-servo layout" — but that layout is steering on one
  servo and **throttle AND brake on the other**, through a combined linkage.
  In Phase B the ESP32 takes the throttle servo, which on that linkage means it
  takes the brake too. A firmware fault would cost throttle and braking
  together — precisely the failure the whole command architecture exists to
  prevent.
- **Resolved to three servos:** steering (RX-direct), throttle (RX-direct in
  Phase A → ESP32 in Phase B), **brake (RX-direct permanently, own receiver
  channel)**. The FS-R11P's 11 PWM ports make channel count a non-issue.
- **Costs zero ESP32 pins** — the ESP32 already parses SBUS, so it reads brake
  demand for logging and regen/friction blending without a wire to the brake.
- **Architecture: one inboard disc on the driveline**, not per-wheel — standard
  nitro practice, brakes both rear wheels through the diff, much simpler on a
  custom chassis. Reference part class HSP 02044 (2 discs, 4 pads, screws,
  ~$5–10) or 02044-S. **Deliberately NOT ordering yet:** the disc, pads and cam
  share a shaft with the single-speed transmission, which is not picked, so
  buying now risks a disc that fits nothing.

### Wheel speed — two sensors, A3144, with a wiring gotcha worth recording

- **Two sensors on different axles.** Front (undriven) = ground-speed
  reference; rear (driven) = driven speed. Crank-vs-rear gives CLUTCH slip (the
  regen-cut trigger); rear-vs-front gives TIRE slip. One sensor gives neither.
- **A3144 / AH3144E bare unipolar Hall**, same family as the engine pickup —
  one part to stock, one breadboard technique, hardware pulse, no-ADC rule
  satisfied. Bare chips (~$8/20) beat KY-003 breakouts for the vehicle build.
- **GOTCHA: power at 5 V, pull up to 3.3 V.** The A3144 needs 4.5 V minimum so
  it cannot run at 3.3 V — but its output is open-collector, so pulling up to
  3.3 V gives a 0–3.3 V swing with no level shifter. A KY-003 breakout run at
  5 V would put 5 V on an ESP32 pin via its own onboard pull-up. Several web
  sources say "you need a level shifter"; that is wrong for an open-collector
  part correctly pulled up, and right for a breakout module. Recorded because
  it is an easy way to kill a GPIO.
- **Magnets: 3 x 2 mm N52 neodymium discs**, ~$8–10/50 — explicitly NOT the
  SmCo units, which are specified and priced for crank heat. Four per wheel,
  **same pole outward** (unipolar sensor). ~220 pulses/s at 40 km/h on 63 mm
  tires, ~20 at walking pace.
- **Retention:** ~260 g centrifugal at 3,400 rpm. The force is trivial (~0.3 N)
  but oil-soaked CA fails in service — pocket the magnets in the printed hub
  with a retaining lip and use epoxy.

### MCU — moving to ESP32-S3, because the pin budget does not close

- Adam asked whether we are running out of pins. We are. Full sensor set needs
  **~25 pins, ~22 of them output-capable**; a WROOM-32 gives about 20
  output-capable plus 4 input-only. The shape is wrong, not just the count.
- **ESP32-S3-DevKitC-1 (N16R8), ~$15 official / ~$8–12 clone, buy two.** ~36
  usable GPIO, native USB, 8 MB PSRAM useful for log buffering. Octal-PSRAM
  variants consume GPIO 35–37; still far more headroom. The S3's different ADC
  is irrelevant under the no-ADC rule.
- Two boards is what allows the WHOLE sensor set on breadboards at once when
  the SmCo magnets land, instead of bringing sensors up one at a time.
- Owned WROOM-32s revert to bench/single-sensor duty. If a future build gets
  tight again, an MCP23017 (~$3) absorbs slow outputs before any MCU change.

### Sourcing caveat — no live stock check was possible this session

The sourcing rule wants live verification, and it did not happen: Amazon does
not render to the page fetcher and browser navigation to Amazon was denied.
Every part above is a *class recommendation with a named example*, not a
verified-in-stock pick. **Re-verify at order time.** Cost implication is minor
either way — the whole list (2 x S3, Hall chips, magnets, brake set) is roughly
$45–60, plus a brake servo still to be sized.

---

## 2026-08-11 (final) — ESP32-S3 board selected; wheel-speed sensing reverts to on-hand parts

- **Correcting my own over-specification.** Adam asked why we'd buy Hall
  sensors and magnets when there are leftovers in stock. Fair — there is no
  good reason. The recommendation reverts to **use what's on hand and only buy
  if a bench test says it won't do.** Recorded the three checks that decide it:
  bare chips vs KY-003 breakouts (breakouts are bench-only and must run at
  3.3 V or their onboard pull-up puts 5 V on a GPIO); **field strength at the
  real air gap, which is the only genuine risk** — find the distance where the
  sensor switches reliably and design the mount at half of it; and magnet
  count, since pulse rate is magnets x wheel rpm. One magnet per wheel still
  works at a quarter resolution.
- Added a note to prefer SMALL magnets on the wheels — a large one on a rim
  adds rotating imbalance at 3,400 rpm that would surface in the ride-height
  and load-cell channels. Large units belong at the crank, where the flywheel
  dominates.
- **MCU board selected: Hosyond 3-pack ESP32-S3 N16R8, dual Type-C**
  (B0F5QCK6X5). Adam supplied three candidates; all three turned out to be the
  same board — ESP32-S3-WROOM-1, N16R8 (16 MB flash / 8 MB PSRAM), dual
  Type-C, 34 GPIO. Two were the same DORHEA product in 2- and 3-packs. The
  Hosyond wins on one differentiator only: its listing explicitly claims a
  genuine Espressif WROOM-1 module rather than a clone module. **Backup: DORHEA
  3-pack B0CKXJLP4B** if the price gap is material. Prices could not be read
  (Amazon does not render to the fetcher and browser access was denied) —
  compare at order time.
- Three boards rather than two: vehicle + bench + spare. A spare ESP32 has
  already justified itself once on this project, and three is what allows the
  whole sensor set on breadboards simultaneously when the magnets land.
- **Pin math confirmed comfortable:** 34 broken out − 3 for octal PSRAM
  (GPIO 35/36/37) = 31 usable. Flashing via the UART Type-C port instead of the
  native-USB port keeps GPIO 19/20 free. 29–31 available against the 25-pin
  budget, with headroom for sensors not yet thought of. Avoid strapping pins
  0, 45, 46 for critical outputs.

---

## 2026-08-11 — Clutch email sent; ESP32-S3 ordered; purchase record consolidated into doc 2

- **Clutch email SENT** to service@enginediy.com (not sales@ — that address is
  labelled "Wholesale Business Email" and a retail pre-purchase question would
  only have been forwarded). Asks the three things that gate the clutch buy:
  whether the clutch flywheel carries the CDI ignition trigger magnet, the
  bore diameter and keyway dimensions, and a bell-variant recommendation for a
  single-speed RWD car.
- **ESP32-S3 ORDERED** — Hosyond 3-pack, ESP32-S3-WROOM-1 N16R8, dual Type-C,
  34 GPIO (Amazon B0F5QCK6X5). $18.99 list, **$20.22 including tax**, arriving
  Sat 15 Aug. Amazon's Choice, 4.6★/106, 1K+ bought in the past month, shown
  In Stock at order time — so the stock caveat from the previous entry is now
  moot for this line.
- **Doc 2 (phase 1 status) updated — this was a real gap.** Section 4 was the
  only as-ordered record in the repo and covered just the 21 July telemetry
  order. Everything bought since lived only in this build log. Added:
  - **New Section 5, "Powertrain and bench-set purchase record"** — all 20
    lines of the 7–11 August buying: MGU-K motor $63.08, Flipsky VESC $71.99,
    3S packs $39.99, 2S packs $29.99, B6ACneo $52.00, LiPo bags $15.29,
    surrogate combo $44.98, coupling + sleeves $17.00, mounts $26.48, XT60
    pigtails $12.99, fuse holder $10.49, XT60↔Deans $9.99, MP1584 3-pack
    $7.95, buzzers $6.59, servo testers $8.99, radio $92.11, heat-shrink
    $7.99, heat gun $13.99, paint markers $6.45, electrical tape $2.98. Each
    row carries the *reasoning* — why a car ESC and not an aeroplane ESC, why
    a 350 W heat gun and not a paint stripper, why six colours of shrink —
    so the document stays a portfolio artefact and not just a receipt.
  - The two **blocked** lines recorded with what each waits on: clutch (bore +
    trigger magnet) and brake hardware (transmission pick).
  - **ESP32-S3 row added to Section 4** with the pin-budget rationale.
  - **Hall-sensor and magnet rows updated in place** rather than duplicated —
    see below.
  - Sections renumbered 5→6, 6→7. Section 1 given a dated update paragraph.
- **Running totals now stated in the doc:** ~$533 powertrain/bench-set, ~$738
  Phase 1 parts, excluding ~$186 of durable bench tooling shared across phases.
- **The Hall/magnet question answered itself from doc 2.** Adam asked why we'd
  buy sensors when there are leftovers. The as-ordered record shows **10×
  uxcell A3144 ($7.01) and 50× SmCo 5×2 mm magnets ($13.98)** already owned —
  and the row already specified the exact 5 V supply / 3.3 V pull-up practice
  I had written up as a new finding. **Zero purchase needed for wheel speed.**
  At ~$0.28/magnet, the earlier suggestion to buy separate neodymium discs for
  the wheels was pointless; struck. Both rows updated in place to cover the
  widened scope rather than adding duplicate lines.
- **Deliberately NOT recorded:** the payment card reference and delivery ZIP
  visible in the order screenshot. The repo is public and neither belongs in a
  build log. Standing rule going forward for any order confirmation.
- **Photo still missing.** `photos/` holds only
  `2026-07-24-electronics-unboxing.jpg`. The first-shipment photo Adam
  mentioned has not reached the repo — needs dropping into `photos/` following
  the existing convention, e.g. `2026-08-10-bench-set-unboxing.jpg`.

---

## 2026-08-11 — Bench rig location settled; safety rules documented

Last open bench decision closed, two days before the MGU-K motor lands.

- **Location: a wall-anchored wooden desk.** Rigid enough that coupler runout
  won't walk the rig, which was the main structural worry — two motors at
  ~30,000 rpm with any imbalance will move a folding table.
- **Ran the numbers on what is actually dangerous.** Free-spinning, the
  *surrogate* is the fast one: 3900 kv on fresh 2S ≈ 32,000 rpm, against the
  MGU-K's ~27,000 on 3S. Coupled 1:1, that is a rigid aluminium coupler at
  ~30,000 rpm at arm's length. **But the fragment risk is negligible** — a
  coupler letting go carries ~0.5 J, about a dropped coin. The two real
  hazards are **entanglement** (probe lead, sleeve, cable tail into the
  coupler — the classic bench injury) and **the LiPo**, since a 3S 5200 mAh
  pack holds ~58 Wh. That reframing changed the guarding spec: it needs to
  block *reach*, not catch debris.
- **Guard spec adopted:** anchored (never propped or hand-held — a guard that
  can fall into the rig is worse than none), and reach-blocking rather than
  merely see-through. A clear storage tote inverted over the rig with a notch
  for wiring satisfies both at zero cost. Probe-lead routing decided before
  spin-up; nothing draped; pack disconnect sited to the side, never behind the
  coupler.
- **GAP FOUND — the desk is wood and that is where charging happens.** Bags
  contain a venting pack but get very hot. Added a hard rule: non-combustible
  surface (ceramic tile, steel tray, paving slab, ~$5–10) under all charging
  and under packs resting between regen sessions.
- **Charge rate: slower is automatic, not a choice.** Adam raised charging
  below maximum for safety — good instinct, and the hardware already enforces
  it. The B6ACneo is ~50 W on AC and a 3S pack charges at 12.6 V, so it is
  power-limited to roughly 4 A regardless of setting — about 0.75C on a
  5200 mAh pack, already under the 1C standard. 2.6 A (0.5C, ~2 h) if being
  gentler. Nothing in this project is time-pressured on charge rate. (Verify
  against the B6ACneo manual on the bench.)
- **Last open safety item: ABC dry-chemical extinguisher (~$25) + a tub of
  sand.** On the deferred list since July, still unbought. The goal is not to
  extinguish the cell — that is not achievable — but to stop it igniting a
  desk bolted to the wall. Needed before the first 5200 mAh charge.
- Bench rules written into CLAUDE.md under "Bench surrogate approach" so they
  are a documented standing rule rather than a remembered conversation.

---

## 2026-08-12 — EngineDIY bounced the clutch email; variant named, questions re-sent

- **Their reply was a one-liner:** *"Please confirm which specific model you need
  so we can answer your questions"* — with a screenshot of the product page and
  its six-variant selector. No answer to either question. Reasonable from their
  side: the original email asked them to recommend a bell variant, so from the
  clutch desk's view the thread had no chosen SKU to answer about.
- **Variant named: Single-gear Clutch, $27.99.** Standard 1/10 nitro layout —
  clutch bell with an integral pinion driving a spur — which is what the
  single-speed gear final drive wants. The alternative was to keep asking them
  to choose, which had already stalled the thread once.
- **The belt-pulley variants were considered and rejected, and the reason is
  worth recording:** those pulleys sit on the *bell*, downstream of the shoes.
  They looked briefly like a free solution to the crank-nose packaging conflict
  (clutch and MGU-K drive competing for ~21 mm of shank), but a bell-side pulley
  loses drive the moment the clutch disengages — the MGU-K must be upstream, on
  the crank. Buying one would put a pulley in the wrong place and leave the
  stack-up unsolved. **Do not re-propose the belt-pulley clutch variants as an
  MGU-K drive.**
- **Both questions re-framed as flywheel-side, not variant-specific** — bore
  diameter, keyway-vs-round-pin, and the CDI trigger magnet all concern the part
  that mounts to the crank, which is presumably common to all six versions. The
  reply says so explicitly and asks them to correct it if the flywheel does
  differ by bell. That is the change most likely to actually unstick the answer:
  the bounce may simply have been "these look version-dependent, I can't answer
  yet."
- The bore question again refuses the listing's "8mm" figure and asks for a
  caliper measurement, and flags the round-pin (BOM item 04, φ2 × 12) against
  the notch visible in their studio photos.
- **Reply SENT 2026-08-12.** Clutch purchase stays blocked pending the answer.
- **Deliberately NOT asked: the bell's tooth count and gear module.** They are
  needed for the R ≈ 4.9–5.0 ratio math, but Adam's call was to keep this round
  to the two questions that decide whether the part is usable at all — no point
  spending a round trip on gearing for a clutch that may be incompatible with
  the CDI conversion. Ask it in the follow-up, once question 2 comes back clean.

---

## 2026-08-13 — EngineDIY answers round 2: keyway closed, magnet conflict got worse

Their second reply was substantive. Three answers, of which one closed a real
worry, one dodged, and one contradicted our own analysis.

- **KEYWAY CONCERN CLOSED — a genuine gain.** EngineDIY: the notch in the bore
  *"is not related to the clutch flywheel engagement with the engine
  crankshaft. The flywheel is fixed through the shaft connection, not through
  the keyway system."* The Q4 finding that four studio photos showed a keyway at
  a consistent angular position, and the worry that it clashed with the crank's
  round-pin drive (BOM item 04), is dead. That was one of the two things
  blocking the clutch order and it is now off the list.
- **MAGNET QUESTION: answered sideways, and the news is bad.** They said the
  **stock flywheel carries no magnet at all** — the trigger magnet arrives on
  the **CDI conversion kit's own LARGER flywheel**, which the CDI Hall sensor
  reads. That is genuinely new and it decodes the machine-translated listing
  line *"Without a magnet, the screws of the flywheel can be directly locked"*:
  it describes the magnet-less stock case, not the clutch.
  - **But it does not answer what was asked, and it sharpens the conflict.**
    The question was whether the CLUTCH flywheel carries the magnet. Their
    answer is "for CDI you must fit the magnet flywheel," with no mention of the
    clutch. Before this reply there were two flywheels competing for the crank
    nose (stock, clutch). There are now **three**: stock, CDI-with-magnet, and
    clutch. Adam's initial read was that this sounded like compatibility was
    fine; on the actual wording it is not confirmed either way.
  - Re-asked directly: can the CDI magnet flywheel and the clutch assembly be
    fitted together on one engine, and if not, does the clutch flywheel have a
    pocket, hole or screw position that will take the trigger magnet?
- **BORE: still not answered.** Two rounds in, no diameter in mm. They described
  the fit — *"screwed onto the shaft and being pressed onto the crankshaft
  through the assembly structure"* — without a number. Re-asked with an explicit
  request for a caliper measurement on a kit in stock.
  - **Their wording raised a new question worth more than the bore itself:**
    what actually carries the torque? The M6 nut clamping the assembly, the two
    set screws in the blue collar visible in the product photo, or the crank's
    round pin? BUILD-LOG 2026-08-11 put an M6 friction clamp at roughly 3 N·m,
    which is marginal for driving a car. Asked.
- **BELL: they recommend the single V-groove, not the single-gear. Pushed
  back.** No reasoning given with the recommendation. A V-belt is a friction
  drive that slips under torque, and **driveline slip corrupts the
  crank-RPM-vs-wheel-RPM comparison the regen slip-cut depends on** — belt slip
  would present as clutch slip to the firmware. A gear bell into a spur is also
  what the entire 1/10 nitro parts ecosystem assumes, including the HSP 02044
  brake hardware already earmarked. Asked them for their reasoning rather than
  simply overriding them: if the gear bell's pitch doesn't mate with common
  1/10 spurs, that is a real reason and would change the pick.
- **The gear spec question came off the shelf early.** The previous entry
  deliberately held back the bell's tooth count and module for a later round.
  The V-groove recommendation made it load-bearing for the variant choice, not
  just the ratio math, so it went into this email after all.
- **Follow-up SENT 2026-08-13.** Clutch purchase still blocked, now on the
  magnet-coexistence answer alone plus the bore number. One of the two original
  blockers (keyway) is cleared.
- **Process note that keeps earning its keep:** the vendor answered the question
  they found easiest to answer, not the one asked, in both rounds. Read vendor
  replies against the original question text, not for general reassurance.

---

## 2026-08-14 — EngineDIY answers round 3 BY VIDEO: clutch + CDI magnet flywheel fit together (demonstrated)

Instead of a text reply, EngineDIY sent a 76-second hands-on video (1080p60,
kept out of the repo — it lives locally as `111.mp4`) of someone assembling the
clutch onto an ST-NF2 with the gas/CDI parts on the bench. Reviewed
frame-by-frame. **The audio contains no narration** — verified by Whisper
transcription with VAD (empty transcript) and a speech-band level analysis
(everything under −30 dB); the video is visual-only, so the frames are the
whole message. (An unfiltered Whisper pass hallucinated "4.5mm/5.5mm" from
parts-handling noise — those numbers are NOT real and must not be cited.)

- **THE BLOCKING QUESTION IS ANSWERED: yes, the clutch and the CDI magnet
  flywheel coexist — the clutch is BUILT ON the magnet flywheel.** The
  demonstrated stack, inboard to outboard:
  1. Stock flywheel and M6 nut come off the crank nose (the crank's cross-pin
     is briefly visible on the bare nose, ~0:08).
  2. The **CDI magnet flywheel** goes on — the deep knurled cup with a
     **pressed-in magnet clearly visible in the rim face** (~0:30). The cup
     shape clears the start pulley and belt entirely. Set screws are tightened
     with a hex key; the crank pin plus the nut do the axial/drive retention.
  3. A **backing disc** drops over two **drive pins that protrude from the
     flywheel face** (~0:44–0:48).
  4. The **blue anodized shoe carrier with its garter spring** indexes onto
     those pins (~0:58) — the shoes are **pin-driven off the flywheel face,
     not friction-driven**. This also substantially answers the
     what-carries-the-torque question at the clutch stage.
  5. The **bell/drum rides a bearing on a central stub** and is retained by a
     small screw at the nose end (~1:02–1:15). Bell spins free of the crank
     below engagement, as it should.
- **So one flywheel does both jobs**: trigger magnet for the CDI Hall sensor
  AND clutch carrier. The three-flywheels-one-nose conflict collapses to a
  supported, demonstrated configuration. The clutch is buyable as far as
  compatibility goes.
- **Still NOT answered by the video:**
  - **Bore diameter in mm** — no caliper appears in the video. Still owed.
  - **Bell tooth count / module** — worse, the demonstrated bell looks like a
    **smooth drum with no gear pinion and no obvious V-groove** in any frame,
    so which variant was filmed is unclear; possibly the pulley face bolts on
    separately. The gear-spec question stands.
  - M6 thread pitch/hand, mounting-hole topology, one-way bearing ID —
    untouched, as expected for a clutch video.
- **New packaging fact for the crank-nose stack-up (feeds the MGU-K mount
  design):** the assembled clutch stack visibly LENGTHENS the nose — flywheel
  cup + shoes + bell + retaining screw occupy everything outboard of the start
  pulley. The MGU-K drive must couple to a crank-speed feature (flywheel body
  or the central stub), and the only crank-speed face left exposed at the end
  is that small retaining screw. Draw the stack from these frames before
  designing the mount; extracted stills are worth keeping alongside the CAD.
- **Process note:** after two rounds of answering the easiest question, a video
  was their best answer yet — a demonstration can't dodge. Worth explicitly
  inviting photos/video in future vendor questions ("a phone photo answers this
  completely" phrasing already used for the mounting-pattern question).
- **Next actions:** re-ask ONLY the two open clutch items (bore number, bell
  gear spec / which variant was filmed); everything else on the clutch is
  cleared. Order stays held on those two.

## 2026-08-14 (later) — Clutch research pass: video variant identified, single-gear pick re-derived from our own requirements, order unblocked

Adam asked two questions: does the missing bore number still block the
purchase, and do we need the vendor at all for the variant choice, or can we
conclude it ourselves? Did the listing-photo research. Answers: no, and
ourselves.

- **The variant in the video is the V-GROOVE BELT version — the one they
  recommended.** The [ST-NF2 clutch kit listing](https://www.enginediy.com/collections/toyan-l200-accessories/products/clutch-assembly-kit-for-semto-st-nf2-engine-model)
  (6 variants, $27.99–36.99) shows all variants share IDENTICAL internals —
  flat two-pin flywheel disc, blue shoe carrier with garter spring, black hex
  center adapter, two small bearings, retaining screw — and differ ONLY in the
  bell: gear pinion, V-groove, double-V, synchronous (timing) pulley, or
  marine collet-cup. The bell in the video has the stepped snout with
  circumferential grooves of the V-groove bells (the ridges visible at ~1:15
  are the V-channels — earlier read as "smooth drum, no gear" was wrong on
  this detail). **Consequence: the compatibility demonstration transfers to
  every variant**, since the flywheel/shoe architecture is common; only the
  output interface changes.
- **Bore number: officially demoted from order-blocker to
  measure-on-arrival.** The worry was a machining mismatch with the 8 mm
  pin-driven nose; a factory video of the kit hand-assembling onto a real
  ST-NF2 retires that risk. Under the design-for-slop rule the caliper governs
  on arrival anyway.
- **Single-gear pick re-derived from OUR requirements (not their advice):**
  1. **The regen slip-cut requires positive drive on the clutch output.** The
     firmware detects clutch slip as VESC-RPM vs wheel-RPM divergence. A
     V-belt is a friction drive that slips under torque transients — belt slip
     is INDISTINGUISHABLE from clutch slip in that comparison, so the V-groove
     variants would blind the exact protection they feed. (Note: this concern
     is about the clutch BELL's belt interface, not the spur — a gear bell
     into a spur is positive drive with no slip mode short of tooth failure.)
  2. **Synchronous (timing) pulley is positive drive and technically
     acceptable**, but loses on ecosystem: a ~5:1 reduction needs a large
     mating timing pulley in an axle-compatible bore (scarce), adds belt
     tension/center-distance constraints to a custom chassis, can skip teeth
     under shock, and costs $7 more for no advantage over a gear.
  3. **The gear bell mates with the entire 1/10 nitro ecosystem** — spurs in
     every tooth count, and the planned HSP 02044 brake hardware mounts on
     the same spur/topshaft layout. Pinion in the photos is ~15–16T.
  4. **Their V-groove recommendation is rational FOR THEIR CUSTOMERS** —
     these kits are marketed for model ships and generators, where a V-belt's
     misalignment forgiveness and shock absorption are virtues and slip is
     harmless. They don't know our regen control loop exists. Overriding
     their advice is justified by analysis, not stubbornness.
- **Gear module/tooth count: still unpublished anywhere** (listing, Stirlingkit
  mirror, reviews). NOT worth a vendor round: the spur order is already gated
  on the tire pick and the bench-measured engagement RPM, so the sequence is
  order clutch → caliper the pinion on arrival (mod = OD/(N+2)) → buy the
  matching spur. Worst-case exotic pitch → print a bench spur at the lab or
  buy the kit ecosystem's own mating gear. A [Stirlingkit RTR crawler built on
  this engine](https://www.stirlingkit.com/products/modified-toyan-fs-l200-1-10-2-4g-4ch-nitro-offroad-crawler-vehicle-rc-car-rtr)
  proves the car conversion path exists but publishes no gear specs either.
- **DECISION: order the plain Single-gear Clutch, $27.99** ([listing](https://www.enginediy.com/products/single-gear-clutch-assembly-rc-model-ship-upgrade-part-for-toyan-fs-l200-double-cylinder-4-stroke-methanol-engine-model),
  in stock, 3 reviews, one photo-confirmed on a Toyan; current 10%-off code
  "2026" on the site). Nothing blocking remains. It also unblocks the
  engagement-RPM bench measurement on the surrogate rig, which the
  transmission ratio and regen-window math wait on.
- **Vendor thread narrows to the machining-critical unknowns only:** M6 nose
  thread pitch/hand and the mounting-hole topology. No more clutch questions
  needed. (Optional courtesy: nothing. The gear spec self-serves with
  calipers.)
- **ORDERED same day: Single-gear Clutch, $25.20 total** (list $27.99; the
  site's 10% code brought it down). On arrival: caliper the bore and the
  pinion (OD + tooth count → module), then the spur order can follow the tire
  pick and the bench-measured engagement RPM.

<!-- Append new entries at the bottom, newest last: ## date — headline, then bullets for progress / problems / resolutions. -->
