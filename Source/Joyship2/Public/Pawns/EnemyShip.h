#pragma once

#include "CoreMinimal.h"
#include "BaseShip.h"
#include "Components/SphereComponent.h"
#include "EnemyShip.generated.h"

class UHealthComponent;
class USphereComponent;

UCLASS()
class JOYSHIP2_API AEnemyShip : public ABaseShip
{
    GENERATED_BODY()

public:
    AEnemyShip();

    // Aggro sphere that triggers when player enters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
    USphereComponent* AggroSphere;

    // Health component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
    UHealthComponent* HealthComp;

    // Start following target (callable from Blueprints)
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void StartFollowing(AActor* Target);

    // Stop following (callable from Blueprints)
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void StopFollowing();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Overlap handlers for the aggro sphere
    UFUNCTION()
    void OnAggroBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAggroEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Following state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
    bool bFollowing = false;

    // The actor we are following (player)
    AActor* FollowTarget = nullptr;

    // Rotation interpolation speed when turning to face the target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float RotationSpeed = 4.f;
};
