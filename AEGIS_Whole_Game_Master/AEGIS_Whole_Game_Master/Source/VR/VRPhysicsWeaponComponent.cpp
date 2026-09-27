#include "VRPhysicsWeaponComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UVRPhysicsWeaponComponent::UVRPhysicsWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVRPhysicsWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    WeaponBody = Cast<UPrimitiveComponent>(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
    if (!WeaponBody)
    {
        return;
    }

    WeaponBody->SetSimulatePhysics(true);
    WeaponBody->SetNotifyRigidBodyCollision(true);
    WeaponBody->SetUseCCD(bRequireCCD);
    WeaponBody->OnComponentHit.AddDynamic(this, &UVRPhysicsWeaponComponent::HandleComponentHit);
}

void UVRPhysicsWeaponComponent::SetTrackedTarget(const FVRTrackedPose& InTarget)
{
    TrackedTarget = InTarget;
}

void UVRPhysicsWeaponComponent::SetPhysicsDriveEnabled(bool bEnabled)
{
    bPhysicsDriveEnabled = bEnabled;
}

FVector UVRPhysicsWeaponComponent::ComputeLinearDrive(const FTransform& Current, float DeltaTime) const
{
    if (!WeaponBody)
    {
        return FVector::ZeroVector;
    }

    const float SafeDt = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
    const FVector PositionError = TrackedTarget.WorldTransform.GetLocation() - Current.GetLocation();
    const FVector CurrentVelocity = WeaponBody->GetPhysicsLinearVelocity();

    // PD-style velocity target. Tracking remains 1:1 in intent while Chaos owns collision response.
    const FVector DesiredVelocity = TrackedTarget.LinearVelocity + PositionError * 60.0f;
    const FVector DesiredAcceleration = (DesiredVelocity - CurrentVelocity) / SafeDt;
    return DesiredAcceleration.GetClampedToMaxSize(LinearDriveForce);
}

FVector UVRPhysicsWeaponComponent::ComputeAngularDrive(const FTransform& Current, float DeltaTime) const
{
    if (!WeaponBody)
    {
        return FVector::ZeroVector;
    }

    const float SafeDt = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
    const FQuat ErrorQuat = TrackedTarget.WorldTransform.GetRotation() * Current.GetRotation().Inverse();

    FVector Axis;
    float Angle;
    ErrorQuat.ToAxisAndAngle(Axis, Angle);
    if (Angle > PI)
    {
        Angle -= 2.0f * PI;
    }

    const FVector CurrentAngularVelocity = WeaponBody->GetPhysicsAngularVelocityInRadians();
    const FVector DesiredAngularVelocity = TrackedTarget.AngularVelocity + Axis * (Angle * 45.0f);
    const FVector AngularAcceleration = (DesiredAngularVelocity - CurrentAngularVelocity) / SafeDt;
    return AngularAcceleration.GetClampedToMaxSize(AngularDriveTorque);
}

void UVRPhysicsWeaponComponent::ResolveTrackingSafety(const FTransform& Current)
{
    const float Error = FVector::Distance(Current.GetLocation(), TrackedTarget.WorldTransform.GetLocation());
    if (Error <= MaxTrackingError)
    {
        return;
    }

    // Hook a normalized Resistance haptic signal here. Intentionally do not teleport the
    // simulated body to the tracked target; the collision-resolved pose remains authoritative.
}

void UVRPhysicsWeaponComponent::StepPhysics(float DeltaTime)
{
    if (!bPhysicsDriveEnabled || !WeaponBody || !TrackedTarget.bTracked)
    {
        return;
    }

    const FTransform Current = WeaponBody->GetComponentTransform();
    ResolveTrackingSafety(Current);

    const FVector LinearAcceleration = ComputeLinearDrive(Current, DeltaTime);
    WeaponBody->AddForce(LinearAcceleration, NAME_None, true);

    const FVector AngularAcceleration = ComputeAngularDrive(Current, DeltaTime);
    WeaponBody->AddTorqueInRadians(AngularAcceleration, NAME_None, true);
}

void UVRPhysicsWeaponComponent::HandleComponentHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (!WeaponBody || !OtherComp)
    {
        return;
    }

    const FVector WeaponVelocity = WeaponBody->GetPhysicsLinearVelocityAtPoint(Hit.ImpactPoint);
    const FVector OtherVelocity = OtherComp->IsSimulatingPhysics()
        ? OtherComp->GetPhysicsLinearVelocityAtPoint(Hit.ImpactPoint)
        : FVector::ZeroVector;

    const FVector RelativeVelocity = WeaponVelocity - OtherVelocity;
    const float NormalSpeed = FMath::Abs(FVector::DotProduct(RelativeVelocity, Hit.ImpactNormal));
    const FVector TangentialVelocity = RelativeVelocity -
        FVector::DotProduct(RelativeVelocity, Hit.ImpactNormal) * Hit.ImpactNormal;

    FVRWeaponContact Contact;
    Contact.OtherComponent = OtherComp;
    Contact.Point = Hit.ImpactPoint;
    Contact.Normal = Hit.ImpactNormal;
    Contact.RelativeVelocity = RelativeVelocity;
    Contact.NormalSpeed = NormalSpeed;
    Contact.TangentialSpeed = TangentialVelocity.Size();
    Contact.Energy = 0.5f * FMath::Max(WeaponBody->GetMass(), 0.01f) * FMath::Square(NormalSpeed);

    Contact.Type = (OtherActor && OtherActor->ActorHasTag(TEXT("Weapon")))
        ? EVRWeaponContactType::Deflect
        : EVRWeaponContactType::EnvironmentImpact;

    OnWeaponContact.Broadcast(Contact);
}
