# Power-Module Micro-Study — MGU-K Coupling + Rear Module Envelope

2026-08-14. Input to chassis packaging (§4.1 of chassis-packaging.md).
Orientation decided by Adam: **nose rearward** (performance + F1 mimicry).
All video-derived dimensions are photogrammetry against the Ø~50 mm magnet
flywheel and carry ±15% until calipers on arrival.

## 1. Measured geometry (vendor video, frames at ~0:40 / 1:12)

| Feature | Estimate | Method |
|---|---|---|
| Clutch stack protrusion from crankcase face | **~45–50 mm** | side profile, flywheel-Ø scaled |
| Flywheel knurled band, axial | ~15–18 mm | same |
| Bell drum Ø / length | ~35 mm / ~14 mm | same (matches 1/10 nitro bell norms) |
| Snout (bearing land + retainer) | ~12 mm | same |
| Exposed rim between crankcase and shoe plane | present, several mm — **the only wide crank-speed surface on the engine** | visual |
| Start belt exit direction | downward, inboard of the flywheel plane | visual |

## 2. Coupling ratio: why 1:1 is not arbitrary

The MGU-K must add torque up to 16,000 crank rpm. The motor's back-EMF
ceiling: 2170 kv on a sagged 3S (~10.5 V) gives ~22,800 rpm absolute, and
usable current headroom (V_pack − V_bemf across ~30 mΩ) collapses well before
that. The 2026-08-07 sizing (390–470 W deliverable at the top of the band) was
computed **at motor speed = crank speed** — any overdrive ratio spends that
margin: at 1.25× the motor sees 20,000 rpm where back-EMF leaves only ~1.3 V
of forcing voltage. Underdrive (motor slower) divides torque at the crank.
**Ratio = 1:1 keeps the validated sizing exactly valid.** With a belt this is
a pulley-count choice, not a shaft-line constraint.

Design torque through the coupling: Kt = 9.55/2170 ≈ 4.4 mN·m/A; at the 54 A
phase cap → **~0.24 N·m motor torque** (450 W at 16k ✓, and the same cap
governs assist at low rpm). Harvest is 2–5 A — negligible. Design the drive
for 0.5 N·m (2× transient factor).

## 3. Concept comparison

### Concept 1 — flywheel-rim timing-belt drive (RECOMMENDED)

A toothed ring clamped around the flywheel's exposed knurled rim (Ø50) drives
an equal-tooth pulley on the MGU-K, mounted parallel to the crank within the
engine's own silhouette.

- **Belt must be synchronous (timing), never V/round** — same logic as the
  clutch bell: the VESC is the crank tach and the regen slip-cut's reference;
  a friction belt would corrupt both. A timing belt only changes the tach by
  a known constant (=1 here).
- **Spec: HTD 3M, 9 mm wide, 60T ring (PD 57.3 mm over the Ø50 rim) : 60T
  motor pulley.** GT2-6 is marginal at 0.5 N·m; 3M-9 carries it with margin.
  Equal pulleys → 180° wrap, no idler. Motor pulley needs a 3.175 mm bore
  (ream or sleeve a 4 mm stock part).
- **Motor placement: above the crankcase.** Motor axis ≈ 59 mm from the crank
  axis (sum of pitch radii + clearance) puts the Ø36 body flush with the
  engine's 92 mm height — zero added width, zero added length. CG cost:
  175 g at ~74 mm height. Vertical belt run clears the start belt (exits
  downward) by design; confirm on arrival.
- **Speed check on the printed prototype ring:** rim speed at 16k rpm on
  Ø~62 mm is ~52 m/s → hoop stress in PETG ≈ 3.4 MPa vs ~50 MPa yield: 15×
  margin. A printed ring is legitimate, not a hack; machine aluminum later if
  balance demands.
- **Service/packaging wins:** nose end stays free (clutch shoes are a wear
  item; starter needs the region); **rear overhang beyond the clutch stack is
  ZERO** — the rear axle can sit at the bell plane; no dependence on the
  unknown M6 pitch/hand.
- **Risks (all checkable on arrival, none design-fatal):** rim axial exposure
  must accept a 9 mm belt + flanges (§1 says ~15 mm exists); clamp-ring grip
  on the knurl (fallback: bolt through the flywheel face holes); ring
  concentricity/balance at 16k rpm; start-belt path interference.

### Concept 2 — coaxial stub through the bell (FALLBACK)

Replace the kit's central adapter/retainer with a custom crank-speed stub
passing through the bell bearings, coupling inline to the MGU-K behind the
bell.

- Wins on elegance: rigid 1:1 inline, perfect tach, no belt.
- **Loses on every packaging axis for a nose-rearward car:** module becomes
  stack (48) + coupler (~18) + motor (53) ≈ **119 mm behind the crankcase**,
  hanging the motor ~70 mm past the rear axle line — 175 g of polar moment at
  the worst address on the car.
- **Blocked on the M6 pitch/hand answer** (custom threaded stub), needs
  precision turning (runout at 16k rpm), and buries the clutch behind the
  motor for service. The spur must also clear the Ø36 body right behind a
  Ø~18 pinion.
- Keep alive as fallback if the rim's exposed band measures too narrow.

### Score (module length / machining risk / service / tach fidelity / CG)

Concept 1: **win / win / win / tie(-ε: belt-skip risk, mitigated by flanges +
tension) / tie(+high motor vs +long overhang)**. Concept 2 wins only on
mechanical purity. **Decision: Concept 1, pending two arrival measurements
(rim exposure, start-belt clearance).**

## 4. Rear module envelope (output to chassis packaging)

Nose rearward, Concept 1:

- **Length:** engine 112 + stack ~48 ≈ 160 mm, then layshaft/spur + inboard
  brake + rear diff zone ~40 mm overlapping the stack end → **rear module
  ~200 mm of chassis length**, rear axle near the bell plane → minimal rear
  overhang (bumper/diffuser only).
- **Width:** engine-governed, ~90 mm (+ exhaust reserve TBD on arrival).
- **Height:** engine-governed, ~92 mm; MGU-K inside that silhouette.
- **Layshaft placement freedom:** the 16T:~80T mod-0.6 mesh needs a ~29 mm
  center offset from the crank axis, direction free — place it low-and-left
  toward the diff, opposite the motor's high position; the offset direction
  is a chassis-height vs floor-cutout trade for the layout iteration to
  resolve.

## 5. On-arrival checklist (clutch kit, then engine)

1. Caliper the pinion: OD + tooth count → module (spur order feeds off this)
2. Caliper the bore, stack length, rim axial exposure, rim true Ø
3. Check clamp-ring land: knurl profile, runout of the rim as mounted
4. Map the start-belt path and starter bracket against the vertical belt run
5. Confirm flywheel-face screw holes usable as ring bolt fallback
6. Spin-down inertia test once on the bench (feeds effective-mass model)

## 6. Purchases implied (NOT yet authorized — flag before ordering)

HTD 3M belt (~$6–9), 60T 3M pulley stock or printed, 3.175-bore conversion for
the motor pulley (~$5–10), belt-tension idler hardware if arrival geometry
demands it. Total well under $30; fold into the next combined order after the
arrival checks.
