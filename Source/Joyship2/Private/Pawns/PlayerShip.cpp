#include "Pawns/PlayerShip.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

APlayerShip::APlayerShip()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	// Good default Joyship-style settings
	SpringArm->TargetArmLength = 900.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 4.f;
	SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f)); // slight tilt

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void APlayerShip::BeginPlay()
{
	Super::BeginPlay();

    // initialize fuel
    CurrentFuel = MaxFuel;
    OnFuelChanged.Broadcast(CurrentFuel, MaxFuel);

    // Initialize boost fuel
    CurrentBoostFuel = MaxBoostFuel;
    OnBoostFuelChanged.Broadcast(CurrentBoostFuel, MaxBoostFuel);

    // Set initial camera FOV
    if (Camera)
    {
        Camera->SetFieldOfView(NormalFOV);
    }
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update camera FOV smoothly based on boost state
	if (Camera)
	{
		float TargetFOV = (bBoostActive && bThrusting) ? BoostFOV : NormalFOV;
		float CurrentFOV = Camera->FieldOfView;
		float LerpAlpha = FMath::Clamp(FOVTransitionSpeed * DeltaTime, 0.f, 1.f);
		float NewFOV = FMath::Lerp(CurrentFOV, TargetFOV, LerpAlpha);
		
		// Only update if the value actually changed
		if (!FMath::IsNearlyEqual(NewFOV, CurrentFOV, 0.01f))
		{
			Camera->SetFieldOfView(NewFOV);
		}
	}

	// Manage boost fuel consumption and regeneration
	if (bBoostActive && bThrusting && CurrentBoostFuel > 0.f)
	{
		// Consume boost fuel while boosting and thrusting
		float BoostFuelUsed = BoostFuelConsumptionRate * DeltaTime;
		float PreviousBoostFuel = CurrentBoostFuel;
		CurrentBoostFuel = FMath::Max(0.f, CurrentBoostFuel - BoostFuelUsed);
		// Broadcast if boost fuel changed
		if (CurrentBoostFuel != PreviousBoostFuel)
		{
			OnBoostFuelChanged.Broadcast(CurrentBoostFuel, MaxBoostFuel);
		}
		// If boost fuel ran out, disable boost
		if (CurrentBoostFuel <= 0.f)
		{
			DisableBoost();
		}
		// Reset the regen timer when actively boosting
		TimeSinceBoostDeactivated = 0.f;
	}
	else if (!bBoostActive && CurrentBoostFuel < MaxBoostFuel)
	{
		// Track time since boost was deactivated
		TimeSinceBoostDeactivated += DeltaTime;

		// Only regenerate after the delay has passed
		if (TimeSinceBoostDeactivated >= BoostFuelRegenDelay)
		{
			float BoostFuelRegen = BoostFuelRegenRate * DeltaTime;
			float PreviousBoostFuel = CurrentBoostFuel;
			CurrentBoostFuel = FMath::Min(MaxBoostFuel, CurrentBoostFuel + BoostFuelRegen);
			// Broadcast if boost fuel changed
			if (CurrentBoostFuel != PreviousBoostFuel)
			{
				OnBoostFuelChanged.Broadcast(CurrentBoostFuel, MaxBoostFuel);
			}
		}
	}

	// Apply rotation every frame
    // Apply rotation every frame (call even when input is nearly zero so physics can be cleared)
    RotateShip(RotationInput, DeltaTime);

	// Apply thrust while held
	// Always call ApplyThrust so targets update; when not thrusting, target goes to zero
	if (bThrusting)
	{
		// If we have fuel, apply thrust and consume fuel
		if (CurrentFuel > 0.f)
		{
			ApplyThrust(DeltaTime);
			// Consume fuel
			float FuelUsed = FuelConsumptionRate * DeltaTime;
			float PreviousFuel = CurrentFuel;
			CurrentFuel = FMath::Max(0.f, CurrentFuel - FuelUsed);
			// Broadcast if fuel changed
			if (CurrentFuel != PreviousFuel)
			{
				OnFuelChanged.Broadcast(CurrentFuel, MaxFuel);
			}
			// If fuel ran out this frame, stop thrusting next frame
			if (CurrentFuel <= 0.f)
			{
				bThrusting = false;
			}
		}
		else
		{
			// No fuel: ensure thrusting is disabled and target is zero
			bThrusting = false;
			this->TargetLinearVelocity = FVector::ZeroVector;
		}
	}
	else
	{
		// Tell the ship to stop producing thrust (target velocity zero)
		this->TargetLinearVelocity = FVector::ZeroVector;
	}
}

void APlayerShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis binding (keyboard, controller, or touch axis)
	PlayerInputComponent->BindAxis("Rotate", this, &APlayerShip::RotateInput);

	// Thrust button
	PlayerInputComponent->BindAction("Thrust", IE_Pressed, this, &APlayerShip::StartThrust);
	PlayerInputComponent->BindAction("Thrust", IE_Released, this, &APlayerShip::StopThrust);
}

/* ------------ INPUT HANDLERS ------------ */

void APlayerShip::RotateInput(float Value)
{
	RotationInput = Value;
}

void APlayerShip::StartThrust()
{
	// Only allow starting thrust if we have fuel
	if (CurrentFuel > 0.f)
	{
		bThrusting = true;
	}
}

void APlayerShip::StopThrust()
{
    bThrusting = false;
}

void APlayerShip::RefillFuel(float Amount)
{
    if (Amount <= 0.f) return;
    CurrentFuel = FMath::Clamp(CurrentFuel + Amount, 0.f, MaxFuel);
    OnFuelChanged.Broadcast(CurrentFuel, MaxFuel);
    UE_LOG(LogTemp, Warning, TEXT("[PlayerShip] RefillFuel: NewFuel=%.2f"), CurrentFuel);
}

void APlayerShip::EnableBoost()
{
	if (bBoostActive) return;
	if (CurrentBoostFuel <= 0.f) return;

	bBoostActive = true;
	// Cache the current ThrustForce if not already cached
	if (CachedNormalThrustForce == 0.f)
	{
		CachedNormalThrustForce = ThrustForce;
	}
	// Apply boost multiplier
	ThrustForce = CachedNormalThrustForce * BoostMultiplier;
	UE_LOG(LogTemp, Warning, TEXT("[PlayerShip] Boost enabled. ThrustForce: %.2f -> %.2f"), CachedNormalThrustForce, ThrustForce);
}

void APlayerShip::DisableBoost()
{
	if (!bBoostActive) return;

	bBoostActive = false;
	// Restore original thrust force
	ThrustForce = CachedNormalThrustForce;
	// Start the regeneration delay timer
	TimeSinceBoostDeactivated = 0.f;
	UE_LOG(LogTemp, Warning, TEXT("[PlayerShip] Boost disabled. ThrustForce: %.2f"), ThrustForce);
}

void APlayerShip::RefillBoostFuel(float Amount)
{
    if (Amount <= 0.f) return;
    float PreviousBoostFuel = CurrentBoostFuel;
    CurrentBoostFuel = FMath::Clamp(CurrentBoostFuel + Amount, 0.f, MaxBoostFuel);
    // Broadcast if boost fuel changed
    if (CurrentBoostFuel != PreviousBoostFuel)
    {
        OnBoostFuelChanged.Broadcast(CurrentBoostFuel, MaxBoostFuel);
    }
    UE_LOG(LogTemp, Warning, TEXT("[PlayerShip] RefillBoostFuel: NewBoostFuel=%.2f"), CurrentBoostFuel);
}
