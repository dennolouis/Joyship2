#include "Actors/Portal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"

APortal::APortal()
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

	// Visual mesh (optional, can be set in Blueprint)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnTriggerBeginOverlap);
	}

	// Update trigger radius if changed in editor
	if (TriggerVolume)
	{
		TriggerVolume->SetSphereRadius(TriggerRadius);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Portal] %s initialized. LinkedPortal: %s"), *GetName(), LinkedPortal ? *LinkedPortal->GetName() : TEXT("None"));
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APortal::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Don't teleport if on cooldown
	if (IsActorOnCooldown(OtherActor))
	{
		return;
	}

	// Only teleport pawns
	if (!OtherActor->IsA<APawn>())
	{
		return;
	}

	TeleportActor(OtherActor);
}

void APortal::TeleportActor(AActor* ActorToTeleport)
{
	if (!ActorToTeleport || !LinkedPortal)
	{
		return;
	}

	PerformTeleportation(ActorToTeleport, LinkedPortal);
}

void APortal::PerformTeleportation(AActor* ActorToTeleport, APortal* ExitPortal)
{
	if (!ActorToTeleport || !ExitPortal || !CanTeleport() || !ExitPortal->CanTeleport())
	{
		return;
	}

	OnActorTeleported.Broadcast(ActorToTeleport, ExitPortal);
	bCanTeleport = false;
	ExitPortal->SetCanTeleport(false);

	// Get exit location
	FVector ExitLocation = ExitPortal->GetActorLocation() + ExitPortal->ExitOffset;

	// Preserve the actor's rotation
	FRotator ExitRotation = ActorToTeleport->GetActorRotation();

	// Preserve velocity (important for arcade gameplay feel)
	FVector PreservingVelocity = FVector::ZeroVector;
	if (APawn* Pawn = Cast<APawn>(ActorToTeleport))
	{
		// Try to get velocity if the pawn has movement
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			PreservingVelocity = Character->GetCharacterMovement()->Velocity;
		}
		else
		{
			// For other pawns, try to get velocity through the root component
			if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ActorToTeleport->GetRootComponent()))
			{
				PreservingVelocity = RootPrim->GetComponentVelocity();
			}
		}
	}

	// Teleport the actor
	ActorToTeleport->SetActorLocation(ExitLocation);
	ActorToTeleport->SetActorRotation(ExitRotation);

	// Restore velocity
	if (APawn* Pawn = Cast<APawn>(ActorToTeleport))
	{
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			Character->GetCharacterMovement()->Velocity = PreservingVelocity;
		}
		else
		{
			if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ActorToTeleport->GetRootComponent()))
			{
				RootPrim->SetPhysicsLinearVelocity(PreservingVelocity);
			}
		}
	}

	// Play effects at exit portal
	if (ExitPortal->TeleportEffect)
	{
		UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExitPortal->TeleportEffect, ExitLocation);
		if (SpawnedEffect)
		{
			// Auto destroy the component when the system finishes
			SpawnedEffect->SetAutoDestroy(true);
		}
	}

	if (ExitPortal->TeleportSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExitPortal->TeleportSound, ExitLocation);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Portal] Teleported %s from %s to %s"), *ActorToTeleport->GetName(), *GetName(), *ExitPortal->GetName());
}

bool APortal::IsActorOnCooldown(AActor* Actor) const
{
	return !bCanTeleport;
}

void APortal::SetLinkedPortal(APortal* NewLinkedPortal)
{
	LinkedPortal = NewLinkedPortal;
}
