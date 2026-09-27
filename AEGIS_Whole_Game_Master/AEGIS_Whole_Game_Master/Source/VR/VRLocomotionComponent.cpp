#include "VRLocomotionComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"

UVRLocomotionComponent::UVRLocomotionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVRLocomotionComponent::SetMoveInput(FVector2D Stick)
{
    MoveInput = Stick.GetClampedToMaxSize(1.0f);
}

void UVRLocomotionComponent::StartDash(FVector2D OptionalDirection)
{
    const FVector2D Requested = OptionalDirection.IsNearlyZero() ? MoveInput : OptionalDirection;
    FVector Direction = GetPlanarMoveDirection();

    if (!Requested.IsNearlyZero())
    {
        const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
        const FVector Right = GetOwner()->GetActorRightVector().GetSafeNormal2D();
        Direction = (Forward * Requested.Y + Right * Requested.X).GetSafeNormal2D();
    }

    if (Direction.IsNearlyZero())
    {
        Direction = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
    }

    Dash.bActive = true;
    Dash.Direction = Direction;
    Dash.RemainingTime = DashDuration;
    Dash.Speed = DashSpeed;
}

void UVRLocomotionComponent::SetRotationMode(EVRRotationMode InMode)
{
    RotationMode = InMode;
}

void UVRLocomotionComponent::SetComfortVignetteEnabled(bool bEnabled)
{
    bUseComfortVignette = bEnabled;
}

void UVRLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!PawnOwner || !UpdatedComponent)
    {
        return;
    }

    if (Dash.bActive)
    {
        TickDash(DeltaTime);
    }
    else
    {
        TickWalk(DeltaTime);
    }
}

FVector UVRLocomotionComponent::GetPlanarMoveDirection() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = GetOwner()->GetActorRightVector().GetSafeNormal2D();
    return (Forward * MoveInput.Y + Right * MoveInput.X).GetSafeNormal2D();
}

void UVRLocomotionComponent::TickWalk(float DeltaTime)
{
    const FVector TargetVelocity = GetPlanarMoveDirection() * (MaxWalkSpeed * MoveInput.Size());
    CurrentVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, Acceleration);

    FHitResult Hit;
    SweepMove(CurrentVelocity * DeltaTime, Hit);
}

void UVRLocomotionComponent::TickDash(float DeltaTime)
{
    const float Step = FMath::Min(DeltaTime, Dash.RemainingTime);
    const FVector Delta = Dash.Direction * Dash.Speed * Step;

    FHitResult Hit;
    const bool bBlocked = SweepMove(Delta, Hit);

    Dash.RemainingTime -= Step;
    if (bBlocked || Dash.RemainingTime <= 0.0f)
    {
        Dash.bActive = false;
        CurrentVelocity = FVector::ZeroVector;
    }
}

bool UVRLocomotionComponent::SweepMove(const FVector& Delta, FHitResult& Hit) const
{
    if (!UpdatedComponent)
    {
        return false;
    }

    const FQuat Rotation = UpdatedComponent->GetComponentQuat();
    const FVector Start = UpdatedComponent->GetComponentLocation();
    const FVector End = Start + Delta;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(VRLocomotionSweep), false, GetOwner());
    return UpdatedComponent->MoveComponent(Delta, Rotation, true, &Hit, MOVECOMP_NoFlags, ETeleportType::None);
}
