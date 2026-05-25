#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UParticleSystem;
class USoundBase;
class ACollectable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealthCapacity);

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

    // Collectable class to spawn on death (optional)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collectable")
    TSubclassOf<ACollectable> CollectableClass;

    // Probability threshold for spawning a collectable (0.0 to 1.0). Random number is compared against this value.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CollectableSpawnChance = 0.5f;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Explode();

    /* ------------ DELEGATES ------------ */
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;
};
