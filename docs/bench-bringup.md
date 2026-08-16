# Bench bring-up — ordered procedure

Written 2026-08-16, the day after the full bench set landed. This is the
running order for turning a table of boxes into a working surrogate rig.
It exists because the natural order (plug the exciting things in first) is
the wrong one: two safety gates are still open, and they block different
work. Everything below is ordered so nothing waits on a gate it doesn't
actually need.

Log every step's result in BUILD-LOG as it happens — the failures are the
portfolio content.

## Safety gates — BOTH CLOSED 2026-08-16

| Gate | Blocked | Status |
|---|---|---|
| **G1 — fire kit** | Charging any pack; pack rest | **CLOSED.** Kidde FA110G delivered. Sand and non-combustible charging surface available at the work location. |
| **G2 — guard** | Anything that spins, incl. VESC motor detection | **CLOSED.** Guarding available at the work location. |

Work is happening at **an acquaintance's workbench**, not the wall-anchored
desk the original bench rules were written against. Two consequences, neither
blocking:

- **Re-check the rigidity assumption before Stage 7.** The "wall-anchored, so
  coupler runout won't walk the rig" argument was about *that* desk. Press
  down on a corner of this one: if it rocks or shuffles, clamp the motor
  bracket to it rather than trusting mass, and keep the guard weighted.
  Runout imbalance at ~30,000 rpm is what walks a rig, and a bench that walks
  puts the coupler somewhere you didn't plan for.
- **Charging a 5200 mAh pack is someone else's property risk.** Tell the owner
  what's charging, where, and on what surface, before the first charge — not
  as ceremony, but because they should be able to say no, and because they
  need to know what the tub of sand is for if they walk in on it.

All stages are now unblocked. Stages 1–4 still come first: they are the fiddly
soldering, they need no rotation, and Stage 4c is a prerequisite for Stage 5.

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

## Stage 4 — Sensor cable check + motor timing

**4a. NO SPLICE NEEDED — confirmed 2026-08-16 by photo.** The Flipsky box
included a sensor cable that mates the Hobbywing motor's ribbon directly:
motor end-bell → black 6-conductor ribbon → white 6-pin joint → coloured
pigtail → VESC sensor port. The planned solder job is deleted, and so is the
~$9 pre-crimped JST-PH fallback kit (B08T89ZK2Q) — do not order it.

**VESC SENSE port pinout** (from the Flipsky wiring sheet, connector 2), in
order along the header:

```
GND | H3 | H2 | H1 | TMP | 5V
```

**5 V and GND sit at opposite ends of the same 6-pin connector.** That is
precisely the geometry where a flipped cable puts 5 V onto ground — one
mis-orientation, both rails swapped, hall ICs dead. Note also that the same
sheet gives the COMM port as `5V | 3.3V | GND | ADC | TX | RX | ADC2`, which is
the ESP32 UART link for B4.

**Reversal risk is retired: the connectors are keyed** (confirmed on the parts
2026-08-16). They physically cannot be mated backwards or upside down, so the
hand-error path to 5 V on ground doesn't exist here.

What keying does *not* guarantee is that the two vendors agree on pin order
behind those keyed housings — Flipsky's cable and Hobbywing's motor are
independently designed, and a keyed connector will happily present a wrong
mapping in the one orientation it allows. The residual risk is low (both follow
the standard RC sensored layout, and Flipsky ships this cable for exactly this
motor class), so this is now **a 30-second confirmation, not a gate**:

1. Unmate the pair; keep the motor out of it.
2. Power the VESC from **USB only** — no pack on the bus.
3. Probe each pigtail pin against battery-negative. Expect exactly one pin at
   ~5.0 V (red) and one at 0 V (black).
4. Confirm the motor-side connector puts its Vcc and ground on those same two
   positions rather than crossed.

The three hall wires can still land in any order — VESC detection sorts them.

**While the meter is out: does the motor actually have a thermistor?** The 6th
conductor is nominally the temp line, but plenty of sensored RC motors leave it
unpopulated behind a fully-pinned connector. Measure resistance between the
temp pin and ground at the *motor* connector:

**RESULT 2026-08-16: ~10 kΩ — the NTC is fitted.** Motor-temp foldback is
viable, so the manual's 90 °C ceiling becomes something the controller enforces
rather than something to respect by hand. No separate sensor needed, and the
burst-duty argument now has a mechanism behind it.

**Still to pin down: the beta coefficient.** 10 kΩ gives nominal resistance,
not how steeply it falls with temperature. VESC Tool's common 10 K options
(beta 3380 and 3435 in most versions) agree at 25 °C by definition and diverge
at the top of the range — roughly 10 °C apart where the foldback actually
operates. In a motor where 90 °C permanently demagnetises the magnets, that is
the wrong place to carry a blind error.

Two-point method, ten minutes:

```
beta = ln(R1/R2) / (1/T1 - 1/T2)      T in kelvin
```

Point one is room temperature — **measure the actual room temperature, do not
assume 25 °C.** For point two, warm the can (hair dryer, or sealed in a bag in
hot water) and read resistance and can temperature at the same moment. Aim for
50 °C+: the wider the spread, the less a temperature error matters. Then pick
the closest VESC option.

Pragmatic alternative: set the default 10 K option, then during the first
sustained runs tape one of the owned **MAX31855 thermocouples** to the can and
compare it against the VESC's reported motor temperature. Two are on hand and
already tested — a good use for one before the engine needs them.

Setpoints either way, from the manual's hard limit: **foldback starts
70–75 °C, fully cut by ~85 °C, never reach 90 °C.**

**4b. Set the motor end-bell timing to the zero mark.** Non-negotiable before
any four-quadrant or regen use: a 17.5T with advanced timing behaves
asymmetrically between drive and brake, and the regen half is the half this
project is about. Do it now, with the motor unpowered on the bench, not later
with wires attached.

**4c. Bolt the MGU-K to the blue aluminium bracket and clamp the bracket to
the desk.** Do this before any powered test, coupler or no coupler — Stage 5's
motor detection spins the motor, and an unbolted 3650 on a benchtop walks.

## Stage 4d — Pack connector and fuse (prerequisite for everything past A4)

The VESC ships with bare tinned battery leads. **Solder the mating XT60 on, and
put the 50 A MAXI blade fuse in the positive lead while it is open** — the
holder and fuses arrived with the bench set, and they live in the same wire.

```
pack (+) → 50 A MAXI fuse holder → VESC red
pack (−) →                          VESC black
```

**Why a temporary friction/taped joint is not acceptable past the A4 voltage
check:** contact resistance is high and variable, so at the ~41 A of an assist
burst it becomes a heater; and a joint that parts under vibration **opens a live
inductive DC bus**, which is the load-dump failure mode the no-BMS decision was
built around — a disconnect mid-regen spikes voltage into the controller. It
also re-sparks the cap inrush every time it chatters. Separately, two bare leads
near a 5200 mAh 80C pack is a short waiting to happen, and that pack will supply
hundreds of amps into one.

For a 30-second, milliamp-level voltage measurement with the joint watched and
nothing conductive nearby, tape will survive. It must not survive to Stage 5.

Practical:

- **Verify polarity against the pack before heating anything.** Red = +,
  black = −. Reversed polarity destroys a VESC instantly.
- **One lead at a time, heat-shrink slid on first, insulated before starting the
  second.** Two bare joints open simultaneously is how a pack gets shorted.
- **Iron: the owned Miniware TS101 + 65 W PD supply is adequate** (bought
  2026-07-22, verified genuine 07-23). Two conditions: it must be on the 65 W
  supply with a 3 A-rated cable, since the TS101 only reaches full power at
  20 V — a thin charge cable that caps at 5 V gives a 15 W iron with no warning;
  and **use a chisel/bevel tip (TS-D24 / TS-C4 class, ~$8–12)**, not the stock
  fine conical. Tip geometry matters more than wattage here: a fine tip has
  almost no thermal mass, so you dwell, and dwelling is what deforms the XT60
  housing. ~380–400 °C, flux both sides, tin the wire and flood the cup
  separately, then bring them together.
- **The XT60 came pre-leaded** (confirmed by photo 2026-08-16), so there is no
  cup soldering and no plastic near any joint — the melted-housing risk is gone
  and the chisel tip drops from near-required to merely preferable.
- **Three joints, all wire-to-wire**, because the fuse is inline:
  `VESC red → fuse tail`, `fuse tail → XT60 (+)`, `VESC black → XT60 (−)`.
  Strip ~8 mm, tin both, lap the strands **side by side rather than butted** —
  more contact area and it survives vibration — flow solder through, shrink.
  Slide the heat-shrink on before soldering.
- **Dry-fit the XT60 against the pack BEFORE soldering.** Wrong gender is
  discovered cheaply now or expensively after three joints.
- **Fuse CONFIRMED 2026-08-16: MAXI, 50 A, 32 V** — printed on the fuse body.
  Correct on all three counts: MAXI (standard ATC stops at 40 A, so a 50 A ATC
  fuse does not exist), 50 A as specified, and 32 V is comfortably above a 3S
  bus. Nothing to change. A 30/40 A fuse would have nuisance-blown during ~41 A
  assist bursts and presented as a controller cutout — miserable to diagnose.
- **Check whether the fuse-holder tail is copper or CCA** (copper-clad
  aluminium — common in cheap automotive wire, identifiable by silvery rather
  than copper strands). CCA has materially higher resistance and solders poorly
  because the cladding is thin. In the main positive lead at 40 A, replace it
  with silicone copper wire if so.

## Stage 5 — VESC bring-up

First stage where something rotates. Motor detection spins the motor, so the
full pre-spin ritual applies even though the coupler is off.

### 5.0 Pre-spin ritual (every session, not just the first)

1. Guard positioned and weighted.
2. **Decide where the multimeter probe leads live** and anchor every cable off
   the shaft line. Nothing draped. Entanglement is the hazard, not fragments.
3. XT60 disconnect sited **to the side** — reachable without reaching across
   the motor.
4. Sleeves, lanyards, cable tails accounted for.

Traction pack at storage charge is correct for this stage. Nothing here needs
a charged pack, and a partly-empty pack is the safer one to make mistakes on.

### 5.1 Firmware first, config second

Connect over USB, read the firmware version, and **do any firmware update
before touching configuration** — updating wipes the config, so a careful
setup done first is a setup done twice.

USB alone powers the logic side; motor detection needs the pack connected.

### 5.2 Detection order

1. **Set the battery parameters before detection**: LiPo, 3S, so the cutoffs
   land somewhere sane while detection is drawing current. Stage 6 tightens
   them.
2. **FOC detection** — resistance and inductance (no rotation), then flux
   linkage (spins the motor).
3. **Hall sensor detection** — separate step, spins slowly, writes the hall
   table. Wire order doesn't matter; this is the step that sorts it.
4. Set sensor mode to hall, with sensorless handover above the crossover ERPM.

**Pole pairs = 2** on a 3650 (4-pole). This matters far beyond the config
field: **the VESC reports ERPM, not mechanical RPM.** Mechanical RPM =
ERPM / 2. Since the MGU-K is crank-mounted, the VESC *is* the crank
tachometer, and the regen slip-cut compares that number against a
wheel-derived RPM. **A missing divide-by-2 there is a 2× error in the
comparison that gates regen on a self-reinforcing clutch slip.** Put the
conversion in one place in firmware and name it clearly.

### 5.3 Verify before moving on

- Smooth startup from standstill in **both** directions at low duty — that is
  what the sensored path was bought for; if it cogs, the hall table is wrong.
- **Sanity-check the motor temperature reading at room temperature.** The NTC
  arrives via the temp wire from 4a and needs the right sensor type selected.
  A plausible-looking foldback configured against a misread sensor is worse
  than no foldback, because it will be trusted. Warm the motor can with a hand
  and watch the number move the right way.

**Coupler stays OFF through this entire stage.** One motor at a time.

## Stage 6 — Set and prove every limit (before the first regen event)

These limits *are* the no-BMS safety argument. There is no second layer behind
them, so each one gets proven rather than typed.

| Setting | Value | Why |
|---|---|---|
| Battery current max | ~41 A | Full-assist burst from the sizing study |
| Battery current max **regen** | **−10 A** | The no-BMS charge cap |
| Motor current max | ~50–54 A phase | Burst duty only |
| Battery cutoff start / end | **10.2 V / 9.9 V** | Soft foldback, then hard |
| Motor temp foldback | enabled, via 4a temp wire. **Hard ceiling 90 °C** | The duty argument depends on it |
| MOSFET temp foldback | leave at defaults | Flipsky 4.20 has no margin to spare |
| Control timeout | **200–300 ms** | Default is ~1000 ms — far too long |

**Where 90 °C comes from:** the Hobbywing manual states outright that the motor
can must never exceed 90 °C, because above that the magnets demagnetise and the
coils can melt. That is a permanent-damage threshold, not a warning — so set
foldback to *start* well below it (70–75 °C) and be fully cut by ~85 °C. The
manual also prescribes the tuning method this project should copy for gearing:
start with a small pinion, run, measure temperature, and only increase gearing
while temperatures stay low.

### The gap worth knowing about: there is no graceful high-voltage regen limit

The plan's 4.15 V/cell ceiling is a **charger-side** number. The VESC has no
equivalent "taper regen as cells approach 4.15 V" control — its maximum input
voltage is a *fault* threshold, which trips rather than tapers, and tripping
mid-regen dumps an inductive bus, which is the exact failure mode the no-BMS
decision was designed to avoid.

**So the mitigation is procedural: never begin a regen session on a full
pack.** Start at ≤4.0 V/cell (≤12.0 V pack) so there is real headroom for
harvested charge. At −10 A into 5200 mAh this is not a constraint in
practice — it is simply a session-start check. Add it to the pre-run
checklist, not to firmware.

### The unplug test

Command a steady output, **pull the control link**, and confirm the output
stops inside the timeout window. Then repeat holding the ESP32 in reset.

A limit that has only been typed into a config field has not been tested; it
has been *hoped for*. This is the test that turns the config into a safety
argument, and it happens **before the first regen event**.

Log the measured stop time in BUILD-LOG. That number is portfolio content.

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
