#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AdaptiveAI_DataAssets.generated.h"

UENUM(BlueprintType)
enum class EAIAdaptiveAction : uint8
{
    Pressure,
    Counter,
    Reposition,
    Defend,
    Probe
};

USTRUCT(BlueprintType)
struct FPatternFeatureVector
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RepetitionScore = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DirectionBias = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimingBias = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefenseBias = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpacingBias = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CounterConfidence = 0.f;
};

USTRUCT(BlueprintType)
struct FAdaptiveCounterCandidate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CounterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAIAdaptiveAction Action = EAIAdaptiveAction::Counter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseUtility = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Risk = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VarietyBonus = 0.f;
};

UCLASS(BlueprintType)
class AEGISAI_API UAdaptiveAIPolicy : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FAdaptiveCounterCandidate> CounterCandidates;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MinimumReactionDelay = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ConfidenceDecayPerSecond = 0.15f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MaxAdaptationStrength = 1.0f;
};
