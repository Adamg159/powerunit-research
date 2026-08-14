# Chassis Packaging — Working Document

Started 2026-08-14, while the engine is on backorder. Style decision:
**function-first mule** — the components dictate the footprint; a body style is
committed only after the dummy-block layout shows what fits. Scale is 1/10
(per doc 01), but proportions follow the hardware, not a class rulebook.

**Outputs this work owes the rest of the project:**

1. Wheelbase / track / wheel-tire pick → unblocks steering geometry work
2. Telemetry-board envelope + mounting zone → unblocks the vehicle PCB
3. A frozen component layout the real chassis CAD grows around

Everything here is CAD-planning grade. Physical measurement on arrival governs
(design-for-slop); every number that comes from drawings, not calipers, is
marked ~.

---

## 1. Fixed constraints (established, with sources)

### Engine (SEMTO ST-NF2) — from factory manuals + BUILD-LOG 2026-08-11

| Item | Value | Confidence |
|---|---|---|
| Envelope | 112 × 90 × 92 mm, 535 g bare | listing |
| Crank axis | runs the long (112 mm) dimension; inline twin | manual views |
| Crank height above mount face | ~15 mm ± 1.5 | 3-way recon |
| Mounting | 4 × M4 blind, 38 mm across crank axis (solid), ~40–42 mm along (medium). Topology (rhombus vs rectangle) UNRESOLVED — slot every hole | recon |
| Flywheel overhang | Ø~50 flywheel hangs **9–10 mm BELOW the mount plane** → chassis plate needs a cutout at the flywheel/start-pulley end | 3-way confirmed |
| Output end | start pulley + flywheel/clutch share ONE end (the nose); other end is timing/fan | manual |
| Crank nose | ~21 mm exposed, 8 mm shank, M6 thread (pitch/hand unknown), 2× φ2 pin drive | recon |
| Exhausts, carb, plug positions | UNKNOWN until arrival — reserve side/top clearance | — |

### Powertrain stack on the crank nose — from the 2026-08-14 vendor video

Assembled order: magnet flywheel (deep cup, clears start pulley) → pin disc →
shoe carrier → bell w/ ~15–16T pinion on a bearing → nose-end retaining screw.
Clutch kit product weight 180 g. Stack length beyond the crankcase face:
**estimate ~30–40 mm, measure on arrival** — this plus the MGU-K coupling sets
the rear module length.

### MGU-K and coupling

- Hobbywing QuicRun 3650SD G2: Ø36 × ~53 mm body, 3.175 mm shaft, ~175 g
- **Direct crank-nose coupling (decided 2026-08-11), concept OPEN — the two
  candidates below are the packaging study's first job (§4)**
- VESC: Flipsky Mini FSESC4.20, ~65 × 30 × 20 mm + wiring, ~90 g

### Electrical / RC / sensors

| Item | Envelope | Mass |
|---|---|---|
| Traction pack (Zeee 3S 5200 hard case) | ~139 × 47 × 39 mm | ~400 g |
| ESP32-S3 board + telemetry PCB | envelope = an OUTPUT of this work; reserve ~90 × 60 mm tray | ~80 g |
| Receiver FS-R11P | ~47 × 27 × 15 | ~15 g |
| Servos ×3 (steering std-size; throttle + brake can be mini) | 41×20×38 / 32×12×29 | ~55 + 2×25 g |
| Fuel tank (ships with starter kit, size unknown) | placeholder ~80 × 45 × 35 (≈80 ml) | ~120 g full |
| Sensors: pitot (nose), 2× ToF ride height (front/rear underbody), 2× wheel Hall (one per axle), head temp ×2, MPU-6050, load-cell provision | small, but each needs a mounting FACE in the layout | ~60 g |

**Sensor mounting is a first-class packaging requirement, not an afterthought:**
pitot needs clean nose air; ToF needs unobstructed floor views front and rear;
wheel Hall sensors need brackets at one front and one rear upright/axle;
load-cell downforce measurement (Phase 3) needs a provision at the
chassis-to-suspension interfaces — design the towers so cells can be inserted
later without a chassis reprint.

### Other hard rules bearing on layout

- Ignition world and electronics world never share wiring or power; star
  grounds; shielded sensor runs — **physical separation of CDI/plug side from
  the ESP32/VESC tray is a layout input**
- Start pulley + belt must remain ACCESSIBLE for the starter (contents of
  starter kit unknown until arrival — do not bury the nose end)
- Brake disc lives on the driveline, chosen together with the transmission
- RWD; front wheels steer only

---

## 2. Mass budget v0 (CAD placeholder values)

| Group | g |
|---|---|
| Engine bare | 535 |
| Clutch stack + magnet flywheel | 180 |
| CDI kit (coil, box, harness) | 100 (placeholder) |
| MGU-K motor | 175 |
| VESC + wiring | 120 |
| Traction pack | 400 |
| Fuel tank, full | 120 |
| Servos ×3 + receiver | 120 |
| ESP32-S3 + PCB + sensors | 100 |
| Chassis print (PETG/ASA-class values) | 600 |
| Wheels/tires/axles/diff/suspension | 450 |
| Aero + body (deferred style) | 150 |
| Wiring, fasteners, misc | 150 |
| **Total** | **≈ 3.2 kg** |

Cross-check: BUILD-LOG 2026-08-11 effective-mass estimate (3.4–3.7 kg incl.
rotating inertia) sits consistently above this static total. **Design tires,
bearings and suspension for a ~3.0–3.5 kg vehicle** — roughly double a typical
1/10 nitro touring car. This number is why the tire pick is not automatic.

CG goals: pack low and central; fuel tank at the CG so burn-off doesn't walk
the balance; engine mass mid-rear for RWD traction.

---

## 3. The layout is more constrained than it looks

**Transverse crank is dead on arrival.** The crank line is engine (112) +
clutch stack (~35) + coupling + MGU-K (~55–70): **~210–230 mm end to end**. A
1/10-class track is ~190–200 mm outside, ~150–160 mm between the wheels.
The crank line physically does not fit across the car. **Longitudinal crank is
forced.** (Good: that is also the standard nitro-car architecture, so the
whole parts ecosystem agrees.)

Remaining orientation choice — **which way does the nose point?**

- **Option A — nose rearward (working favorite):** clutch pinion sits near the
  rear axle → spur + brake disc on a short layshaft → immediately into the
  rear diff. Shortest possible drive path, rear mass bias for traction, MGU-K
  overhangs the rear like an F1 gearbox. Costs: rear overhang moment, MGU-K
  is the first thing to arrive in a rear impact, starter access is at the tail.
- **Option B — nose forward:** standard nitro middle-shaft layout, driveshaft
  runs back to the rear diff. Costs: MGU-K and clutch crowd the steering and
  front axle exactly where Ackermann hardware wants space; front-heavy on a
  RWD car; long driveshaft. Kept alive only until the dummy block kills it
  properly.

Fore-aft, the car divides into three zones either way: **steering + pitot
zone** (needs clear width for linkage), **energy zone** (pack transverse +
fuel at CG — the 139 mm pack fits across a ~150 mm inner width, barely; check
first in CAD), **power module** (engine + clutch + MGU-K + layshaft + diff as
ONE rigid assembly on slotted mounts).

Expected outcome (to be proven, not assumed): wheelbase lands **~280–330 mm**
— stretched relative to a 255–262 mm touring car, normal for the hardware; a
mule wears its wheelbase honestly.

---

## 4. Work sequence

### 4.1 Power-module micro-study (FIRST — it sizes the rear of the car)

Resolve the crank-nose stack-up as ONE problem (CLAUDE.md rule). Two MGU-K
coupling concepts to draw against the video's stack geometry:

- **Concept 1 — flywheel rim drive:** small toothed belt off a pulley bolted
  to / clamped around the magnet flywheel's exposed rim (Ø~50, knurled),
  up to the MGU-K mounted above/beside the crankcase. Precedent: Toyan's own
  12 V generator kit taps a dedicated crank-nose pulley. Pros: MGU-K off the
  crank line (shortens the module), no interference with the bell screw.
  Cons: needs a clamp/bolt interface on a part we don't control the geometry
  of; belt tension path; ratio ≠ 1:1 (fine — VESC RPM just needs the constant).
- **Concept 2 — coaxial stub extension:** replace the kit's central hex
  adapter / retaining screw with a custom crank-speed stub that passes through
  the bell bearing and extends outboard; MGU-K couples inline behind the bell.
  Pros: elegant 1:1 inline, rigid tach. Cons: custom turned part against
  unknown M6 pitch/hand (blocked on that answer!), lengthens the module,
  MGU-K body Ø36 must clear the spur mesh right behind a Ø~18 pinion.

Decision criteria: module length, custom-machining risk (M6 unknowns), spur
mesh clearance, service access (clutch shoes are a wear item — can it come
apart without pulling the MGU-K?). Output: rear module envelope L × W × H.

### 4.2 Dummy-block CAD set

Model every §1/§2 item as a simple prism/cylinder with mass properties at
placeholder values, each on its own coordinate system, engine block including
the **flywheel overhang below the mount plane** and the **nose-end keep-out
for starter access**. Assembly datum = engine mount plane + crank axis.

### 4.3 Layout iterations

Arrange Option A vs B; pack transverse-vs-longitudinal; tank at CG. Track each
iteration's WB / track / CG height / polar moment. Kill B on evidence.

### 4.4 Tire study (design only — no purchase yet)

Selection criteria in order: **(1) load capacity at ~800 g/corner static**,
(2) diameter window from the layout (ground clearance for the flywheel
cutout + crank height), (3) availability per the sourcing rule (live stock
check + named backup at pick time), (4) a wheel/hex standard the custom
uprights can adopt (12 mm hex preferred). Candidate classes: 1/10 touring
63–65 mm (ratio math baseline), 1/10 rally ~67–75 mm (more load margin, same
footprint class), short-course ~110 mm (excessive; only if mass balloons).
**The ratio target moves with the pick:** R ≈ 4.9–5.0 holds only for 63–65 mm;
scale linearly with diameter (e.g. 70 mm → R ≈ 5.4).

### 4.5 Freeze + hand off

Freeze WB / track / tire pick → steering work starts. Freeze the electronics
tray zone → telemetry-board envelope to the PCB task. Log the frozen numbers
in BUILD-LOG and CLAUDE.md.

---

## 5. Open items this work does NOT wait for

- M6 pitch/hand + mount-hole topology (vendor thread) — only Concept 2 of the
  micro-study is exposed to it; slotted holes absorb the topology tie
- Clutch kit arrival — refines the stack length the micro-study estimates
- Engine arrival — final-fit everything; that is what design-for-slop is for
