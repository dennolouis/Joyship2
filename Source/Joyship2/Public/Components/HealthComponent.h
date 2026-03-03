#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JOYSHIP2_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Max health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.f;

    // Current health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.f;

    // Explosion effect to play on death
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* ExplosionEffect;

    // Explosion sound
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
    USoundBase* ExplosionSound;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Explode();
};
