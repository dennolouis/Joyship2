#include "Pawns/BaseShip.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

ABaseShip::ABaseShip()
{
	PrimaryActorTick.bCanEverTick = true;

    // Root (capsule for collisions)
    Root = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));
    Root->InitCapsuleSize(44.f, 88.f);
    // Use a physics-friendly collision profile so gravity and forces are applied
    Root->SetCollisionProfileName(TEXT("PhysicsActor"));
    // Enable physics and gravity by default so Blueprint can simulate physics
    Root->SetSimulatePhysics(true);
    Root->SetEnableGravity(true);
    SetRootComponent(Root);

	// Ship mesh
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    ShipMesh->SetupAttachment(Root);
    // Mesh should not simulate physics when the capsule root is the physics body
    ShipMesh->SetSimulatePhysics(false);
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseShip::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
    if (Root)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] BeginPlay: Root=%s Simulating=%s Gravity=%s Mass=%.2f CollisionProfile=%s CollisionEnabled=%d"),
            *Root->GetName(),
            Root->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"),
            Root->IsGravityEnabled() ? TEXT("true") : TEXT("false"),
            Root->GetMass(),
            *Root->GetCollisionProfileName().ToString(),
            (int)Root->GetCollisionEnabled());
        // Log world gravity
        if (GetWorld())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BaseShip] World gravity Z=%.2f"), GetWorld()->GetGravityZ());
        }

        // List overlapping components (helps detect initial overlap that can block physics)
        TArray<UPrimitiveComponent*> Overlaps;
        Root->GetOverlappingComponents(Overlaps);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] BeginPlay: Overlapping components count=%d"), Overlaps.Num());
        for (UPrimitiveComponent* Comp : Overlaps)
        {
            if (Comp)
            {
                UE_LOG(LogTemp, Warning, TEXT("[BaseShip]   Overlap: %s (Simulating=%s CollisionEnabled=%d)"), *Comp->GetName(), Comp->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"), (int)Comp->GetCollisionEnabled());
            }
        }

        // If blueprint defaults overrode C++ settings, enforce correct physics settings at runtime
        if (!Root->IsSimulatingPhysics())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BaseShip] BeginPlay: Enabling SimulatePhysics on Root"));
            Root->SetSimulatePhysics(true);
        }
        Root->SetEnableGravity(true);
        Root->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Root->SetCollisionProfileName(TEXT("PhysicsActor"));
        Root->SetMobility(EComponentMobility::Movable);

        // Wake physics and apply a tiny impulse to ensure the body wakes and reacts
        Root->WakeRigidBody();
        Root->WakeAllRigidBodies();
        FVector TinyImpulse = FVector(0.f, 0.f, -5.f) * Root->GetMass();
        Root->AddImpulse(TinyImpulse);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] BeginPlay: Applied tiny impulse=(%.2f,%.2f,%.2f) to wake body"), TinyImpulse.X, TinyImpulse.Y, TinyImpulse.Z);
    }
}

void ABaseShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply drag
    // If physics is simulating, let the physics system drive movement
    if (Root && !Root->IsSimulatingPhysics())
    {
        Velocity *= Drag;

        // Clamp speed
        if (Velocity.SizeSquared() > MaxSpeed * MaxSpeed)
        {
            Velocity = Velocity.GetSafeNormal() * MaxSpeed;
        }

        // Move actor with sweep so collisions work
        AddActorWorldOffset(Velocity * DeltaTime, true);
    }
    else if (Root)
    {
        // When simulating physics, update stored velocity from the physics body
        Velocity = Root->GetComponentVelocity();
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] Tick: Physics simulated. Velocity=(%.2f,%.2f,%.2f) Mass=%.2f"), Velocity.X, Velocity.Y, Velocity.Z, Root->GetMass());
    }
}

/* ---------------- MOVEMENT ---------------- */

void ABaseShip::RotateShip(float Input, float DeltaTime)
{
	if (FMath::IsNearlyZero(Input)) return;

    // If physics is enabled, directly set angular velocity to avoid unbounded spin.
    if (Root && Root->IsSimulatingPhysics())
    {
        // Desired angular speed in degrees/sec -> convert to radians/sec
        float DesiredDegPerSec = Input * TurnSpeed;
        float DesiredRadPerSec = FMath::DegreesToRadians(DesiredDegPerSec);

        // Use actor forward as the roll axis
        FVector RollAxis = GetActorForwardVector();

        // Clamp to a reasonable angular velocity
        float MaxRad = FMath::DegreesToRadians(720.f); // 720 deg/s cap
        DesiredRadPerSec = FMath::Clamp(DesiredRadPerSec, -MaxRad, MaxRad);

        FVector AngularVel = RollAxis * DesiredRadPerSec;
        Root->SetPhysicsAngularVelocityInRadians(AngularVel, false);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip: Set angular velocity=(%.2f,%.2f,%.2f) (rad/s) Input=%.2f"), AngularVel.X, AngularVel.Y, AngularVel.Z, Input);
    }
    else
    {
        FRotator Rot = GetActorRotation();
        Rot.Roll += Input * TurnSpeed * DeltaTime;
        SetActorRotation(Rot);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip: Kinematic rotation applied. NewRoll=%.2f Input=%.2f"), Rot.Roll, Input);
    }
}

void ABaseShip::ApplyThrust(float DeltaTime)
{
    FVector Forward = GetActorUpVector();

    if (Root && Root->IsSimulatingPhysics())
    {
        // Apply a force in the forward direction
        FVector Force = Forward * ThrustForce * Root->GetMass();
        Root->AddForce(Force);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] ApplyThrust: Applied force=(%.2f,%.2f,%.2f) ThrustForce=%.2f Mass=%.2f"), Force.X, Force.Y, Force.Z, ThrustForce, Root->GetMass());
    }
    else
    {
        Velocity += Forward * ThrustForce * DeltaTime;
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] ApplyThrust: Kinematic thrust. Velocity=(%.2f,%.2f,%.2f)"), Velocity.X, Velocity.Y, Velocity.Z);
    }
}

/* ---------------- HEALTH ---------------- */

void ABaseShip::ApplyDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.f)
	{
		OnShipDestroyed();
	}
}

void ABaseShip::OnShipDestroyed()
{
	Destroy();
}
