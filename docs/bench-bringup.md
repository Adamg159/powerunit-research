# Bench bring-up — ordered procedure

Written 2026-08-16, the day after the full bench set landed. This is the
running order for turning a table of boxes into a working surrogate rig.
It exists because the natural order (plug the exciting things in first) is
the wrong one: two safety gates are still open, and they block different
work. Everything below is ordered so nothing waits on a gate it doesn't
actually need.

Log every step's result in BUILD-LOG as it happens — the failures are the
portfolio content.

## The two open gates

| Gate | Blocks | Clears when |
|---|---|---|
| **G1 — fire kit.** Kidde FA110G ordered $24.97, arriving ~Aug 17. Sand tub and non-combustible surface (ceramic tile / steel tray, ~$5–10) still unbought. | **Charging any pack**, and leaving a pack at rest anywhere but a non-combustible surface. | Extinguisher in hand AND tile/tray + sand on the desk. |
| **G2 — guard.** No anchored, reach-blocking guard exists yet. A clear storage tote inverted over the rig, notched for wiring, satisfies both bench rules at ~$8. | **Anything that spins** — including VESC motor detection, which spins the motor. | Tote on the desk, notched, sitting over the rig under its own weight. |

**Add the clear tote to the same errand as the tile and sand.** It was never
on a purchase list because the bench rules describe it as free/improvised, so
it can quietly fail to exist. It is the single item standing between today and
a spinning rig.

Neither gate blocks Stages 1–4 below. Do those first; they are most of the
day's work and all of the fiddly soldering.

## Stage 1 — Label the packs (5 minutes, do it before anything else)

The delivered packs are **EMEPOVGY**, not the Zeee named in the plan — same
spec class, note it for warranty. CM5202 is the 3S 11.1 V traction pack;
CM5201 is the 2S 7.4 V surrogate pack. They are near-identical in the hand
and share XT60.

Paint-pen both, large, on **both faces and the balance lead**: `3S TRACTION`
and `2S SURROGATE`. The failure this prevents is 3S into the surrogate
car-ESC, which over-revs the coupled MGU-K.

While the meter is out: read each cell on the balance leads and write the
resting voltages in BUILD-LOG. Expect ~3.8 V/cell storage charge. **Reading
is not charging — this is not gated by G1.** A cell more than ~0.05 V off its
neighbours, or any pack below ~3.5 V/cell, is a warranty conversation before
it is a bench part.

Clip a low-voltage buzzer to each balance lead now so it's habitual later.

## Stage 2 — Radio failsafe ritual (unblocked, zero traction power)

Run the day-one configuration and both bench acceptance tests already
specified in [radio-setup.md](radio-setup.md) — RX SET output mode, the four
per-channel failsafe positions including the sentinel channel, then Test A
(TX off) and Test B (serial-path loss).

Needs only the TX, RX, a servo or two, and the electronics rail. No traction
pack, nothing spinning. It is the one bench task with a written pass/fail
already waiting for it, which makes it the right thing to do first with
tools in hand.

**Meter the SERVO pin's idle voltage before it touches any ESP32** — if it
idles near 5 V, insert the 1k/2k divider first.

## Stage 3 — ESP32-S3 arrival test (unblocked)

Flash [arrival-test](../firmware/arrival-test/arrival-test.ino) to all three
new boards and record their eFuse MACs alongside the three WROOM-32 identities
in [firmware/README.md](../firmware/README.md).

Two changes from the WROOM-32 procedure documented there:

- FQBN is `esp32:esp32:esp32s3`, not `esp32:esp32:esp32`; IDE board is
  **ESP32S3 Dev Module**.
- **Use the UART Type-C port, not the native-USB one.** Per the pin budget,
  native USB costs GPIO 19/20. Establishing the habit now avoids a board that
  only ever enumerates one way.

Testing all three, not one, is the point of buying three: it distinguishes a
bad board from a bad procedure while there's still a return window.

## Stage 4 — Hall sensor adapter + motor timing (unblocked, the fiddly bit)

**4a. Splice the JST-ZH 6-pin (motor) → JST-PH (VESC) sensor adapter.** Map by
FUNCTION, not by position — the two connectors' pinouts are not a rotation of
each other. Per both manuals: 5 V, GND, temp, and three hall lines.

- Multimeter-verify 5 V and GND on the VESC side **before the motor end is
  ever connected**. Reversed 5 V/GND kills the hall ICs instantly and silently.
- The three hall wires can land in any order — VESC detection sorts them.
- **Keep the temp wire.** It carries the motor NTC that the thermal foldback
  depends on; assist duty is built on it.
- Strain-relieve both ends. Heat-shrink is in the box.

If the Flipsky box turned out to have no sensor pigtail, the fallback is the
~$9 pre-crimped JST-PH kit (B08T89ZK2Q) — check the box before ordering.

**4b. Set the motor end-bell timing to the zero mark.** Non-negotiable before
any four-quadrant or regen use: a 17.5T with advanced timing behaves
asymmetrically between drive and brake, and the regen half is the half this
project is about. Do it now, with the motor unpowered on the bench, not later
with wires attached.

**4c. Bolt the MGU-K to the blue aluminium bracket and clamp the bracket to
the desk.** Do this before any powered test, coupler or no coupler — Stage 5's
motor detection spins the motor, and an unbolted 3650 on a benchtop walks.

## Stage 5 — VESC bring-up (gated on G2: guard)

Requires the tote. Motor detection spins the motor, and the bench rules'
entanglement hazard is live the moment anything rotates.

Before spin-up, and in this order:

1. Guard positioned. **Decide where the multimeter probe leads live**, and
   anchor every cable off the shaft line. Nothing draped.
2. XT60 disconnect sited **to the side**, reachable without crossing the
   rotating parts.
3. Traction pack at storage charge is fine for detection — this stage needs
   no charging, so G1 stays clear.

Then: VESC Tool over USB, firmware check, motor detection (sensorless params
first, then hall detection), and confirm the sensored startup is smooth from
standstill in both directions.

**Coupler stays OFF through this entire stage.** One motor at a time.

## Stage 6 — Set and prove every limit (gated on G2, before any regen)

The limits are the no-BMS decision's entire safety argument, so they get
proven, not merely typed in:

- Regen/charge current cap **−10 A**
- Battery cutoff start/end **10.2 V soft / 9.9 V hard**
- Charge ceiling **4.15 V/cell** (charger-side)
- Motor NTC temp foldback enabled, using the temp wire from 4a
- Command timeout **200–300 ms**

**Unplug-test the timeout**: command a steady output, pull the control link,
and confirm the output stops inside the window. A limit that has only been
typed into a config field is not a limit. All of this happens **before the
first regen event**, per the plan.

## Stage 7 — Surrogate rig (gated on G2, and this is where the fire kit starts to matter)

Couple the GoolRC 3900 kv surrogate 1:1 to the MGU-K with the aluminium jaw
coupler. The surrogate runs on **its own 2S pack** — never the bench PSU, never
the traction pack.

Free-spinning, the surrogate is the fast one: ~32,000 rpm no-load on a fresh
2S. Coupled, that's a rigid coupler at ~30,000 rpm at arm's length. Guard down.

Commanded by a servo tester (two arrived) until the radio takes over.

This is where the first **sustained** regen test happens — tens of watts into
the traction pack for minutes, not the sub-second free-spin flick. That test
means a pack that has been charged and will be cycled, so **G1 must be closed
before it**: extinguisher present, packs charging and resting on the
non-combustible surface.

## Stage 8 — Numbers worth capturing while the rig is up

The rig is also a measurement instrument, and these feed decisions that are
currently blocked on guesses:

- **Coupled spin-down inertia**, which sets the ~10–20 % effective-mass adder
  (accel predictions should use ~3.4–3.7 kg, not 3.0).
- **Deliverable assist power at RPM on a sagged 3S pack** — checks the 17.5T
  motor pick against the ~390–470 W figure it was chosen on.
- Thermal behaviour of 2–5 s assist bursts, which is what the duty timer gets
  set from.

Clutch engagement RPM — the other big open number — waits on the clutch, which
is ordered but not delivered.

## What this sequence deliberately does not do

- **No pack charging until G1 closes.** Storage charge carries Stages 1–6.
- **No coupler until Stage 7.** Two motors on one shaft is not a bring-up
  configuration; it's a test configuration.
- **No engine anything.** Still backordered to late August.
