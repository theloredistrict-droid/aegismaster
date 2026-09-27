#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDefinition.generated.h"

UENUM(BlueprintType)
enum class EWeaponMassClass : uint8
{
    Light,
    Medium,
    Heavy,
    Colossal
};

USTRUCT(BlueprintType)
struct FWeaponCombatProfile
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EWeaponMassClass MassClass = EWeaponMassClass::Medium;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MassKg = 1.4f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float PoiseDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float GuardDamage = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ImpactForce = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float TraceRadiusCm = 3.0f;
};

USTRUCT(BlueprintType)
struct FWeaponTraceSocketSet
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName BaseSocket = TEXT("Blade_Base");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName MidSocket = TEXT("Blade_Mid");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName TipSocket = TEXT("Blade_Tip");
};

UCLASS(BlueprintType)
class AEGISWEAPONS_API UWeaponDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
    TSoftObjectPtr<UStaticMesh> WeaponMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
    FWeaponCombatProfile CombatProfile;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
    FWeaponTraceSocketSet TraceSockets;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
    TSoftObjectPtr<UDataAsset> AnimationSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
    TSoftObjectPtr<class UNiagaraSystem> TrailFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
    TSoftObjectPtr<class UNiagaraSystem> ImpactFX;
};
