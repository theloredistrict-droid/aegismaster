#pragma once
#include "CoreMinimal.h"
#include "MonetizationTypes.generated.h"

USTRUCT(BlueprintType)
struct FBattlePassPurchaseResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    int32 UnlockedTiers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 RetroactiveRewardCount = 0;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorCode;
};

USTRUCT(BlueprintType)
struct FEntitlementGrant
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString PlayerId;

    UPROPERTY(BlueprintReadOnly)
    FString EntitlementId;

    UPROPERTY(BlueprintReadOnly)
    FString SourceTransactionId;

    UPROPERTY(BlueprintReadOnly)
    FDateTime GrantedAtUtc;
};
