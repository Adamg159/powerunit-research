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
- **Problem: the third board was fried during tuning** — scrapped. Inventory is now
  2 working bucks, 0 spares.
- Coverage check against the plan: only two 5 V rails are ever called for — the Phase 1
  telemetry logger and the Phase 2 ignition controller (same one-active-plus-spare logic
  as the 3× ESP32 buy). So **nothing is blocked**; Phase 1 needs just one buck.
- Decision: restore the spare, but no rush — MP1584 3-packs are ~$7, so ride it on the
  next parts order (deferred engine-arrival items) rather than paying shipping for a
  standalone order. Until then the bench runs with zero buck margin, which is acceptable
  for bench work on USB-powered sensor tests.

---

<!-- Append new entries at the bottom, newest last: ## date — headline, then bullets for progress / problems / resolutions. -->
