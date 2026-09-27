#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "CombatStateMachine.generated.h"

class UAnimMontage;
class UWeaponDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStateChanged, ECombatState, PreviousState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatResolutionEvent, const FCombatHitContext&, Context);

UCLASS(ClassGroup=(Combat), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class AEGISCOMBAT_API UCombatStateMachine : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatStateMachine();

    UFUNCTION(BlueprintCallable, Category="Combat|State")
    bool TryEnterState(ECombatState RequestedState);

    UFUNCTION(BlueprintCallable, Category="Combat|Input")
    void BufferAttack(EStrikeDirection Direction, bool bHeavy);

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void BeginGuard();

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void EndGuard();

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void BeginParryWindow();

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void EndParryWindow();

    UFUNCTION(BlueprintCallable, Category="Combat|Animation")
    void BeginAttackActiveWindow();

    UFUNCTION(BlueprintCallable, Category="Combat|Animation")
    void EndAttackActiveWindow();

    UFUNCTION(BlueprintCallable, Category="Combat|Combat")
    ECombatResolution ResolveIncomingHit(const FCombatHitContext& IncomingHit, double ContactTimestamp);

    UFUNCTION(BlueprintPure, Category="Combat|State")
    ECombatState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category="Combat|Defense")
    bool IsParryWindowOpen() const { return bParryWindowOpen; }

    UFUNCTION(BlueprintPure, Category="Combat|Combat")
    bool IsAttackWindowOpen() const { return bAttackWindowOpen; }

    UPROPERTY(BlueprintAssignable, Category="Combat|Events")
    FCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Combat|Events")
    FCombatResolutionEvent OnCombatResolution;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    bool CanTransition(ECombatState From, ECombatState To) const;
    void ConsumeBufferedInput();
    void UpdateStateTimers(double NowSeconds);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Tuning")
    float InputBufferSeconds = 0.20f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Tuning")
    float PerfectParryWindowSeconds = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Tuning")
    float GuardBreakRecoverySeconds = 0.70f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|State")
    ECombatState CurrentState = ECombatState::Neutral;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Input")
    TArray<FCombatInputCommand> InputBuffer;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    bool bParryWindowOpen = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    bool bAttackWindowOpen = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    double ParryWindowOpenedAt = -1.0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    double AttackWindowOpenedAt = -1.0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    double StateEnteredAt = 0.0;

    // Frame/timestamp used by authoritative hit resolution.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Timing")
    uint32 SimulationFrame = 0;
};
