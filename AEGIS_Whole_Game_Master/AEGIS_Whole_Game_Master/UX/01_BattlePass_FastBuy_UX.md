# AEGIS — Battle Pass Fast-Buy UX Blueprint

## Design goal
Make the purchase path fast to understand and fast to complete after deliberate user intent, without dark patterns.

### Screen A — Season Forge
```text
┌─────────────────────────────────────────────┐
│ SEASON: FORGE OF THE CROWN                  │
│                                             │
│        45 / 100 TIERS                       │
│                                             │
│ FREE TRACK        PREMIUM TRACK             │
│ [loot]             [loot]                   │
│ [loot]             [locked reward]          │
│                                             │
│ PREMIUM PASS                                │
│ Unlock premium rewards you already earned   │
│                                             │
│ ₹XXX                                         │
│ [ PURCHASE ]                                │
│                                             │
│ [Compare Ultimate]     [Back]              │
└─────────────────────────────────────────────┘
```

### Screen B — Purchase confirmation
```text
┌─────────────────────────────────────┐
│ CONFIRM PURCHASE                    │
│                                     │
│ Premium Pass                        │
│ Unlock: 45 eligible premium tiers   │
│                                     │
│ Total: ₹XXX                         │
│                                     │
│ [ Confirm purchase ]   [ Cancel ]   │
└─────────────────────────────────────┘
```

### Screen C — Server-confirmed reward reveal
```text
PURCHASE CONFIRMED

45 PREMIUM REWARDS UNLOCKED

[Reveal All]
[Inspect]
[Continue]
```

### Tier skip drawer
```text
UNLOCK TIERS
Current: 45
Target: 48

3 tiers × configured price
Total: XXX Premium Currency

[Confirm unlock] [Cancel]
```

### Accessibility
- Minimum target sizes appropriate for VR.
- Voice/read-aloud labels.
- Strong contrast.
- No color-only purchase status.
- Confirmation vibration is distinct from gameplay haptics.
