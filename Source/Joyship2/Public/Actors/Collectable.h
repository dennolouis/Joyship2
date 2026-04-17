#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Collectable.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class APlayerShip;

UCLASS()
class JOYSHIP2_API ACollectable : public AActor
{
    GENERATED_BODY()

public:
    ACollectable();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Visual mesh (set in Blueprint)

    // Visual mesh (set in Blueprint)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collectable")
    UStaticMeshComponent* MeshComp;

    // Magnet radius: when player is within this distance the collectable will move toward the player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    float MagnetRadius = 600.f;

    // Interp speed used to move toward the player when magnet is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    float AttractionSpeed = 8.f;

    // Distance at which the collectable is considered collected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    float CollectDistance = 100.f;

    // If true, the collectable will auto-collect when within CollectDistance.
    // If false, collection must be triggered explicitly (e.g. via overlap in Blueprint calling Collect()).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    bool bAllowRangeCollect = false;

    // Internal flag to avoid double-collect
    bool bCollected = false;

    // Cached player pawn while magnet is active
    APawn* CachedPlayer = nullptr;

    // Tick gating to reduce frequency of attraction calculations
    float TickAccumulator = 0.f;

    // How often (seconds) to run attraction logic while active (default 20 Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    float TickInterval = 0.05f;

    // Fail-safe: automatically attempt collection after this delay (seconds) when magnet is activated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
    float MagnetFailSafeDelay = 0.5f;

    // Internal timer handle for fail-safe
    FTimerHandle MagnetFailSafeTimer;

    // Attempt a collection when the fail-safe fires
    void AttemptFailSafeCollect();

    // Note: collision components are expected to be added in Blueprint. Call ActivateMagnet/DeactivateMagnet and Collect from BP overlap events.

public:
    // Collect the item (can be called from C++ or Blueprints). Collector may be null.
    UFUNCTION(BlueprintCallable, Category = "Collectable")
    void Collect(APlayerShip* Collector);

    // Blueprint event to respond to collection (spawn FX, give fuel/coins, etc.)
    UFUNCTION(BlueprintImplementableEvent, Category = "Collectable")
    void OnCollected(APlayerShip* Collector);

    // Activate magnet attraction toward the given pawn (call from Blueprint when your trigger begins overlap)
    UFUNCTION(BlueprintCallable, Category = "Collectable")
    void ActivateMagnet(APawn* Pawn);

    // Deactivate magnet attraction for the given pawn (call from Blueprint when your trigger ends overlap)
    UFUNCTION(BlueprintCallable, Category = "Collectable")
    void DeactivateMagnet(APawn* Pawn);
};
