#include "Actors/ForceVolume.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Pawns/PlayerShip.h"
#include "Kismet/GameplayStatics.h"

AForceVolume::AForceVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	// Root: Sphere trigger volume
	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetSphereRadius(TriggerRadius);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(TriggerVolume);

	// Arrow component for visualizing force direction
	ForceDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForceDirectionArrow"));
	ForceDirectionArrow->SetupAttachment(RootComponent);
	ForceDirectionArrow->ArrowSize = 2.0f;
	ForceDirectionArrow->ArrowLength = 200.0f;
}

void AForceVolume::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AForceVolume::OnTriggerBeginOverlap);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AForceVolume::OnTriggerEndOverlap);
	}

	// Update trigger radius if changed in editor
	if (TriggerVolume)
	{
		TriggerVolume->SetSphereRadius(TriggerRadius);
	}
}

void AForceVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply force to all actors currently in the trigger
	FVector EffectiveForce = GetEffectiveForceDirection() * ForceMagnitude;
	for (APlayerShip* ShipInTrigger : ActorsInTrigger)
	{
		if (ShipInTrigger && ShipInTrigger->GetRootComponent())
		{
			if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ShipInTrigger->GetRootComponent()))
			{
				RootPrim->AddForce(EffectiveForce, NAME_None, true);
			}
		}
	}
}

void AForceVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	APlayerShip* PlayerShip = Cast<APlayerShip>(OtherActor);
	if (PlayerShip)
	{
		AddActorToTrigger(PlayerShip);
	}
}

void AForceVolume::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	APlayerShip* PlayerShip = Cast<APlayerShip>(OtherActor);
	if (PlayerShip)
	{
		RemoveActorFromTrigger(PlayerShip);
	}
}

void AForceVolume::AddActorToTrigger(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	APlayerShip* PlayerShip = Cast<APlayerShip>(Actor);
	if (PlayerShip)
	{
		ActorsInTrigger.Add(PlayerShip);
	}
}

void AForceVolume::RemoveActorFromTrigger(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	APlayerShip* PlayerShip = Cast<APlayerShip>(Actor);
	if (PlayerShip)
	{
		ActorsInTrigger.Remove(PlayerShip);
	}
}

FVector AForceVolume::GetEffectiveForceDirection() const
{
	// Determine which direction to use
	FVector Direction = ForceDirection;

	if (bUseActorForward)
	{
		Direction = GetActorForwardVector();
	}
	else if (bUseActorRight)
	{
		Direction = GetActorRightVector();
	}
	else if (bUseActorUp)
	{
		Direction = GetActorUpVector();
	}

	// Normalize the direction to ensure consistent force magnitude
	return Direction.GetSafeNormal();
}
