#include "Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"

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
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Loc, Rot);
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Loc);
    }

    Owner->Destroy();
}
