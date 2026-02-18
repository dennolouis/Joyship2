#include "Pawns/PlayerShip.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

APlayerShip::APlayerShip()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	SetRootComponent(ShipMesh);

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
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply rotation every frame
	if (!FMath::IsNearlyZero(RotationInput))
	{
		RotateShip(RotationInput, DeltaTime);
	}

	// Apply thrust while held
	if (bThrusting)
	{
		ApplyThrust(DeltaTime);
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
	bThrusting = true;
}

void APlayerShip::StopThrust()
{
	bThrusting = false;
}
