# PROJECT AEGIS — Expanded AAA Systems & Multiplayer Design
## Adaptive AI • Nemesis Rivals • Procedural Forging • Reactive Arenas • Social Hub • 50v50 Territory War

> **Architecture note:** Combat is represented in the networking layer as deterministic, abstracted melee-ability events. Presentation can remain cinematic, while gameplay truth lives on the authoritative server.

---

## 1. North-Star Experience

AEGIS is a premium third-person melee-action game in which every encounter evolves around player expression, rival adaptation and a reactive world.

The expanded pillars are:

1. **Adaptive Opposition** — opponents learn a player's repeated tendencies without needing a cloud-trained model during a match.
2. **Persistent Rivalry** — elite opponents remember defining encounters and return with authored counters, changed tactics and visible history.
3. **Physical Crafting Fantasy** — forging is a tactile gameplay loop whose output is expressed through deterministic equipment stats and presentation.
4. **Reactive Arenas** — destruction and weather alter routes, visibility, footing and tactical opportunities without taking control away from combat readability.
5. **Social Scale** — the Home Forge expands into an optional persistent social hub and a territory-war meta layer.
6. **Sensory Feedback** — every major gameplay event has synchronized audio, camera and haptic responses.
7. **Authoritative Multiplayer** — the server owns rules, outcomes and persistence-sensitive state.

---

# 2. Runtime Architecture

```text
AegisGameInstance
├── Profile / Identity
├── Online Session Subsystems
├── Matchmaking Adapter
├── Telemetry / Analytics
└── Feature Flags

AegisCombat
├── Combat State Machine
├── Input Buffer
├── Ability / Attack Definitions
├── Hit Resolution
├── Poise / Stagger
└── Combat Event Bus

AegisAI
├── Combat Memory Component
├── Pattern Feature Extractor
├── Adaptive Policy Model
├── Nemesis Profile
├── StateTree / Behavior Tree Executor
└── EQS Tactical Queries

AegisCrafting
├── Forge Activity Director
├── Equipment Definition Assets
├── Balance Solver
├── Crafting Inventory
└── Cosmetic Wear / Presentation

AegisWorld
├── Weather Director
├── Surface State Manager
├── Destruction Manager
└── Arena Tactical State

AegisSocial
├── Home / Social Hub
├── Clan Service
├── Trading Service
├── Matchmaking
└── Territory War Service

AegisHaptics
├── Event → Haptic Profile
├── Platform Capability Adapter
├── Trigger Response
└── Accessibility Overrides
```

Design rule: no single module should own the complete experience. Systems publish typed events; listeners decide how to visualize, replicate or persist them.

---

# 3. Adaptive AI

## 3.1 Goal

AI should feel adaptive rather than clairvoyant. It learns from observable behavior, has a bounded memory window and retains only the statistics required to choose better counters.

## 3.2 Hybrid model

Use a **hybrid tactical architecture** rather than training a new neural network on every match:

```text
Perception / Combat Events
        ↓
Feature Extractor
        ↓
Rolling Behavior Window
        ↓
Pattern Statistics ──────→ Counter Library
        ↓                         ↓
NNE Inference (optional) → Utility Scoring
        ↓
StateTree / BT Action Selection
        ↓
Action Execution
        ↓
Outcome + Confidence Update
```

Unreal's Neural Network Engine can provide a common API for runtime model evaluation and can execute CPU/GPU-oriented inference through runtime interfaces; treat it as an optional decision augmentation rather than a dependency for baseline gameplay. citeturn341956search0turn341956search1

## 3.3 Feature vector

A compact feature vector can contain:

- recent ability categories and directions
- spacing bucket
- attack frequency
- average commitment duration
- defense preference
- dodge-side preference
- recovery punish frequency
- whiff frequency
- successful counter categories
- current stamina/poise bands
- weather/surface context
- team context in multiplayer

Do not store raw player input indefinitely. Keep a rolling window and aggregate statistics.

## 3.4 Adaptive memory

Recommended memory tiers:

**Immediate memory:** last 8–32 combat events.

**Encounter memory:** weighted statistics for the current fight.

**Rival memory:** a compact summary persisted to the Nemesis profile.

**Global archetype memory:** designer-authored priors shared by a class of opponents.

## 3.5 Counter selection

Every candidate response receives a utility score:

```text
Utility =
  CounterFit × 0.40
+ RangeFit × 0.15
+ TimingFit × 0.15
+ RiskAdjustedReward × 0.15
+ TerrainFit × 0.10
+ VarietyBonus × 0.05
```

The values are initial tuning targets, not fixed requirements. A controlled exploration term prevents the AI from becoming perfectly repetitive itself.

## 3.6 Fairness rules

The AI must never react to unavailable information.

Rules:

- no reading client-only hidden inputs
- no future-state knowledge
- minimum reaction delay
- counter confidence must decay when behavior changes
- difficulty modifies decision quality, not hidden information
- the same combat situation should remain debuggable from a recorded event stream

## 3.7 Blueprint conceptual structure

### `BP_AI_Director`

```text
Event BeginPlay
    ↓
Initialize Combat Brain
    ↓
Subscribe to Combat Event Bus
    ↓
Update Perception / Memory
    ↓
Evaluate Adaptation Interval
    ↓
Run Feature Extraction
    ↓
Pattern Analyzer
    ↓
Counter Candidate Generator
    ↓
Utility / NNE Inference
    ↓
StateTree Decision
    ↓
Execute Melee Ability / Reposition / Defend
    ↓
Record Outcome
```

### `BP_CombatMemoryComponent`

```text
RecentEvents : RingBuffer<FCombatEvent>
PatternStats : Map<EPatternId, FPatternStats>
CounterStats : Map<ECounterId, FCounterStats>
ConfidenceByPattern : Map<EPatternId, float>
```

### `BP_NemesisProfile`

```text
RivalId
DisplayName
EncounterCount
DefeatCount
SignaturePlayerPatterns
KnownCounterPatterns
PreferredArena
ScarVisualSeed
PersonalityPreset
EscalationTier
```

### StateTree structure

```text
Root
├── Dead
├── Recover
├── Evaluate Threat
│   ├── Immediate Counter
│   ├── Reposition
│   ├── Pressure
│   ├── Defend
│   └── Probe
└── Cooldown / Variety Check
```

StateTree/Behavior Tree remains responsible for execution flow, while the adaptive subsystem supplies context and weighted decisions. EQS can select tactical locations such as open lanes, cover-like landmarks or safer spacing positions. citeturn341956search10turn341956search13

---

# 4. Nemesis Rivalry System

## 4.1 Identity

A Nemesis is not a random respawn. It is a persistent gameplay record tied to an opponent archetype.

## 4.2 Rival lifecycle

```text
Opponent Created
      ↓
Encounter Recorded
      ↓
Player Defeats Rival
      ↓
Nemesis Snapshot
├── Defeat Context
├── Player Tendencies
├── Effective Counters
├── Arena Context
└── Cosmetic Scar Seed
      ↓
Rival Returns
      ↓
Counter Package Selected
      ↓
Encounter Played
      ↓
Profile Updated
```

## 4.3 Counter package

A counter package contains:

- three to five likely responses
- one signature response
- fallback behavior if the player adapts
- preferred engagement range
- preferred terrain situation
- difficulty-scaled confidence

## 4.4 Scar presentation

Use deterministic cosmetic seeds so the same rival preserves recognizable history across sessions without storing large rendered-state blobs.

---

# 5. Procedural Forging & Equipment Balancing

## 5.1 Forging fantasy

The blacksmithing activity should read as a physical craft simulation, while gameplay output is driven by a deterministic ruleset.

```text
Raw Material
   ↓
Heat Stage
   ↓
Shape Stage
   ↓
Balance Stage
   ↓
Finish Stage
   ↓
Equipment Stat Validation
   ↓
Crafted Equipment ID
```

## 5.2 Simulation boundary

Simulate visually rich interactions where they improve the fantasy, but convert the outcome into authoritative parameters at the end of each stage.

Example parameter families:

- balance bias
- handling class
- durability band
- guard stability
- recovery modifier
- cosmetic material set

The multiplayer server stores the resulting equipment definition, not every hammer-frame or particle position.

## 5.3 Balance solver

```text
Desired Handling
Desired Power
Material Budget
Craft Skill
       ↓
Constraint Solver
       ↓
Legal Stat Envelope
       ↓
EquipmentDefinition
```

The solver should clamp outputs to designer-authored envelopes so procedural crafting cannot create impossible combinations.

---

# 6. Tactical Destruction & Weather

## 6.1 Arena state model

```text
ArenaState
├── WeatherState
│   ├── Rain
│   ├── Mud
│   ├── Ice
│   └── Wind / Ash / Snow
├── SurfaceState Map
├── DestructionState
├── VisibilityState
└── HazardState
```

## 6.2 Weather director

`BP_WeatherDirector` owns the high-level weather state.

Weather changes must be:

- server-authored
- deterministic from a match seed + event timeline
- replicated as compact state
- interpreted locally by Niagara/audio/material systems

## 6.3 Surface modifiers

Each surface carries a data asset such as:

```text
SurfaceType
TraversalModifier
EvadeModifier
GripModifier
ElementalInteraction
VFXProfile
AudioProfile
```

The gameplay result is calculated server-side; the material/Niagara layer only visualizes the current state.

## 6.4 Destruction tiers

**Tier 0:** cosmetic response.

**Tier 1:** local VFX/audio/camera reaction.

**Tier 2:** small gameplay prop break.

**Tier 3:** tactical landmark transformation.

**Tier 4:** arena set-piece transition.

Keep expensive Chaos simulations sparse and event-driven. A 100-player war should never require hundreds of unconstrained rigid-body objects to run at full fidelity at once.

---

# 7. Social Hub

## 7.1 Home evolution

The single-player Home Forge becomes two modes:

**Private Home:** deterministic presentation stage, fast menus, no social replication burden.

**Social Fortress:** persistent shared space with bounded population and instanced activities.

## 7.2 Social hub topology

```text
World Gateway
├── Fortress Plaza
├── Clan Hall District
├── Forge District
├── Training Courtyard
├── Exhibition Walk
└── Duel Instances
```

Do not put 100-player combat into the same social simulation. Use social spaces for low-intensity interaction and transition into dedicated combat sessions.

## 7.3 Zero-loading-screen illusion

Use asynchronous destination preparation and camera handoff. The UX can feel continuous even though the underlying match may be a separate server/session.

---

# 8. Territory War — 50v50

## 8.1 Match definition

- 100 concurrent players
- two 50-player clans
- persistent territorial objective
- phased siege progression
- limited respawn/reinforcement rules
- match duration target: 20–40 minutes
- authoritative dedicated server

## 8.2 Phase model

```text
Staging
  ↓
Outer Objective
  ↓
Breach / Counter-Breach
  ↓
Inner Objective
  ↓
Final Control Window
  ↓
Territory Result
  ↓
Persistent World Update
```

## 8.3 Server-authoritative rules

Server owns:

- player positions as accepted simulation state
- ability state
- hit/contact results
- health/poise/stagger
- destruction ownership
- weather phase
- objective state
- score
- reinforcements
- territory result

Client predicts cosmetic anticipation and local presentation only.

---

# 9. Multiplayer Replication Strategy

For a 100-player battle, every client should not receive identical high-frequency state for every actor.

### Visibility bands

**Band A — combat-critical:** nearby enemies/allies in active engagement.

**Band B — tactical:** actors relevant to nearby objectives or threat assessment.

**Band C — strategic:** distant actors represented through coarser state.

**Band D — cosmetic:** low-frequency or event-only updates.

UE's current networking stack includes the generic replication system, Replication Graph and Iris. Epic documents Iris as a replication option designed to improve scalability through filtering, prioritization and more efficient shared work; however, the documentation currently labels Iris as experimental, so the production choice should be proven by load tests before committing a shipping architecture. citeturn287724search2turn287724search0turn287724search7

If Iris is selected, use connection filtering/prioritization aggressively so distant combat actors do not consume the same budget as the player's immediate threat set. citeturn287724search4

---

# 10. Server Infrastructure Requirements

## 10.1 Recommended reference architecture

```text
                Global Edge / API
                       │
              ┌────────┴─────────┐
              │ Match / Clan API │
              └────────┬─────────┘
                       │
                 Matchmaker
                       │
              ┌────────┴────────┐
              │ Territory Queue │
              └────────┬────────┘
                       │
          Dedicated 100-player Servers
        ┌──────────────┼──────────────┐
        │              │              │
   Region A       Region B       Region C
        │              │              │
   Game Servers    Game Servers    Game Servers
        │              │              │
   Telemetry / Logs / Metrics / Traces
                       │
            Persistent Match Result
                       │
        Profile / Clan / Territory DB
```

## 10.2 Hosting platform options

A managed game-hosting service can provision, place and scale dedicated servers. Amazon GameLift Servers currently supports managed EC2 and managed container fleets and exposes scaling and session-management capabilities for session-based multiplayer games. citeturn338874search3turn338874search5turn338874search6

A Kubernetes-native alternative is Agones, which provides Fleet/GameServer allocation and autoscaling primitives on Kubernetes; its current documentation describes FleetAutoscaler policies for maintaining warm capacity. citeturn338874search10turn338874search0

## 10.3 Per-match server baseline

Start benchmarking with:

- Linux dedicated server build
- 100 connected players
- 60 Hz authoritative simulation target for combat-critical logic
- adaptive network update rates by replication band
- server-side destruction budget
- server-side AI only for NPCs actually participating in the match
- structured telemetry for Game Thread, server frame time, network serialization, physics, AI and memory

Do **not** select an instance SKU by CPU core count alone. The final size should be determined by a representative 100-player load test containing the worst combination of replication, AI, destruction and weather activity.

## 10.4 Fleet capacity

Plan capacity around **ready servers**, not merely average concurrent players.

Example policy:

```text
Warm Capacity = Expected Concurrent Siege Matches × 1.25
Burst Capacity = Warm Capacity + Event Surge Buffer
```

For example, if a region expects 40 simultaneous 100-player sieges, warm for at least 50 server slots before a large event, then autoscale above that threshold.

GameLift supports managed fleet scaling, while Agones provides FleetAutoscaler policies and warm-server concepts that map cleanly to this model. citeturn338874search4turn338874search0

## 10.5 Network bandwidth

Measure actual packet rates from the prototype. A simple planning model is:

```text
Outbound ≈ Players × AverageReplicatedBitsPerSecond / 8
Inbound  ≈ Players × Input/CommandBitsPerSecond / 8
```

Add protocol overhead, bursts and retransmission headroom. Avoid designing to a theoretical maximum based on raw actor counts.

## 10.6 Regional placement

Place match servers close to the player population. Managed GameLift fleets support multiple geographic locations; cloud placement should be selected from measured latency, not from country-level assumptions alone. citeturn338874search7

## 10.7 Persistence services

Separate persistent state from the match server:

```text
Match Server
    ↓
Signed Match Result
    ↓
Match Result Service
    ├── Clan Ledger
    ├── Territory Service
    ├── Rewards Service
    └── Audit Log
```

The match server should not write directly to the primary persistent database for every combat event.

## 10.8 Reliability targets

Initial service-level goals:

- graceful server replacement on host failure
- idempotent match-result processing
- reconnect window for transient disconnects
- no duplicate territory rewards
- clan-war result auditability
- regional failover plan for matchmaking services

---

# 11. Haptic Feedback Architecture

## 11.1 Event model

```text
Gameplay Event
      ↓
Haptic Event Router
      ↓
Haptic Profile
      ↓
Platform Adapter
      ↓
Controller Output
```

## 11.2 Haptic profile

```text
HapticProfile
├── Intensity
├── Duration
├── LowFrequencyAmplitude
├── HighFrequencyAmplitude
├── TriggerResistance
├── TriggerStart
├── TriggerEnd
└── AccessibilityScale
```

## 11.3 Feedback matrix

| Gameplay event | Rumble | Trigger | Spatial bias | Camera |
|---|---:|---:|---|---:|
| light contact | low | none | contact-side | off |
| heavy contact | medium | light | contact-side | micro |
| perfect defense | high pulse | medium | centered/front | micro |
| guard break | high | strong | centered | medium |
| hard landing | medium | none | vertical | small |
| fatigue threshold | low periodic | progressive | centered | none |
| environment collapse | broad low-frequency | none | direction of event | medium |

Use capability checks and per-platform profiles. Haptics are never authoritative gameplay state.

---

# 12. Anti-Cheat & Competitive Integrity

100-player clan warfare requires server-side validation.

Validate:

- command sequence numbers
- command timing bounds
- movement envelopes
- ability-state legality
- cooldown/resource rules
- impossible event ordering
- inventory ownership
- territory authorization

Never trust client-supplied damage, score or reward values.

---

# 13. Telemetry

Record structured events such as:

```text
CombatEvent
AIAdaptationEvent
NemesisUpdate
CraftCompleted
WeatherChanged
DestructionEvent
ObjectiveCaptured
PlayerDisconnected
ServerFrameSpike
ReplicationBudgetExceeded
```

Every 50v50 session should produce a replay/audit summary without storing an unnecessarily large raw state stream.

---

# 14. Performance Gates

Before 50v50 alpha:

1. 100-player connection soak test.
2. 100-player combat stress test.
3. 100-player destruction stress test.
4. weather-transition stress test.
5. AI adaptation stress test.
6. packet-loss and latency simulation.
7. reconnect test.
8. server crash/replacement test.
9. match-result idempotency test.
10. territory double-award prevention test.

A feature is not multiplayer-ready when it merely works in PIE with eight clients; it is ready when its worst-case server frame, bandwidth and recovery behavior have been measured.

---

# 15. Production Roadmap

### Milestone A — Adaptive Combat Prototype
- event-driven AI memory
- counter library
- first NNE experiment
- StateTree integration
- deterministic replay of decisions

### Milestone B — Nemesis Slice
- persistent rival profile
- counter package selection
- cosmetic history
- rematch flow

### Milestone C — Forge & Reactive Arena
- forging activity prototype
- deterministic equipment solver
- weather state machine
- destruction tiers

### Milestone D — Social Fortress
- shared hub
- clan identity
- trading service
- instanced duels

### Milestone E — 50v50 Alpha
- dedicated server
- matchmaking
- replication budget
- objective system
- territory result service
- observability stack

### Milestone F — Siege Beta
- regional fleet scaling
- reconnects
- anti-cheat validation
- soak testing
- disaster recovery

---

# 16. Architectural Invariants

1. Gameplay truth lives on the authoritative simulation.
2. AI adapts only from information available to the character.
3. Procedural crafting outputs deterministic, bounded results.
4. Weather and destruction replicate as compact state, not raw simulation history.
5. Social hubs are separated from high-intensity battle servers.
6. Persistent services consume signed match outcomes instead of transient combat spam.
7. Haptics, camera, Niagara and audio subscribe to gameplay events; they do not define them.
8. Every competitive system must be replayable and diagnosable from structured telemetry.
