# Radio system — selection, wiring, and failsafe reference

Selected 2026-08-07 after a manual-verified market pass. Purchase pending
approval (~$86). This file is the bench reference for wiring and the
day-one failsafe ritual.

## The system

- **Transmitter: Flysky FS-G7P+** — 10-channel pistol grip, LCD, per-channel
  RX configuration from the TX (`OK → function menu → RX SET`).
- **Receiver: FS-R11P** — 11 PWM channel ports PLUS a dedicated `SERVO`
  serial port (iBUS or SBUS), simultaneous **by hardware design** (verified
  in the official FS-R11P manual: ports [1]–[6] = CH1–6, [9]–[13] = CH7–11,
  [8] = VCC/BVD 3.5–9 V, [14] = SENS telemetry, [15] = SERVO serial out).
  No integrated gyro to fight. Spare RX: FS-R11P ~$30, or FS-R7P ~$21 (same
  ANT protocol and same PWM+SERVO architecture).
- Combo: ~$86.49 Amazon ([B0C89458YY](https://www.amazon.com/dp/B0C89458YY));
  backup listings B09SLN2ZDM / spare RX B0FHW7V79V. G7P+ pairs with the
  FS-R11P/R7P/R7D/R7V family — NOT the older FS-BS6/AFHDS-2A receivers.
- **Why it won:** only candidate passing every hard requirement on
  primary-source documentation — simultaneous PWM + serial by design, and
  the ONLY vendor that documents serial link-loss signaling (SBUS byte-23
  failsafe flag bits, called out verbatim in the manual). The classic
  FS-iA6B was rejected on evidence: it repeats stale frames on link loss
  with no flag (IBusBM issue #24) — disqualifying for Phase B safety.
- **Backup system (if Flysky stock dies):** Radiolink RC6GS V3 + R7FG
  (~$75, B09Y8H434N). Simultaneity confirmed (Mode 2: CH1–6 PWM + CH7
  SBUS) but failsafe-flag behavior is undocumented, the serial out eats a
  channel, and the gyro must be verified OFF (green-only LED) every session.

## Wiring map

| Consumer | RX port | Notes |
|---|---|---|
| Steering servo | CH1 | Plain PWM, RX-direct — never touches the ESP32 |
| Engine throttle servo (Phase A) | CH2 | RX-direct. **No Y-lead needed**: the ESP32 reads the same CH2 demand from the serial stream |
| Opto ignition-kill (post-CDI) | CH3 | PWM into the opto input side; opto output crosses to the ignition world per EMI rules |
| ESP32 serial tap | SERVO [15] signal → UART2 RX (e.g. GPIO16 — non-strapping) | Grounds common at the electronics rail; RX + servos powered from the electronics rail into [8] VCC/BVD |
| Assist/regen mode select | any of CH4–CH11, assigned to a TX knob/switch | ESP32 reads it from the serial stream — zero extra wiring |

## Day-one failsafe configuration (from the G7P+ RX SET menu)

1. Output mode: **PWM + S.BUS** (channel ports stay PWM; SERVO port streams
   SBUS — chosen over iBUS because SBUS carries documented failsafe flags).
2. Per-channel failsafe (modes Not Set / OFF / ON):
   - CH2 throttle = ON, position captured at **carb idle**
   - CH3 kill = ON at the **KILL-asserting** value (pick kill polarity so
     link-loss failsafe AND opto-dark both mean engine dead)
   - CH1 steering = ON at center
   - One unused channel (e.g. CH10) = ON at an extreme value the TX never
     commands — a sentinel the ESP32 cross-checks besides the flag bits
3. Failsafe judgment time: 300 ms default is fine (settable 250–1000 ms).
4. **Meter the SERVO pin's idle voltage before it touches the ESP32** — if
   it idles near 5 V, insert a 1k/2k divider first.

## Bench acceptance tests (log results in BUILD-LOG)

- **Test A — transmitter off (RF loss):** drivetrain clear, engine off.
  Kill the TX: within ~300 ms steering centers, throttle drives to idle,
  CH3 asserts KILL, and the ESP32 log shows byte-23 bits set (bit 2
  frame-lost, bit 3 failsafe) plus the sentinel. Power TX back on; verify
  clean recovery.
- **Test B — serial-path loss (distinct failure):** link healthy; pull the
  SERVO lead from the ESP32. The ESP32's ~100 ms frame timeout must force
  zero-assist/idle VESC commands while steering and throttle keep working
  on their PWM ports — a dead serial link or dead ESP32 never costs
  steering. Repeat with the ESP32 held in reset to simulate a crash.

## ESP32 integration

- SBUS = 100000 baud, 8E2, **inverted** UART; ESP32 inverts natively:
  `Serial2.begin(100000, SERIAL_8E2, RX_PIN, -1, true);` — RX-only, no TX
  pin, no external inverter.
- Frame: 25 bytes; 0x0F header; 22 bytes = 16 channels × 11 bits; byte 23
  flags (bit 2 frame-lost, bit 3 failsafe); 0x00 footer; ~7–14 ms cadence.
- Fallback: same port switches to iBUS (115200 8N1, IBusBM-compatible) if
  SBUS parsing misbehaves — but iBUS has no flag bits; you'd lean on the
  sentinel channel + frame timeout instead.
- 2.4 GHz coexistence with WiFi telemetry: ANT is FHSS and tolerates WiFi,
  but keep the RX antenna several cm from the ESP32 (≥1 cm from carbon or
  metal per the manual), lock the telemetry AP to a fixed channel, disable
  ESP32 Bluetooth, and run the pre-drive range walk-test WITH telemetry
  actively streaming — that is the realistic worst case.
- RX SET settings live in the TX+RX pair: re-verify failsafe behavior after
  any rebind or firmware update (FlyskyAssistant 3.0+).
