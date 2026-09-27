#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "VRCombatTypes.generated.h"

UENUM(BlueprintType)
enum class EVRWeaponContactType : uint8
{
    None,
    Hit,
    Deflect,
    Parry,
    Bind,
    Scrape,
    EnvironmentImpact
};

UENUM(BlueprintType)
enum class EVRWeaponBindPhase : uint8
{
    None,
    Soft,
    Firm,
    Releasing
};

USTRUCT(BlueprintType)
struct FVRTrackedPose
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FTransform WorldTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadWrite)
    FVector LinearVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FVector AngularVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    bool bTracked = false;

    UPROPERTY(BlueprintReadWrite)
    float TrackingConfidence = 0.0f;
};

USTRUCT(BlueprintType)
struct FVRWeaponContact
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UPrimitiveComponent> OtherComponent = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector Point = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector Normal = FVector::UpVector;

    UPROPERTY(BlueprintReadOnly)
    FVector RelativeVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float NormalSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TangentialSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Energy = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EVRWeaponContactType Type = EVRWeaponContactType::None;
};

USTRUCT(BlueprintType)
struct FVRBindRuntimeState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EVRWeaponBindPhase Phase = EVRWeaponBindPhase::None;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UPrimitiveComponent> OtherWeapon = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector WorldContactPoint = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ElapsedSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Pressure01 = 0.0f;
};
