#include "Components/HealthComponent.h"
#include "Actors/Collectable.h"
#include "Kismet/GameplayStatics.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
    CurrentHealth -= DamageAmount;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    if (CurrentHealth <= 0.f)
    {
        Explode();
    }
}

void UHealthComponent::Explode()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Loc = Owner->GetActorLocation();
    FRotator Rot = Owner->GetActorRotation();

    if (ExplosionEffect)
    {
        UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, Loc, Rot);
        if (SpawnedEffect)
        {
            // Auto destroy the component when the system finishes
            SpawnedEffect->SetAutoDestroy(true);
        }
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Loc);
    }

    // Spawn collectable based on probability if class is set
    if (CollectableClass && FMath::FRand() < CollectableSpawnChance)
    {
        GetWorld()->SpawnActor<ACollectable>(CollectableClass, Loc, Rot);
    }

    Owner->Destroy();
}
