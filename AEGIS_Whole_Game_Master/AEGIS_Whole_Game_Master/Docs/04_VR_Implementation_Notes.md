# AEGIS VR — Implementation Notes

## 1. Physics sub-stepping

Drive `UVRPhysicsWeaponComponent::StepPhysics()` from a physics-safe callback or a custom physics scene step rather than ordinary frame `Tick()` when deterministic contact behavior matters.

Recommended sequence:

```text
XR sample
   ↓
Pose prediction/filter
   ↓
Pre-physics target update
   ↓
Chaos sub-step
   ├─ hand/weapon drive
   ├─ CCD
   ├─ contact generation
   └─ constraints
   ↓
Contact event extraction
   ↓
Post-physics gameplay events
   ↓
Haptics / audio / Niagara
```

## 2. Do not teleport colliders

A common VR shortcut is:

```cpp
Weapon->SetWorldTransform(TrackedTransform);
```

That defeats the physical premise. The production implementation should instead drive a physics body toward the tracked target using a stiff motor/constraint and allow Chaos to resolve obstacles.

## 3. Contact channel strategy

Use dedicated object channels for:

- VR weapon.
- VR body proxy.
- Enemy weapon.
- Armor.
- Breakable environment.
- Non-combat world.

Not every collision should become a gameplay hit. Filter contacts before expensive combat processing.

## 4. Blade segment sweep

In addition to rigid-body contact, sample blade sockets each physics step:

```text
PreviousBase → CurrentBase
PreviousMid  → CurrentMid
PreviousTip  → CurrentTip
```

Run swept line/capsule tests for each segment. This catches high-speed edge crossings and supplies a stable contact point even when a rigid-body pair generates a marginal contact.

## 5. Bind constraint ownership

The bind component creates a temporary constraint object owned by the encounter subsystem. It should not permanently attach one sword actor to another.

Constraint teardown must happen when:

- separation > break threshold,
- twist > authored tolerance,
- bind timer expires,
- either weapon changes state,
- actor becomes invalid.

## 6. Network authority

On the client:

- capture XR poses locally,
- simulate local hand/weapon presentation,
- emit compact input/pose packets.

On the server:

- validate locomotion envelope,
- own combat result,
- own final contact classification,
- replicate contact outcome + authoritative weapon correction.

Do not replicate raw per-shape Chaos internals every frame.

## 7. VR frame budgeting

Keep the following off the critical render path:

- crafting persistence writes,
- AI telemetry aggregation,
- cosmetic unlock checks,
- nonessential Niagara spawning,
- analytics batching.

Combat input and collision response remain highest priority.
