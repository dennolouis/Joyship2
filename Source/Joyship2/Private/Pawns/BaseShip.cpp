#include "Pawns/BaseShip.h"
#include "Components/StaticMeshComponent.h"

ABaseShip::ABaseShip()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Ship mesh
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(Root);
	ShipMesh->SetCollisionProfileName(TEXT("Pawn"));
}

void ABaseShip::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ABaseShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply drag
	Velocity *= Drag;

	// Clamp speed
	if (Velocity.SizeSquared() > MaxSpeed * MaxSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}

	//Velocity += FVector(0.f, 0.f, -GravityForce) * DeltaTime;

	// Move actor with sweep so collisions work
	AddActorWorldOffset(Velocity * DeltaTime, true);
}

/* ---------------- MOVEMENT ---------------- */

void ABaseShip::RotateShip(float Input, float DeltaTime)
{
	if (FMath::IsNearlyZero(Input)) return;

	FRotator Rot = GetActorRotation();
	Rot.Roll += Input * TurnSpeed * DeltaTime;
	SetActorRotation(Rot);
}

void ABaseShip::ApplyThrust(float DeltaTime)
{
	FVector Forward = GetActorUpVector();
	Velocity += Forward * ThrustForce * DeltaTime;
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
