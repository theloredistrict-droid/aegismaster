#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VRLocomotionComponent.generated.h"

UENUM(BlueprintType)
enum class EVRRotationMode : uint8
{
    Smooth,
    Snap
};

USTRUCT(BlueprintType)
struct FVRDashRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bActive = false;

    UPROPERTY(BlueprintReadOnly)
    FVector Direction = FVector::ForwardVector;

    UPROPERTY(BlueprintReadOnly)
    float RemainingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Speed = 0.0f;
};

/**
 * Comfort-aware VR locomotion: stick movement plus a physically swept dash.
 * The dash is resolved through collision rather than teleporting the pawn.
 */
UCLASS(ClassGroup=(AEGISVR), BlueprintType, meta=(BlueprintSpawnableComponent))
class AEGISVR_API UVRLocomotionComponent : public UPawnMovementComponent
{
    GENERATED_BODY()

public:
    UVRLocomotionComponent();

    UFUNCTION(BlueprintCallable)
    void SetMoveInput(FVector2D Stick);

    UFUNCTION(BlueprintCallable)
    void StartDash(FVector2D OptionalDirection = FVector2D::ZeroVector);

    UFUNCTION(BlueprintCallable)
    void SetRotationMode(EVRRotationMode InMode);

    UFUNCTION(BlueprintCallable)
    void SetComfortVignetteEnabled(bool bEnabled);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float MaxWalkSpeed = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float Acceleration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
    float DashSpeed = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
    float DashDuration = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
    float DashStaminaCost = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Comfort")
    bool bUseComfortVignette = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Comfort")
    EVRRotationMode RotationMode = EVRRotationMode::Snap;

    UPROPERTY(BlueprintReadOnly)
    FVRDashRuntime Dash;

protected:
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector2D MoveInput = FVector2D::ZeroVector;
    FVector CurrentVelocity = FVector::ZeroVector;
    void TickDash(float DeltaTime);
    void TickWalk(float DeltaTime);
    FVector GetPlanarMoveDirection() const;
    bool SweepMove(const FVector& Delta, FHitResult& Hit) const;
};
