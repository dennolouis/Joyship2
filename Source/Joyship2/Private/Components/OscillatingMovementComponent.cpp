#include "Components/OscillatingMovementComponent.h"

UOscillatingMovementComponent::UOscillatingMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;
}

void UOscillatingMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // Capture the starting location from the owner actor
    AActor* Owner = GetOwner();
    if (Owner)
    {
        StartingLocation = Owner->GetActorLocation();
    }
}

void UOscillatingMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner || !bIsMoving)
    {
        return;
    }

    if (CurrentState == EOscillatingMovementState::Paused)
    {
        PauseElapsedTime += DeltaTime;
        if (PauseElapsedTime >= PauseTime)
        {
            PauseElapsedTime = 0.f;
            CurrentState = EOscillatingMovementState::Moving;
            bMovingTowardTarget = !bMovingTowardTarget;
        }
        return;
    }

    // Get current and target positions
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector Destination = GetCurrentDestination();

    // Calculate direction and distance
    FVector Direction = (Destination - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, Destination);
    float StepDistance = MovementSpeed * DeltaTime;

    if (StepDistance >= Distance)
    {
        // Reached destination
        Owner->SetActorLocation(Destination);

        // Transition to pause state if pause time is set
        if (PauseTime > 0.f)
        {
            CurrentState = EOscillatingMovementState::Paused;
            PauseElapsedTime = 0.f;
        }
        else
        {
            // No pause, switch direction immediately
            bMovingTowardTarget = !bMovingTowardTarget;
        }
    }
    else
    {
        // Move towards destination
        FVector NewLocation = CurrentLocation + Direction * StepDistance;
        Owner->SetActorLocation(NewLocation);
    }
}

FVector UOscillatingMovementComponent::GetCurrentDestination() const
{
    return bMovingTowardTarget ? (StartingLocation + LocationOffset) : StartingLocation;
}

void UOscillatingMovementComponent::StartMovement()
{
    bIsMoving = true;
    CurrentState = EOscillatingMovementState::Moving;
    bMovingTowardTarget = true;
    PauseElapsedTime = 0.f;
}

void UOscillatingMovementComponent::StopMovement()
{
    bIsMoving = false;
    CurrentState = EOscillatingMovementState::Moving;
    PauseElapsedTime = 0.f;
}

void UOscillatingMovementComponent::SetLocationOffset(FVector NewLocationOffset)
{
    LocationOffset = NewLocationOffset;
}

void UOscillatingMovementComponent::SetMovementSpeed(float NewSpeed)
{
    MovementSpeed = FMath::Max(NewSpeed, 0.f);
}

void UOscillatingMovementComponent::SetPauseTime(float NewPauseTime)
{
    PauseTime = FMath::Max(NewPauseTime, 0.f);
}
