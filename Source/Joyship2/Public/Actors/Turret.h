#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class JOYSHIP2_API ATurret : public AActor
{
    GENERATED_BODY()

public:
    ATurret();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Trigger box
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    UBoxComponent* Trigger;

    // Rotating part (the part that aims)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    UStaticMeshComponent* AimMesh;

    // Muzzle location
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    USceneComponent* Muzzle;

    // Projectile to fire
    UPROPERTY(EditAnywhere, Category = "Turret")
    TSubclassOf<AActor> ProjectileClass;

    // Turn speed degrees/sec
    UPROPERTY(EditAnywhere, Category = "Turret")
    float TurnSpeed = 90.f;

    // Seconds needed to look at target before firing
    UPROPERTY(EditAnywhere, Category = "Turret")
    float LookTimeRequired = 1.f;

    // Fire interval while locked
    UPROPERTY(EditAnywhere, Category = "Turret")
    float FireInterval = 1.f;

    // Angle tolerance in degrees to consider "looking at" target
    UPROPERTY(EditAnywhere, Category = "Turret")
    float AimToleranceDegrees = 5.f;

protected:
    // Current target pawn
    APawn* TargetPawn = nullptr;

    // Accumulated time looking at target
    float LookAccum = 0.f;

    // Fire accumulator
    float FireAccum = 0.f;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
