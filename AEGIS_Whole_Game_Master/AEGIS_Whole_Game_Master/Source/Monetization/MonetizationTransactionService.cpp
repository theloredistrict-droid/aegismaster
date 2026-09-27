#include "MonetizationTransactionService.h"

// Architectural reference implementation.
// Provider verification, ledger writes, and idempotency storage belong on the
// authoritative backend, not inside the UE5 client.

bool UCommerceTransactionService::BeginPurchase(const FPurchaseIntent& Intent)
{
    if (!ValidateIntent(Intent) || !IsStorefrontAllowed(Intent))
    {
        return false;
    }

    // Reserve a unique idempotency key before creating a provider-side purchase.
    if (!ReserveIdempotency(Intent.IdempotencyKey))
    {
        // Existing idempotency key means the client should reconcile the
        // existing transaction instead of starting another grant path.
        return false;
    }

    // Transition: Created -> AwaitingProvider
    // The platform/store SDK is invoked from a platform adapter.
    return true;
}

bool UCommerceTransactionService::HandleProviderPending(const FString& ProviderToken)
{
    // Transition: AwaitingProvider -> Pending
    // Never grant entitlements from this state.
    return !ProviderToken.IsEmpty();
}

bool UCommerceTransactionService::HandleProviderCallback(const FString& ProviderToken)
{
    if (ProviderToken.IsEmpty())
    {
        return false;
    }

    FVerifiedPurchase Purchase;
    // Transition: Pending/AwaitingProvider -> Verifying
    if (!VerifyProviderReceipt(ProviderToken, Purchase))
    {
        // Transition: Verifying -> Rejected or RetryableError
        return false;
    }

    if (Purchase.bIsPending)
    {
        // Stay Pending. No grant.
        return false;
    }

    if (Purchase.bIsRevoked)
    {
        // Transition -> Reversed; reconcile an existing entitlement.
        return true;
    }

    if (HasExistingGrant(Purchase))
    {
        // Idempotent replay: transaction already fulfilled.
        return true;
    }

    // Transition: Verifying -> Verified -> Granting
    if (!GrantEntitlementsAtomically(Purchase))
    {
        // Retryable grant failure. Keep the transaction durable and retry.
        return false;
    }

    // Transition: Granting -> Granted
    if (!MarkGranted(Purchase.TransactionId))
    {
        return false;
    }

    if (Completion.IsBound())
    {
        Completion.Execute(Purchase);
    }

    return true;
}

// The following methods are intentionally abstract because the real implementation
// depends on the backend storage/auth/provider SDK. The authoritative service should:
// 1) verify with the platform/provider,
// 2) write the transaction + entitlement in one logical idempotent operation,
// 3) emit an entitlement event,
// 4) reconcile refunds/revocations later.

bool UCommerceTransactionService::VerifyProviderReceipt(
    const FString& ProviderToken,
    FVerifiedPurchase& OutPurchase)
{
    // Server-side HTTPS call to a provider adapter.
    return false;
}

bool UCommerceTransactionService::ValidateIntent(const FPurchaseIntent& Intent) const
{
    return !Intent.PlayerId.IsEmpty()
        && !Intent.ProductId.IsEmpty()
        && !Intent.IdempotencyKey.IsEmpty();
}

bool UCommerceTransactionService::ReserveIdempotency(const FString& IdempotencyKey)
{
    return !IdempotencyKey.IsEmpty();
}

bool UCommerceTransactionService::GrantEntitlementsAtomically(
    const FVerifiedPurchase& Purchase)
{
    // Database transaction:
    // INSERT transaction if absent
    // INSERT entitlement ledger grant if absent
    // COMMIT
    return false;
}

bool UCommerceTransactionService::MarkGranted(const FString& TransactionId)
{
    return !TransactionId.IsEmpty();
}

bool UCommerceTransactionService::HasExistingGrant(
    const FVerifiedPurchase& Purchase) const
{
    return false;
}

bool UCommerceTransactionService::IsStorefrontAllowed(
    const FPurchaseIntent& Intent) const
{
    // Resolve region/storefront policy from the backend catalog rather than
    // trusting the client. This is essential for web-vs-native payment routing.
    return !Intent.Storefront.IsEmpty();
}
