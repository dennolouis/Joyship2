#include "Pawns/BaseShip.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"

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

        // Bind overlap handler
        Root->SetGenerateOverlapEvents(true);
        Root->OnComponentBeginOverlap.AddDynamic(this, &ABaseShip::OnRootBeginOverlap);
        // Also bind actor-level overlap as backup
        OnActorBeginOverlap.AddDynamic(this, &ABaseShip::OnActorBeginOverlapHandler);
        // Bind hit handler for blocking collisions
        Root->SetNotifyRigidBodyCollision(true);
        Root->OnComponentHit.AddDynamic(this, &ABaseShip::OnRootHit);

        // Log overlap-related settings for debugging
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] Overlap settings: GenerateOverlapEvents=%d CollisionEnabled=%d ObjectType=%d Response_WorldDynamic=%d Response_PhysicsBody=%d Response_Pawn=%d"),
            Root->GetGenerateOverlapEvents(),
            (int)Root->GetCollisionEnabled(),
            (int)Root->GetCollisionObjectType(),
            (int)Root->GetCollisionResponseToChannel(ECC_WorldDynamic),
            (int)Root->GetCollisionResponseToChannel(ECC_PhysicsBody),
            (int)Root->GetCollisionResponseToChannel(ECC_Pawn));
    }
}

void ABaseShip::OnRootBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[BaseShip] OnRootBeginOverlap: Other=%s"), OtherActor ? *OtherActor->GetName() : TEXT("None"));
}

void ABaseShip::OnActorBeginOverlapHandler(AActor* OverlappedActor, AActor* OtherActor)
{
    UE_LOG(LogTemp, Warning, TEXT("[BaseShip] OnActorBeginOverlap: Other=%s"), OtherActor ? *OtherActor->GetName() : TEXT("None"));
}

void ABaseShip::OnRootHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("[BaseShip] OnRootHit: Other=%s Normal=(%.2f,%.2f,%.2f)"), OtherActor ? *OtherActor->GetName() : TEXT("None"), NormalImpulse.X, NormalImpulse.Y, NormalImpulse.Z);
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

        // Smoothly interpolate towards target velocities (set by input handlers)
        FVector CurLin = Root->GetPhysicsLinearVelocity();
        FVector NewLin = FMath::VInterpTo(CurLin, TargetLinearVelocity, DeltaTime, LinearSmooth);

        // If target requests movement but current velocity is effectively zero, give a small kick
        if (TargetLinearVelocity.SizeSquared() > KINDA_SMALL_NUMBER && CurLin.SizeSquared() < 1.f)
        {
            FVector Boost = TargetLinearVelocity * 0.25f;
            NewLin = FMath::VInterpTo(CurLin, Boost, DeltaTime, FMath::Max(LinearSmooth * 4.f, 10.f));
            UE_LOG(LogTemp, Warning, TEXT("[BaseShip] Tick: Boosting start movement. Boost=(%.2f,%.2f,%.2f)"), Boost.X, Boost.Y, Boost.Z);
        }

        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] Tick: CurLin=(%.2f,%.2f,%.2f) TargetLin=(%.2f,%.2f,%.2f) NewLin=(%.2f,%.2f,%.2f)"),
            CurLin.X, CurLin.Y, CurLin.Z,
            TargetLinearVelocity.X, TargetLinearVelocity.Y, TargetLinearVelocity.Z,
            NewLin.X, NewLin.Y, NewLin.Z);
        Root->SetPhysicsLinearVelocity(NewLin, false);

        FVector CurAng = Root->GetPhysicsAngularVelocityInRadians();
        FVector NewAng = FMath::VInterpTo(CurAng, TargetAngularVelocity, DeltaTime, AngularSmooth);
        Root->SetPhysicsAngularVelocityInRadians(NewAng, false);
    }
}

/* ---------------- MOVEMENT ---------------- */

void ABaseShip::RotateShip(float Input, float DeltaTime)
{
    // Log entry so we can see incoming input and physics state
    if (Root)
    {
        FVector CurAngVel = Root->GetPhysicsAngularVelocityInRadians();
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip ENTRY: Input=%.4f Simulating=%s CurAngVel=(%.4f,%.4f,%.4f)"), Input, Root->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"), CurAngVel.X, CurAngVel.Y, CurAngVel.Z);
    }

    // If input is nearly zero, stop any physics angular velocity so the ship stops rotating.
    if (FMath::IsNearlyZero(Input))
    {
        if (Root && Root->IsSimulatingPhysics())
        {
            Root->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector, false);
            UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip: Input nearly zero — cleared angular velocity"));
        }
        return;
    }

    // If physics is enabled, directly set angular velocity to avoid unbounded spin.
    if (Root && Root->IsSimulatingPhysics())
    {
        // Desired angular speed in degrees/sec -> convert to radians/sec
        float DesiredDegPerSec = -Input * TurnSpeed;
        float DesiredRadPerSec = FMath::DegreesToRadians(DesiredDegPerSec);

        // Use actor forward as the roll axis
        FVector RollAxis = GetActorForwardVector();

        // Clamp to a reasonable angular velocity
        float MaxRad = FMath::DegreesToRadians(720.f); // 720 deg/s cap
        DesiredRadPerSec = FMath::Clamp(DesiredRadPerSec, -MaxRad, MaxRad);

        FVector AngularVel = RollAxis * DesiredRadPerSec;
        TargetAngularVelocity = AngularVel;
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip: Set TargetAngularVelocity=(%.2f,%.2f,%.2f) (rad/s) Input=%.2f"), AngularVel.X, AngularVel.Y, AngularVel.Z, Input);
    }
    else
    {
        FRotator Rot = GetActorRotation();
        Rot.Roll += -Input * TurnSpeed * DeltaTime;
        SetActorRotation(Rot);
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] RotateShip: Kinematic rotation applied. NewRoll=%.2f Input=%.2f"), Rot.Roll, Input);
    }
}

void ABaseShip::ApplyThrust(float DeltaTime)
{
    FVector Forward = GetActorUpVector();

    if (Root && Root->IsSimulatingPhysics())
    {
        // Set target linear velocity (smoothed each Tick)
        FVector DesiredVel = Forward * ThrustForce; // treat ThrustForce as target speed for simplicity
        TargetLinearVelocity = DesiredVel;
        UE_LOG(LogTemp, Warning, TEXT("[BaseShip] ApplyThrust: Set TargetLinearVelocity=(%.2f,%.2f,%.2f)"), DesiredVel.X, DesiredVel.Y, DesiredVel.Z);
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
    PlayExplosionEffect();
    Destroy();
}

void ABaseShip::PlayExplosionEffect()
{
    FVector Loc = GetActorLocation();
    FRotator Rot = GetActorRotation();

    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Loc, Rot);
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Loc);
    }
}

void ABaseShip::Fire()
{
    if (!ProjectileClass) return;
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn at the ship's muzzle using the ship's up/forward/right offsets
    FVector SpawnLoc = GetActorLocation() + GetActorUpVector() * MuzzleOffset.Z + GetActorForwardVector() * MuzzleOffset.X + GetActorRightVector() * MuzzleOffset.Y;
    FRotator SpawnRot = GetActorRotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = Cast<APawn>(GetInstigator());

    AActor* Projectile = World->SpawnActor<AActor>(ProjectileClass, SpawnLoc, SpawnRot, Params);
    if (Projectile)
    {
        // Try to set initial velocity if it has a ProjectileMovementComponent
        UProjectileMovementComponent* PM = Projectile->FindComponentByClass<UProjectileMovementComponent>();
        if (PM)
        {
            // Force the projectile to travel along the ship's up vector
            PM->Velocity = GetActorUpVector() * PM->InitialSpeed;
        }
    }
}
