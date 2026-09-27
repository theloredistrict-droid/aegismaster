# AEGIS — VR/3D Sword-Fighting Game
## Monetization & Live-Ops Design Architecture
### Production Draft — Commerce, Battle Pass, Web Shop, Security & Ethical UX

> Scope note: this document covers the live-ops/commerce layer. Combat implementation remains separated behind existing gameplay interfaces.

## 1. Product principles

AEGIS uses a cosmetics-first monetization model. Purchases never determine competitive combat power.

Core principles:
- Explicit purchase confirmation: item, price, currency, taxes/fees where applicable, and final charge are visible before authorization.
- No deceptive scarcity. Countdown timers represent genuine availability windows controlled by the catalog service.
- No dark-pattern defaults. No preselected add-ons, accidental double-purchase paths, or confusing currency conversions.
- Earned progression remains meaningful without paying.
- Battle Pass premium rewards unlock retroactively for XP already earned after an explicit purchase.
- Paid skips are optional convenience features, never required to participate in seasonal content.
- Youth/Family controls: configurable spending caps, purchase history, guardian approval hooks where applicable, and storefront/platform parental-control compatibility.
- Server-authoritative entitlements: the client never decides that a purchase succeeded.

## 2. Commerce topology

```text
             +---------------------------+
             |   UE5 Client / VR Hub     |
             |   Battle Pass / Store UX  |
             +-------------+-------------+
                           |
                    HTTPS / platform SDK
                           v
             +---------------------------+
             | Commerce API / BFF        |
             | auth + idempotency        |
             +------+-----------+--------+
                    |           |
          +---------+           +----------------+
          v                                      v
+--------------------+                 +----------------------+
| Transaction Service |                 | Catalog Service      |
| state machine       |                 | SKU/price/windows   |
+---------+----------+                 +----------+-----------+
          |                                       |
          v                                       v
+--------------------+                 +----------------------+
| Entitlement Ledger |                 | Offer/Pass Config    |
| append-only grants |                 | seasonal content    |
+----------+---------+                 +----------------------+
           |
           +--------------------+
           |                    |
           v                    v
   Player Profile DB       Fraud/Audit/Event Bus
```

The UE5 client is a presentation/input client only. It requests a purchase and displays server-confirmed results.

## 3. Currency model

Use at most two player-facing currencies:
1. Earned currency: gameplay-earned and never sold.
2. Premium currency: purchased or granted by eligible promotions/pass rewards.

Never require players to calculate an unclear exchange rate at checkout. Show the real-money price beside the premium amount.

## 4. Battle Pass

### Tracks
- Free Track
- Premium Track
- Ultimate Bundle

### Standard Premium
- Unlocks the premium reward track.
- Does not remove free-track progression.
- XP earned before purchase remains valid.
- Retroactive rewards become claimable after server confirmation.

### Ultimate Bundle
- Premium Track.
- 25 immediate tier unlocks.
- Seasonal cosmetic aura.
- 20% seasonal XP boost.

The Ultimate bundle is presented as a bundle comparison, not as a pressure tactic.

### Tier skips

Architecture supports:
- Single tier unlock.
- Multi-tier unlock.
- Exact quantity confirmation.
- Server-side validation that requested tiers are still eligible.

The UI may show:
`Unlock 3 tiers — 300 Premium Currency`
and a separate final confirmation control.

### Retroactive Claim All

After purchase:

```text
Purchase Confirmed
      |
Load entitlement
      |
Calculate earned XP
      |
Determine all newly unlocked premium rewards
      |
Create claim batch
      |
Grant atomically
      |
Play reward reveal
      |
Persist claim receipt
```

The animation is cosmetic; the ledger remains authoritative.

## 5. Fast Battle Pass purchase UX

### VR / 3D flow

```text
[Physical Season Banner]
       |
[Reward Rack]
       |
[Inspect Premium]
       |
+------------------------------+
| PREMIUM PASS                 |
| 14 rewards already earned   |
| 31 future rewards           |
|                               |
| Price: ₹XXX                  |
| [Purchase] [Back]            |
+------------------------------+
       |
[Final confirmation]
       |
[Server confirmation]
       |
[Claim unlocked rewards]
```

The confirmation stage cannot be bypassed by controller mis-aim, accidental trigger pressure, or a default-selected purchase.

### One-step convenience

For previously authorized, platform-supported purchase sessions, the client may invoke the platform purchase sheet immediately after the player deliberately selects Purchase. The platform controls the final payment authorization.

## 6. Post-match cosmetic offer

Use the match result screen to present a relevant cosmetic preview without pressure language.

Example:

```text
MATCH COMPLETE

Opponent-inspired cosmetic preview
-----------------------------------
Armor Set        1,200 Premium
Weapon Visual     800 Premium

[Inspect]   [Buy]
```

The buy control always exposes final price before payment authorization.

No purchase is initiated merely by touching a preview.

## 7. Flash Store

Catalog service owns:
- start time
- end time
- SKU
- price
- legal/storefront availability
- inventory rule if a real inventory limit exists

Countdowns are derived from trusted UTC server time.

A "limited" label can only appear when a genuine catalog rule makes the offer temporary.

## 8. Gladiator Pass subscription

Benefits:
- Daily premium-currency grant according to the published season configuration.
- Permanent +25% XP multiplier while active.
- Permanent +25% gold multiplier while active.
- One monthly cosmetic armor set.

Subscription UI must state:
- billing interval
- renewal price
- cancellation pathway
- benefits
- trial terms, if any

No hidden auto-renewal wording.

## 9. Web Shop / Cross-platform commerce

Recommended design:
- Account-centric entitlement service.
- Region-aware catalog.
- Platform-specific payment adapters.
- One canonical SKU identity across storefronts.
- Web receipts mapped to the same entitlement ledger.

Important platform boundary:
- Do not assume a web shop can replace native platform billing for digital goods inside every mobile distribution channel.
- Google Play currently requires Google Play Billing for in-app digital goods unless a specific policy exception applies; external payment paths have additional program/region conditions. [Official source: Google Play Payments policy]
- Apple currently provides external-purchase link mechanisms for qualifying apps in specified regions/entitlements; eligibility and storefront rules must be checked before implementation. [Official source: Apple Developer]
- Therefore, the catalog service should select the permitted payment rail per storefront and region rather than hard-code "web payment everywhere."

## 10. Web checkout

```text
Web Shop
  |
Authenticated account
  |
Catalog lookup
  |
Create checkout intent
  |
Payment provider / platform rail
  |
Webhook
  |
Signature validation
  |
Transaction state machine
  |
Entitlement ledger
  |
Game inbox / realtime update
```

Do not trust a browser redirect as proof of payment.

## 11. Transaction security

### Required controls
- Idempotency key per checkout attempt.
- Unique transaction/provider purchase token constraints.
- Server-side receipt/token verification.
- Atomic entitlement grant.
- Immutable audit event.
- Replay protection.
- Pending-state handling.
- Refund/revocation handling.
- Rate limiting.
- Fraud/risk flags.
- Clock-independent server timestamps.

### Never do this

```cpp
Client -> "payment succeeded"
       -> grant item
```

### Do this

```text
Client purchase request
        |
Provider purchase
        |
Provider/server verification
        |
Canonical transaction accepted
        |
Atomic entitlement write
        |
Client notified
```

## 12. Transaction state machine

```text
Created
  |
AwaitingProvider
  |
Pending
  |
Verifying
  |
Verified
  |
Granting
  |
Granted
```

Terminal/error branches:

```text
Created ------> Cancelled
Pending ------> Expired
Verifying ---> Rejected
Granted -----> Reversed
Granting ----> RetryableError
```

`Granted` is idempotent. Reprocessing the same provider token cannot create a second grant.

## 13. Battle Pass entitlement model

```text
SeasonEntitlement
- PlayerId
- SeasonId
- Track
- PurchasedAt
- XPAtPurchase
- UltimateSkipCount
- XPBoostPercent
- ClaimedTierBits / claim records
```

Do not encode every reward as a client-only boolean. The service derives eligibility from the canonical season definition and ledger.

## 14. Live-ops telemetry

Track:
- Pass purchase conversion.
- Free-to-premium conversion.
- Average tiers unlocked at purchase.
- Claim-batch completion.
- Refund rate.
- Payment error rate.
- Purchase latency.
- Web-to-game fulfillment latency.
- Catalog availability errors.

Do not use individual telemetry to pressure a player with personalized scarcity or escalating purchase prompts.

## 15. Live-ops cadence

### Weekly
- Store catalog refresh.
- Cosmetics rotation.
- New challenge set.
- Pricing/catalog integrity audit.

### Season
- Pass launch.
- Balance and progression audit.
- New cosmetic collection.
- Retention and refund review.
- Post-season economy report.

### Incident response
Feature flags can disable:
- a SKU
- a payment rail
- a promotion
- a subscription offer
without deploying a client patch.

## 16. Service SLO targets

Suggested engineering targets:
- Purchase verification p95 < 2 seconds after provider confirmation.
- Entitlement propagation p95 < 5 seconds.
- Duplicate-grant rate: zero.
- Successful recovery after transient provider timeout: >99.9%.
- Catalog reads remain available during store-promo configuration failure.

These are engineering targets, not claims about real-world provider performance.

## 17. Content economics

Keep monetized items primarily cosmetic:
- weapon visual variants
- armor appearance
- rune effects
- banners
- hub decorations
- emotes
- finishers
- profile cosmetics

Competitive ranked outcomes should remain skill/progression driven.

## 18. Release gates

Commerce launch is blocked until:
- duplicate transactions have automated tests.
- refunds revoke/reconcile entitlements.
- pending purchases do not grant items prematurely.
- lost network recovery is tested.
- account migration preserves entitlements.
- platform-specific purchase rails are certified.
- price display is localized and legally reviewed.
- youth/family controls are verified for each storefront.
- catalog changes are audited.
- disaster-recovery restore tests pass.

## 19. Reference
The platform integration layer must be maintained against current first-party storefront rules. Google Play's current documentation requires server-side verification and entitlement handling for completed purchases, with special handling for PENDING transactions and acknowledgement. Apple documents external-purchase link entitlements for qualifying storefronts/regions.
