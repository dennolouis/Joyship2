#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OscillatingMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EOscillatingMovementState : uint8
{
    Moving UMETA(DisplayName = "Moving"),
    Paused UMETA(DisplayName = "Paused")
};

UCLASS( ClassGroup=(Movement), meta=(BlueprintSpawnableComponent) )
class JOYSHIP2_API UOscillatingMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOscillatingMovementComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Offset from the starting location (e.g., (0, 0, 100) moves up 100 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector LocationOffset = FVector::ZeroVector;

    // Speed of movement between locations (units per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 100.f;

    // Pause time between reaching each location (0 = no pause)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PauseTime = 0.f;

    // Starting location (captured at BeginPlay)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    FVector StartingLocation = FVector::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartMovement();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetLocationOffset(FVector NewLocationOffset);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetPauseTime(float NewPauseTime);

private:
    // Whether the component is actively moving
    bool bIsMoving = false;

    // Current state (moving or paused)
    EOscillatingMovementState CurrentState = EOscillatingMovementState::Moving;

    // Time spent pausing
    float PauseElapsedTime = 0.f;

    // Whether we're moving towards the target location
    bool bMovingTowardTarget = true;

    // Get the current destination based on direction
    FVector GetCurrentDestination() const;
};
