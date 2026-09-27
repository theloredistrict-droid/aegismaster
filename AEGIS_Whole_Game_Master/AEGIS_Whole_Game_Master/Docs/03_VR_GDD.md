# AEGIS VR — Immersive Physics Sword-Fighting Game Design Document

## 1. Vision

AEGIS VR is the VR translation of the AEGIS sword-fighting universe into a first-person, physically authored melee experience. The defining rule is simple: **the player's tracked motion is the weapon input**. The game does not ask the player to press a button to perform a sword animation; it asks the player to physically move, control, recover, and manage a weapon whose contact is resolved by Unreal Engine 5 Chaos physics.

The VR version preserves the existing AEGIS pillars—high-end visuals, deep armory, adaptive rivals, social fortress, territory warfare, dynamic arenas—but changes the interaction model so that world objects, weapons, crafting stations and UI are all designed around physical presence.

### Experience pillars

1. **1:1 Combat Authority** — controller/hand tracking generates weapon intent; Chaos determines physical outcome.
2. **Contact Has Consequences** — blades can hit, glance, bind, deflect, scrape, or stop; no phantom penetration.
3. **Embodied Progression** — the player physically uses and carries their gear.
4. **Diegetic Everything** — health, stamina, weapon data, quests and modes are represented in the world.
5. **Comfort Without Losing Physicality** — locomotion and camera systems reduce motion discomfort while preserving physical combat.
6. **Multiplayer-Ready Physics** — local low-latency combat remains responsive while authoritative outcomes are replicated by the server.

---

## 2. Player Loop

### Moment-to-moment loop

Observe → approach → guard/stance → swing/thrust → contact → recover → reposition → exploit opening.

### Session loop

Fortress → armory → physical loadout → war map → activity → rewards/materials → forge/craft → return to fortress.

### Mastery loop

Players improve through their own physical technique: distance control, edge alignment, recovery, parry timing, stamina management, body positioning and weapon handling.

---

## 3. VR Hardware Abstraction

Create `IVRTrackingProvider` as the only gameplay-facing interface for XR devices. OpenXR pose/action data is normalized to a common representation:

```text
HMD Pose
Left Hand Pose
Right Hand Pose
Grip Value
Index/Trigger Value
Thumbstick / Thumbpad
Buttons
Tracking Confidence
Device Capabilities
```

The capability layer exposes haptic channels rather than device-specific gameplay code:

```text
ImpactImpulse
ImpactSharpness
Resistance
ScrapeTexture
BindPressure
FatiguePulse
DashConfirmation
EnvironmentalPulse
```

PSVR2, SteamVR/OpenXR controllers and hand-tracking devices map into this interface.

---

## 4. Combat Architecture

```text
XR/OpenXR Pose
      ↓
Tracked Hand Target Pose
      ↓
Physical Hand Proxy (Chaos)
      ↓
Weapon Physics Body + Constraint
      ↓
Continuous Collision Detection / Sweeps
      ↓
Contact Resolver
      ├─ Hit
      ├─ Deflect
      ├─ Bind
      ├─ Scrape
      └─ Miss / Environment Impact
      ↓
Combat Events
      ├─ Damage / Poise
      ├─ Haptics
      ├─ Niagara
      ├─ Audio
      └─ Camera-free body feedback
```

The player may move the real controller freely, but the **simulation proxy** can encounter the environment. The renderer remains visually stable while the physical object applies correction force and produces matching haptic resistance.

### Core design rule

Do not directly teleport a collidable sword into the controller pose. Instead:

- XR pose = desired target.
- Chaos body = physical simulated state.
- A high-bandwidth spring/constraint/motor follows the target.
- Continuous collision detection prevents tunneling.
- Maximum force/torque prevents the weapon from passing through solid geometry.
- Sub-stepped physics owns contact resolution.

This gives 1:1 intent with physically valid outcomes.

---

## 5. Weapon Physics Model

Each weapon contains:

- Skeletal/static mesh representation.
- Physics asset with tuned mass/inertia.
- Grip transform.
- Blade base/mid/tip sockets.
- Primary cutting edge metadata.
- Secondary blunt zones.
- Damage and poise curves.
- Material friction/restitution.
- Bind eligibility.
- Trace thickness.
- Haptic profile.

### Weight model

Weapon handling is determined by real simulation properties, not an animation montage:

```text
EffectiveHandling = Mass × InertiaTensor × GripOffset × PlayerInputGain
```

Changing balance point therefore changes acceleration, stop distance, torque and perceived weight.

### Edge alignment

The blade's local edge axis is compared against relative velocity and impact normal. Edge-on hits produce different damage/contact behavior from flat or glancing impacts.

---

## 6. Collision and Contact Resolution

### Continuous detection

Use Chaos CCD for the main weapon body and a secondary swept segment test between blade socket positions each physics substep. The secondary test is a gameplay safety net for extremely fast motion and narrow targets.

### Contact categories

| Contact | Result |
|---|---|
| Blade ↔ Blade, high relative speed | Parry / deflection / possible bind |
| Blade ↔ Blade, low tangential speed | Bind |
| Blade ↔ Armor | Damage + scrape/deflect depending on normal |
| Blade ↔ Soft target | Damage event + limited penetration response |
| Blade ↔ Stone/Metal | Sparks, impact impulse, environment damage |
| Heavy strike ↔ Breakable prop | Chaos fracture / impulse event |

### Contact calculation

For contact points `A` and `B`:

```text
RelativeVelocity = VelocityAAtPoint - VelocityBAtPoint
NormalSpeed      = dot(RelativeVelocity, ContactNormal)
TangentialSpeed  = length(RelativeVelocity - NormalSpeed * ContactNormal)
ImpulseEnergy    = 0.5 * ReducedMass * NormalSpeed²
```

These values feed damage, deflection, haptics, sound and Niagara intensity.

---

## 7. Blade Binding System

A bind is a **temporary physical constraint** between two weapon bodies.

### Enter bind when

- Two eligible blade contact zones overlap.
- Relative normal speed is below the hard-impact threshold.
- Tangential relative speed is within the bind envelope.
- Contact angle is within the authored bind cone.
- Neither weapon is in a non-bindable state.

### Bind behavior

Create or activate a Chaos 6DOF-style constraint at the contact point.

Locking is staged instead of binary:

```text
Contact → Soft Bind → Firm Bind → Slip / Break
```

The constraint limits relative motion while preserving enough freedom for the player to physically work the opposing blade away.

### Bind escape

The bind can end through:

- Player separation impulse.
- Twist threshold.
- Edge-roll maneuver.
- Opponent withdrawal.
- Stamina/poise failure.
- Maximum bind lifetime.

All thresholds are data-driven.

---

## 8. Physical Parry

A parry is generated by the physics contact itself rather than a button-timed animation.

A successful parry requires:

- Defender weapon positioned within the authored parry surface.
- Correct contact side.
- Relative velocity above the minimum energy threshold.
- Defender blade orientation within tolerance.
- Timing derived from the actual contact timestamp.

Parry outcome:

```text
Impact → Deflection impulse → attacker poise event → haptic shock → audio/FX
```

Optional assist modes may enlarge the effective parry surface for comfort or accessibility without turning the weapon into a magnet.

---

## 9. Haptic Feedback Design

Haptics are generated from physical measurements rather than hand-authored “hit happened” signals.

| Event | Main signal | Feel |
|---|---|---|
| Light swing | Low-amplitude texture | Air/weapon motion cue |
| Heavy swing | Resistance ramp + low pulse | Mass and momentum |
| Blade impact | Sharp impulse | Distinct clack/shock |
| Hard parry | Sharp bilateral impulse | Strong snap |
| Blade bind | Sustained low oscillation | Metal grinding / pressure |
| Scrape | Grainy high-frequency modulation | Edge dragging across material |
| Heavy armor impact | Deep pulse + short resistance | Dense impact |
| Fatigue | Periodic low amplitude modulation | Character exertion |
| Dash | Brief confirmation pulse | Movement confirmation |
| Environment hazard | Contextual pulse | Weather/terrain feedback |

The haptic subsystem consumes normalized parameters:

```text
Amplitude 0..1
Frequency 0..1
Sharpness 0..1
Duration ms
TriggerResistance 0..1
TextureId
```

### Adaptive trigger logic

Trigger resistance rises with:

- weapon inertia during active acceleration,
- bind pressure,
- environmental drag,
- fatigue state.

Never use resistance values that can physically prevent a safe release.

---

## 10. Physical Locomotion

### Base locomotion

- Left stick: movement relative to HMD forward or body forward.
- Right stick: smooth or snap rotation.
- Head-relative movement uses a filtered planar heading.
- Acceleration and deceleration are applied through a kinematic capsule rather than direct position teleporting.

### Dash dodge

Dash is a physical burst movement, not a canned animation.

Inputs:

```text
DashDirection = normalized(StickDirection or BodyIntent)
DashSpeed     = BaseDashSpeed × StaminaMultiplier
DashTime      = AuthoredDuration
```

The capsule is swept against world collision throughout the dash. A dash cannot phase through walls.

### Physical dodge integration

When the player moves their torso/head significantly relative to their root, the locomotion system retains their real-world pose while the virtual root provides only the minimum correction needed to remain inside the playspace boundary.

### Comfort modes

- Smooth movement.
- Snap turning.
- Vignette during acceleration.
- Reduced dash speed.
- Reduced camera-relative sway.
- Optional seated/standing calibration.
- Height recalibration.

---

## 11. Full-Body IK

Use a layered body solution:

1. HMD target.
2. Hand targets.
3. Pelvis solver.
4. Two-bone / FABRIK-style leg solving.
5. Foot placement against terrain.
6. Spine/chest stabilization.
7. Secondary armor/accessory stabilization.

Recommended responsibility split:

- Animation Blueprint: state/control flow.
- Control Rig: final pose solve.
- IK Rig / Retargeter: reusable retargeting.
- Physics Asset: physical proxies and contacts.

### Foot placement

Perform downward traces from predicted foot positions, derive surface normal, then orient the foot while preserving ankle/hip limits.

---

## 12. Armory as a Physical Space

The fortress armory is a persistent interactive room.

### Equipment flow

1. Walk to weapon rack.
2. Reach with tracked hand.
3. Grab physical weapon handle.
4. Pull weapon from rack.
5. Inspect blade and etched runes.
6. Attach to hip/back/boot socket.
7. Holster confirmation occurs through actual overlap and alignment.

### Safety behavior

A weapon cannot be equipped merely by pointing at it. The interaction requires a valid physical grab and holster pose.

---

## 13. Physical Forging

The crafting loop is designed around simple, repeatable physical interactions:

```text
Heat metal → Hammer → Shape → Quench → Grind → Wrap → Inscribe → Test
```

Physics interactions are limited to meaningful tool affordances. Do not simulate every atom or hammer collision. Instead, use authored deformation targets driven by physical tool velocity, contact location and strike energy.

### Weapon balancing

Crafting parameters directly alter simulation:

- Center of gravity.
- Blade length.
- Blade mass.
- Crossguard width.
- Grip length.
- Hilt material.
- Pommel mass.
- Edge angle.

These feed the weapon's physical asset and gameplay data before the weapon is spawned.

---

## 14. Reactive Runes

Runes are physical/diegetic metadata displays.

The blade can show:

- damage profile,
- weight class,
- center-of-mass location,
- stamina efficiency,
- parry response,
- elemental/arena interactions.

Rune animation is Niagara-driven and reacts to inspection distance, blade heat and recent combat history.

---

## 15. Diegetic Home Fortress

The traditional Home screen becomes a persistent physical location.

### War Map

A giant map table displays:

- Ranked Arena.
- Endless Tower.
- Training Hall.
- Social Fortress.
- Territory War sectors.

The player walks to the location and physically touches a map region or pulls a physical marker.

### Diegetic telemetry

Health/stamina are represented by:

- gauntlet crystals,
- armor strips,
- small emissive plates.

Weapon stats appear as runic overlays on the blade when the player brings it into inspection distance.

No blocking full-screen menus are required for the primary game loop.

---

## 16. Visual Direction

### Nanite

Use Nanite for forge architecture, armor, weapons, stonework and dense environment kits where compatible with VR platform budgets.

### Lumen

Because VR frame budgets are substantially tighter than flat-screen AAA targets, use a platform-specific lighting strategy. Full dynamic Lumen should be treated as a quality tier rather than a universal requirement; baked/precomputed or simplified dynamic lighting should exist for constrained headsets.

### Niagara

Prioritize:

- sparks,
- blade trails,
- dust bursts,
- forge heat,
- rain/snow particles,
- rune emissions.

Every high-frequency VFX system needs scalability controls.

---

## 17. Multiplayer VR Architecture

The VR client owns:

- raw XR tracking,
- local IK,
- local haptics,
- presentation.

The authoritative server owns:

- player root movement authority,
- weapon simulation authority,
- damage,
- parries,
- binds,
- arena destruction,
- stamina/health truth.

Use client-side prediction for player locomotion and local presentation smoothing for the hand/weapon. Reconcile to server-approved transforms and contact outcomes.

For multiplayer blade contact, replicate compact contact events rather than every Chaos solver value:

```text
ContactId
ServerTimestamp
WeaponA
WeaponB
ContactPoint
Normal
ContactType
ImpulseMagnitude
ResultFlags
```

---

## 18. Performance Targets

### PCVR target

- 90 FPS baseline.
- 120 FPS mode on capable hardware.
- Physics simulation sub-stepping tuned to keep contact stable at high swing speeds.
- Aggressive Niagara scalability.
- Dynamic resolution and foveated rendering where supported.

### Standalone / constrained target

Provide a separate rendering profile with reduced geometry/VFX complexity and simplified lighting while preserving the same physics and combat rules.

### Frame-time doctrine

Combat responsiveness has priority over cosmetic fidelity. When overloaded:

```text
Preserve → input → physics contacts → locomotion → readability
Reduce   → particles → secondary animation → distant geometry → expensive lighting
```

---

## 19. VR Safety and Comfort

The player should always be able to:

- pause or open a comfort panel safely,
- reduce locomotion intensity,
- disable or soften trigger resistance,
- adjust reach/height calibration,
- switch between snap and smooth turning,
- use seated or standing profiles.

Avoid gameplay systems that require extreme real-world swings or dangerous body movement.

---

## 20. Production Phases

### Prototype P0 — Physics proof

One room, one sword, two hands, one opponent, one destructible pillar.

Acceptance:

- No obvious blade tunneling at target swing speed.
- Blade contact produces stable deflection.
- Bind can form and break.
- Haptic event timing follows contact.

### P1 — Combat sandbox

Multiple weapons, stamina, armor hit zones, parry, AI sparring, locomotion comfort settings.

### P2 — Fortress / Armory

Physical equipment, forging station, war map, diegetic telemetry.

### P3 — Social and multiplayer

Networked 1v1, then small arena tests, then large-scale Territory War proof-of-concept.

### P4 — Visual scale-up

Nanite/Lumen/MetaHuman/Niagara passes after physics and frame-time budgets are stable.

---

## 21. Technical Acceptance Checklist

Combat:
- 1:1 tracked weapon intent.
- CCD + secondary segment sweep.
- Physically resolved contact.
- Deflection and bind states.
- Data-driven weapon mass/inertia.

VR:
- Calibration.
- Snap/smooth turn.
- Comfort locomotion.
- Full-body IK.
- Haptic abstraction.

World:
- Physical armory.
- Forging interactions.
- Diegetic war map.
- Diegetic player telemetry.

Networking:
- Server-authoritative combat outcomes.
- Compact contact replication.
- Prediction/reconciliation.

---

## 22. Recommended Core Module Layout

```text
Source/AegisVR/
├── Combat/
│   ├── VRCombatTypes.h
│   ├── VRPhysicsWeaponComponent.h
│   ├── VRWeaponBindingComponent.h
│   └── VRCombatContactResolver.h
├── Locomotion/
│   └── VRLocomotionComponent.h
├── XR/
│   ├── VRTrackingProvider.h
│   └── VRHapticsRouter.h
├── Interaction/
│   ├── VRGrabComponent.h
│   └── VRHolsterComponent.h
├── Crafting/
│   └── VRForgeStationComponent.h
├── Body/
│   └── VRFullBodyIKComponent.h
└── UI/
    └── DiegeticHUDComponent.h
```

The supplied headers in this package provide the first implementation seam for the most difficult portion: **tracked weapon intent + continuous collision + bind lifecycle**.
