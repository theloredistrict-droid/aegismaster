#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HomeForgeDirector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHomeArenaReady);

UCLASS()
class AEGISPRESENTATION_API AHomeForgeDirector : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Home|Transition")
    void RequestArenaTransition(FName ArenaId);

    UFUNCTION(BlueprintPure, Category="Home|Transition")
    bool IsArenaReady() const { return bArenaReady; }

    UPROPERTY(BlueprintAssignable, Category="Home|Transition")
    FHomeArenaReady OnArenaReady;

protected:
    virtual void BeginPlay() override;

    void WarmArenaAssets(FName ArenaId);
    void OnArenaAssetsReady();
    void BeginVisualTransition();

    UPROPERTY(VisibleInstanceOnly)
    bool bArenaReady = false;

    UPROPERTY(VisibleInstanceOnly)
    FName PendingArenaId;
};
