#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "VRCombatTypes.h"
#include "VRWeaponBindingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRBindStateSignature, const FVRBindRuntimeState&, State);

/** Data-driven physical blade-to-blade bind coordinator. */
UCLASS(ClassGroup=(AEGISVR), BlueprintType, meta=(BlueprintSpawnableComponent))
class AEGISVR_API UVRWeaponBindingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVRWeaponBindingComponent();

    /** Evaluate a newly detected contact and decide whether it becomes a bind. */
    UFUNCTION(BlueprintCallable)
    bool TryBeginBind(const FVRWeaponContact& Contact, UPrimitiveComponent* OwnWeaponBody);

    /** Update an active bind from current relative motion. */
    UFUNCTION(BlueprintCallable)
    void UpdateBind(float DeltaTime);

    /** Force release, normally after a separation/twist/timeout condition. */
    UFUNCTION(BlueprintCallable)
    void ReleaseBind();

    UPROPERTY(BlueprintAssignable)
    FVRBindStateSignature OnBindStateChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float MaxNormalSpeed = 2.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float MaxTangentialSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float MinPressureToFirm = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float BreakImpulseThreshold = 1400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float TwistBreakDegrees = 42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    float MaxBindLifetime = 1.75f;

    UPROPERTY(BlueprintReadOnly)
    FVRBindRuntimeState RuntimeState;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<UPhysicsConstraintComponent> Constraint = nullptr;

    TWeakObjectPtr<UPrimitiveComponent> OwnBody;
    TWeakObjectPtr<UPrimitiveComponent> OtherBody;

    bool IsEligible(const FVRWeaponContact& Contact) const;
    float CalculatePressure01() const;
    bool ShouldBreakBind(float DeltaTime) const;
    void ConfigureConstraint(const FVector& WorldPoint);
    void BroadcastState();
};
