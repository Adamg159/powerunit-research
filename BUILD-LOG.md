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

## 2026-07-24 (expected) — Main electronics arrival

Remaining electronics order due. Arrival-day checklist (document 3, Part 2) to run while
return windows are open:

- [ ] Flash blink + WiFi-scan sketch on all 3 ESP32s (swap cable before blaming a board)
- [ ] Dial each MP1584 buck to 5.0 V with a multimeter BEFORE it touches anything
- [ ] Run h2testw full write/verify on the Lexar 32 GB microSD
- [ ] Solder headers onto MPU-6050 boards (no friction fit on a vibration sensor)
- [ ] Breadboard a Hall sensor (5 V supply, 10 k pull-up to 3.3 V); polarity-test and
      paint-mark each SmCo magnet's working face (A3144 is unipolar)
- [ ] Wire a MAX31855, confirm sane room-temp reading and fault bits
- [ ] SD module VCC from 5 V rail (never 3.3 V); drop SPI clock if writes are flaky

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

---

<!-- Append new entries at the bottom, newest last: ## date — headline, then bullets for progress / problems / resolutions. -->
