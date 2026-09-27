# AEGIS Territory War — 50v50 Server Architecture

## Match Contract

- 100 players per siege instance.
- Dedicated authoritative server.
- 60 Hz combat-critical simulation target; tune replication rates separately.
- Region-aware matchmaking.
- Warm-server capacity plus event burst buffer.

## Logical Services

```text
Client
  |
  +--> Auth / Session
  |
  +--> Matchmaking
  |       |
  |       +--> Clan Eligibility
  |       +--> Latency / Region Selection
  |       +--> Siege Allocation
  |
  +--> Dedicated Siege Server
          |
          +--> Combat Authority
          +--> Objective Authority
          +--> Weather State
          +--> Destruction State
          +--> Replication
          +--> Anti-Cheat Validation
          +--> Telemetry
          |
          +--> Match Result Service
                  |
                  +--> Territory Ledger
                  +--> Clan Ledger
                  +--> Rewards
                  +--> Audit Log
```

## Replication Bands

```text
Band A: immediate combat relevance
Band B: local tactical relevance
Band C: strategic relevance
Band D: cosmetic/event state
```

Use filtering and prioritization so each connection receives only the information needed at the appropriate rate.

## Capacity Formula

```text
Warm Servers >= Expected Concurrent Matches × 1.25
Burst Servers = Warm Servers + Event Surge Buffer
```

Treat 1.25 as a starting policy, not a universal constant.

## Load Test Scenarios

1. 100 connected players with normal movement.
2. 100 players concentrated in one objective area.
3. peak combat effects.
4. peak destruction state.
5. weather transition.
6. 20–30% packet loss simulation.
7. elevated RTT.
8. reconnect storm.
9. server replacement.
10. duplicate-result submission.

## Platform Choices

### Managed game hosting
Amazon GameLift Servers provides managed EC2 and container fleet options, scaling and session-management tools. citeturn338874search3turn338874search6

### Kubernetes-native
Agones provides GameServer, Fleet, GameServerAllocation and FleetAutoscaler primitives for dedicated-server orchestration on Kubernetes. citeturn338874search10turn338874search0

## UE Replication Note

UE currently documents generic replication, Replication Graph and Iris as available replication approaches. Iris is designed around filtering/prioritization and shared replication work, but Epic's current documentation labels it experimental; prove the final choice in 100-player soak and combat benchmarks before lock-in. citeturn287724search2turn287724search7

## Persistence Rule

The match server sends one signed, idempotent match result. It does not write every combat event to the persistent database.

## Observability

Track:

- server frame time p50/p95/p99
- game-thread time
- physics time
- AI time
- replication time
- bytes/sec in/out
- packet loss
- RTT distribution
- actor count
- destruction count
- connected players
- reconnect count
- match completion rate
