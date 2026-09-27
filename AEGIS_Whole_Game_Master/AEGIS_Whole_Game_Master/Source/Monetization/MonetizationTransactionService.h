#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MonetizationTypes.generated.h"

UENUM(BlueprintType)
enum class ECommerceTxnState : uint8
{
    Created,
    AwaitingProvider,
    Pending,
    Verifying,
    Verified,
    Granting,
    Granted,
    Cancelled,
    Expired,
    Rejected,
    RetryableError,
    Reversed
};

UENUM(BlueprintType)
enum class ECommerceProductType : uint8
{
    PremiumPass,
    UltimatePass,
    TierSkip,
    Subscription,
    Cosmetic,
    Currency
};

USTRUCT(BlueprintType)
struct FPurchaseIntent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TransactionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProductId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IdempotencyKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Storefront;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Region;
};

USTRUCT(BlueprintType)
struct FVerifiedPurchase
{
    GENERATED_BODY()

    UPROPERTY()
    FString TransactionId;

    UPROPERTY()
    FString ProviderToken;

    UPROPERTY()
    FString ProductId;

    UPROPERTY()
    FString PlayerId;

    UPROPERTY()
    FDateTime ProviderTimestamp;

    UPROPERTY()
    bool bIsPending = false;

    UPROPERTY()
    bool bIsRevoked = false;
};

DECLARE_DELEGATE_OneParam(FOnCommerceCompleted, const FVerifiedPurchase&);

UCLASS()
class AEGIS_API UCommerceTransactionService : public UObject
{
    GENERATED_BODY()

public:
    bool BeginPurchase(const FPurchaseIntent& Intent);
    bool HandleProviderPending(const FString& ProviderToken);
    bool HandleProviderCallback(const FString& ProviderToken);

private:
    bool VerifyProviderReceipt(const FString& ProviderToken, FVerifiedPurchase& OutPurchase);
    bool ValidateIntent(const FPurchaseIntent& Intent) const;
    bool ReserveIdempotency(const FString& IdempotencyKey);
    bool GrantEntitlementsAtomically(const FVerifiedPurchase& Purchase);
    bool MarkGranted(const FString& TransactionId);
    bool HasExistingGrant(const FVerifiedPurchase& Purchase) const;
    bool IsStorefrontAllowed(const FPurchaseIntent& Intent) const;

    FOnCommerceCompleted Completion;
};
