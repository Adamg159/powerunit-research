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

---

<!-- Append new entries at the bottom, newest last: ## date — headline, then bullets for progress / problems / resolutions. -->
