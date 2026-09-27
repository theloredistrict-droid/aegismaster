#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "VRCombatTypes.h"
#include "VRPhysicsWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRWeaponContactSignature, const FVRWeaponContact&, Contact);

/**
 * Drives a physical weapon from tracked XR pose without teleporting the collidable body.
 * The owning actor should contain a simulated primitive as the weapon body and a
 * dedicated grip/hand proxy used as the motor target.
 */
UCLASS(ClassGroup=(AEGISVR), BlueprintType, meta=(BlueprintSpawnableComponent))
class AEGISVR_API UVRPhysicsWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVRPhysicsWeaponComponent();

    /** Set the latest XR target. Called from the XR layer once per frame. */
    UFUNCTION(BlueprintCallable)
    void SetTrackedTarget(const FVRTrackedPose& InTarget);

    /** Advances the physics motor. Call from a physics-safe/sub-step path. */
    UFUNCTION(BlueprintCallable)
    void StepPhysics(float DeltaTime);

    /** Enables or disables physical weapon drive. */
    UFUNCTION(BlueprintCallable)
    void SetPhysicsDriveEnabled(bool bEnabled);

    UPROPERTY(BlueprintAssignable)
    FVRWeaponContactSignature OnWeaponContact;

    /** Max linear drive force used to follow the tracked pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Physics")
    float LinearDriveForce = 6500.0f;

    /** Max angular drive torque used to follow the tracked pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Physics")
    float AngularDriveTorque = 1800.0f;

    /** Distance tolerance before the controller target is considered a collision-resolved lag. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Physics")
    float MaxTrackingError = 0.18f;

    /** CCD and sub-step support are expected on the simulated weapon body. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR Collision")
    bool bRequireCCD = true;

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> WeaponBody = nullptr;

    FVRTrackedPose TrackedTarget;
    bool bPhysicsDriveEnabled = true;

    FVector ComputeLinearDrive(const FTransform& Current, float DeltaTime) const;
    FVector ComputeAngularDrive(const FTransform& Current, float DeltaTime) const;
    void ResolveTrackingSafety(const FTransform& Current);

    UFUNCTION()
    void HandleComponentHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);
};
