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
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			CurrentFuel = FMath::Max(0.f, CurrentFuel - FuelUsed);
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
