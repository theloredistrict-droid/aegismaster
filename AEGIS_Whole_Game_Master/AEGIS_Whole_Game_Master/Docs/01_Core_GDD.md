# Project AEGIS: Ultimate 3D Sword Fighting Game
## UE5 AAA Technical Design & Core Architecture

### 1. Product Vision
Project AEGIS is a premium third-person melee combat game built around precise, readable swordplay inside physically reactive arenas. The target experience is: cinematic presentation when the camera is allowed to breathe, but deterministic and low-latency combat when a player commits to an attack, parry, dodge, or counter.

Design pillars:
- **Weapon truth:** weapon mass, reach, recovery, and edge alignment matter.
- **Readable chaos:** destruction and VFX add consequence without hiding combat information.
- **Animation authority:** authored animation provides intention; procedural systems preserve contact, footing, and spatial accuracy.
- **Data-driven content:** weapons, armor, hit reactions, VFX, AI and tuning are assets, not hard-coded branches.
- **Presentation/gameplay separation:** the Home forge and cinematic systems can be rich without coupling themselves to combat simulation.

---

## 2. High-Level UE5 Architecture

### Runtime layers
1. **Core Game Module**
   - GameInstance / Subsystems
   - Save/Profile
   - Asset streaming orchestration
   - Global telemetry and feature flags
2. **Combat Simulation**
   - `UCombatStateMachineComponent`
   - Input buffer
   - Attack definitions
   - Guard/parry windows
   - Hit validation and damage
   - Stagger/poise
3. **Character**
   - Character movement
   - Motion Warping
   - Control Rig
   - Animation Montages / Linked Anim Layers
   - Foot planting and turn-in-place
4. **Weapon & Armory**
   - Weapon actor/component
   - Data assets
   - Collision profiles / trace sockets
   - Material variants
   - Procedural damage masks
5. **Physical World**
   - Chaos rigid bodies
   - Geometry Collection micro-destruction
   - Breakable props
   - Surface reaction data
6. **Presentation**
   - Niagara
   - Lumen lighting/reflection response
   - Level Sequences
   - Audio
   - Camera system
7. **UI**
   - CommonUI/UMG shell
   - HUD widgets
   - Home forge overlay
   - Menus / armory / matchmaking

### Recommended module split
- `AegisCore`
- `AegisCombat`
- `AegisCharacter`
- `AegisWeapons`
- `AegisWorld`
- `AegisPresentation`
- `AegisUI`
- `AegisAI`

The module boundaries are intentional: combat should not depend directly on Niagara, UMG, or editor-only code.

---

# 3. Next-Gen Visual Pipeline

## Nanite
Use Nanite for:
- weapon bodies and ornate guards
- armor plates, buckles, scabbards, props
- architectural kits and statues
- dense battleground debris
- hero environmental assets

Rules:
- Author hero meshes at film-quality geometric density.
- Keep gameplay collision meshes separate from render geometry.
- Use explicit simplified collision for sword/body interaction; never depend on Nanite triangles as combat collision.
- Use virtual textures/material instances for scalable surface detail.

## Lumen
Lighting stack:
- Lumen GI/reflections for dynamic combat spaces.
- Practical key/fill/rim lights for characters where art direction requires it.
- Niagara light-emitting sparks during clashes, but rate-limit dynamic light creation.
- Expose an arena lighting profile as a data asset so each arena can tune exposure, reflection intensity and weather.

## MetaHuman / character presentation
- MetaHuman-derived base character or equivalent high-fidelity custom character.
- Facial system isolated from gameplay state machine.
- Facial/eye focus can react to opponent direction without changing combat hit logic.
- Character LOD/presentation settings should be profile driven for Performance / Quality / Cinematic modes.

## Niagara
Systems:
- weapon trails
- blade contact sparks
- guard/parry burst
- blood/dust equivalents where permitted by rating
- rain/snow/ash
- embers from the forge
- environment hit reactions
- armor damage debris

Every VFX system should have a gameplay tag + surface tag interface, e.g. `FX.Parry.Metal`, `FX.Impact.Stone`.

---

# 4. Combat Design

## Combat loop
**Read → Commit → Contact → Resolve → Recover → Reposition**

Player inputs are interpreted as intentions, not direct animations. The combat layer selects the attack/defense asset that best satisfies the current state, weapon, direction, stance, and movement context.

## Directional striking
Direction comes from a compact 4- or 8-way intent enum:
- High / Low
- Left / Right
- Forward / Back
- Optional diagonal extensions

The attack data asset determines:
- startup frames
- active frames
- recovery frames
- reach
- arc angle
- hit strength
- poise damage
- stamina cost
- guard damage
- movement displacement
- motion-warp target policy

## Frame-perfect parry model
Do not hard-code parry timing in the character class.

Instead:
- animation montages publish a `ParryWindowStart` and `ParryWindowEnd` notify state
- the state machine stores server/local simulation time for the active window
- incoming hit evaluation checks the defender's parry window against the attack contact timestamp
- perfect parry, normal guard and failed guard are distinct resolution outcomes

For network play, the server remains authoritative over final contact resolution. Client prediction may immediately display anticipation VFX/animation, followed by correction if required.

## Input buffer
Recommended:
- 120–250 ms buffer depending on action class
- attack queue length 1–2
- priority rules for emergency defense
- explicit cancel rules

Suggested priority:
1. death/knockdown
2. perfect parry
3. dodge/evade
4. hit-stun/stagger resolution
5. attack transition
6. locomotion

## Weight, poise and stagger
Each weapon and armor set contributes to:
- impact force
- poise damage
- recovery scaling
- movement friction
- guard stability

A lightweight blade may attack faster while a great blade creates larger hit reactions and environmental force.

A simple poise model:
`PoiseAfterHit = CurrentPoise - (ImpactPoise * HitMultiplier) + RecoveryPerSecond * DeltaSeconds`

Thresholds can trigger:
- flinch
- stagger
- heavy stagger
- guard break
- knockdown

---

# 5. Hit Detection & Weapon Physics

## Recommended trace strategy
Use authored socket pairs along the blade:
- `Blade_Base`
- `Blade_Mid`
- `Blade_Tip`

Each simulation update:
1. read previous socket positions
2. read current socket positions
3. sweep between them
4. accumulate unique hit actors/components
5. resolve surface material + hit zone
6. emit combat event

This provides much more reliable fast-sword collision than a single box overlap.

## Weapon physics profiles
Each weapon has:
- mass class
- center of mass offset
- inertia profile
- blade length
- grip length
- edge alignment
- trace thickness
- impact impulse
- parry resistance
- durability

Physics is primarily used for secondary effects and authored reaction, while final combat authority remains deterministic.

---

# 6. Chaos Physics & Destruction

Use Chaos for:
- heavy strike environmental impacts
- breakable props
- debris and fragments
- secondary ragdoll behavior where appropriate
- reactive armor pieces

Do not make every combat interaction fully rigid-body simulated; that would trade responsiveness for uncontrolled solver cost.

Recommended destruction tiers:
- Tier 0: decal/material-only reaction
- Tier 1: Niagara + audio + camera impulse
- Tier 2: fracture a small prop
- Tier 3: local Geometry Collection destruction
- Tier 4: hero scripted destruction / set-piece

---

# 7. Armory & Customization

### Weapon data asset
`UWeaponDefinition` contains:
- presentation mesh
- collision sockets
- combat profile
- animation set
- damage profile
- poise profile
- physics profile
- surface/audio tags
- Niagara references
- upgrade sockets

### Armor data asset
`UArmorSetDefinition` contains:
- skeletal mesh / modular pieces
- material instances
- stat modifiers
- cloth settings
- damage zones
- reactive FX
- wear/damage layers

### Damage appearance
Keep gameplay damage and material appearance separate:
- gameplay tracks durability/damage state
- appearance system converts that state into material parameters / masks
- optionally use runtime virtual textures or render-target masks for localized wear

The system should support a deterministic seed so the same damage event can reproduce the same visual result across clients.

---

# 8. Home Tab: Living 3D Forge

## Scene concept
A monumental ancient forge acts as the interactive Home environment. The player's equipped fighter occupies the center of the stage and performs a low-frequency idle/practice loop. Ember particles drift through volumetric space, rain or ash reacts to the weather profile, and metal surfaces respond to the forge lighting.

## World structure
- `L_HomeForge` persistent scene
- `BP_HomeForgeDirector`
- `BP_HomeCharacterStage`
- `BP_ForgeInteractionPoints`
- `LS_Home_Idle` sequence
- `WBP_HomeOverlay`
- `WBP_HomeNavigation`

## UMG hierarchy
```text
WBP_HomeRoot
├── SafeZone
│   ├── WBP_TopBar
│   │   ├── ProfileCluster
│   │   ├── CurrencyCluster
│   │   └── SettingsButton
│   ├── WBP_HomeNavigation
│   │   ├── PlayButton
│   │   ├── ArmoryButton
│   │   ├── FightersButton
│   │   ├── ArenaButton
│   │   └── OptionsButton
│   └── WBP_ContextPrompt
├── WBP_WeaponPreview
└── WBP_FadeTransition
```

The 3D world is not rendered inside the widget. The camera renders the world; UMG provides the minimal overlay and input routing.

## Home Tab interaction model
- hover/focus a button → subtle camera parallax + UI highlight
- select Armory → camera starts orbit transition toward the weapon rack
- select Play → `HomeForgeDirector` requests arena warm-up
- when the arena is ready, a Level Sequence/Camera system moves from the forge composition into the gameplay spawn composition

## Zero-loading-screen strategy
A truly zero-loading experience requires assets to be available before the visual handoff. Recommended strategy:
- keep the base gameplay framework loaded
- asynchronously stream the chosen arena and its dependent assets while the player is still in Home
- prewarm shaders, Niagara systems, materials and key skeletal meshes
- keep the destination camera composition hidden until ready
- gate the transition on an `ArenaReady` signal

The transition should never begin because a button was clicked; it begins because the destination world is confirmed ready.

---

# 9. Animation Architecture

### Layers
1. locomotion base pose
2. upper-body combat layer
3. additive weapon recoil/weight
4. hit reaction layer
5. facial layer
6. procedural foot/hand correction

### Motion Warping
Use Motion Warping for:
- lunging strikes
- gap closing
- execution alignment
- ripostes
- ledge/position correction

### Control Rig
Use Control Rig for:
- blade-to-target alignment
- wrist correction
- shoulder compensation
- foot planting
- stance stabilization

Animation Notify States are the canonical way to open and close gameplay timing windows.

---

# 10. Camera

Combat camera requirements:
- target-relative framing
- collision-safe spring arm
- attack-dependent subtle FOV impulse
- parry and impact micro-shake, capped for readability
- cinematic overrides only when gameplay is paused or explicitly staged

Create separate camera modes:
- `Combat`
- `LockOn`
- `Execution`
- `ForgeHome`
- `Transition`

---

# 11. Audio

Data-driven audio surfaces:
- weapon vs weapon
- weapon vs armor
- weapon vs stone
- weapon vs wood
- near miss
- parry
- stagger
- guard break

Audio should be synchronized from the same combat events that drive VFX, not from UI or animation alone.

---

# 12. Performance Budgets

Target a scalable architecture rather than a single fixed hardware tier.

Per-frame budgets should be profiled for:
- Game Thread
- Render Thread
- GPU
- Niagara
- Chaos
- Animation
- UI
- streaming

Core principle: hero detail may be unlimited in source assets, but runtime cost is budgeted per arena and per combat encounter.

Add a telemetry overlay for development builds with:
- frame time breakdown
- active Niagara systems
- Chaos island count
- animation update cost
- streaming state
- current combat state
- active hit traces

---

# 13. Save/Profile System

Save:
- unlocked fighters
- weapons
- armor
- customization
- tuning progression
- settings
- last home loadout

Use stable IDs (`FGuid` or gameplay-defined asset IDs) rather than actor names.

---

# 14. Multiplayer-Ready Design

Even for a primarily single-player title, combat events should be structured as authoritative gameplay messages.

Server-authoritative:
- final hit confirmation
- damage
- parry outcome
- stagger
- death
- destruction ownership

Client-predictable:
- local input acceptance
- anticipation montage
- non-authoritative cosmetic VFX
- UI feedback

Use replicated compact state, not replicated raw animation transforms wherever possible.

---

# 15. Production Roadmap

### Phase 1 — Combat blockout
- one fighter
- one sword
- one dummy
- directional attacks
- parry window
- stagger
- camera

### Phase 2 — Vertical slice
- polished arena
- one elite enemy
- Lumen/Nanite final pass
- Niagara combat VFX
- first armory pass
- living forge Home Tab

### Phase 3 — Content scale
- multiple weapon archetypes
- armor sets
- environmental destruction
- AI behavior trees/state trees
- weather variants

### Phase 4 — AAA polish
- cinematic sequences
- advanced facial animation
- audio mixing
- accessibility
- performance profiles
- QA telemetry
- platform certification

---

# 16. Core Development Rule
Combat must remain authoritative, deterministic, inspectable and testable. Rendering, VFX, audio and UI subscribe to combat events; they do not define gameplay truth.

This separation is what allows the project to push Nanite/Lumen/Niagara/MetaHuman quality without turning the combat layer into an untestable cinematic script.
