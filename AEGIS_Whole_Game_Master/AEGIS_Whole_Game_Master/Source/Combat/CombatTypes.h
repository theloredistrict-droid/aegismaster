#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Neutral,
    Locomotion,
    AttackStartup,
    AttackActive,
    AttackRecovery,
    Guard,
    Parry,
    HitReact,
    Stagger,
    GuardBreak,
    Dodge,
    Riposte,
    Knockdown,
    Dead
};

UENUM(BlueprintType)
enum class EStrikeDirection : uint8
{
    None,
    High,
    Low,
    Left,
    Right,
    Forward,
    Back,
    HighLeft,
    HighRight,
    LowLeft,
    LowRight
};

UENUM(BlueprintType)
enum class ECombatResolution : uint8
{
    None,
    Miss,
    Hit,
    Guarded,
    PerfectParry,
    Staggered,
    GuardBroken,
    KnockedDown
};

USTRUCT(BlueprintType)
struct FCombatInputCommand
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStrikeDirection Direction = EStrikeDirection::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double Timestamp = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bHeavy : 1 = false;
};

USTRUCT(BlueprintType)
struct FCombatTimingWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime = 0.0f;

    FORCEINLINE bool Contains(float Time) const
    {
        return Time >= StartTime && Time <= EndTime;
    }
};

USTRUCT(BlueprintType)
struct FCombatHitContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> Attacker = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> Defender = nullptr;

    UPROPERTY(BlueprintReadOnly)
    EStrikeDirection StrikeDirection = EStrikeDirection::None;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(BlueprintReadOnly)
    float ImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PoiseDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    ECombatResolution Resolution = ECombatResolution::None;
};
