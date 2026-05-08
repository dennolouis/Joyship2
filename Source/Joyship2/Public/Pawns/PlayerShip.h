#pragma once

#include "CoreMinimal.h"
#include "BaseShip.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerShip.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelChanged, float, NewFuel, float, MaxFuelCapacity);

UCLASS()
class JOYSHIP2_API APlayerShip : public ABaseShip
{
	GENERATED_BODY()

public:
	APlayerShip();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UFUNCTION(BlueprintCallable)
	void RotateInput(float Value);

	UFUNCTION(BlueprintCallable)
	void StartThrust();

	UFUNCTION(BlueprintCallable)
	void StopThrust();

	// Refill fuel by Amount (clamped to MaxFuel)
	UFUNCTION(BlueprintCallable, Category = "Ship|Fuel")
	void RefillFuel(float Amount);

	// Enable boost - increases thrust force
	UFUNCTION(BlueprintCallable, Category = "Ship|Boost")
	void EnableBoost();

	// Disable boost - restores normal thrust force
	UFUNCTION(BlueprintCallable, Category = "Ship|Boost")
	void DisableBoost();

	/* ------------ DELEGATES ------------ */
	UPROPERTY(BlueprintAssignable, Category = "Ship|Fuel")
	FOnFuelChanged OnFuelChanged;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* ------------ INPUT STATE ------------ */

	float RotationInput = 0.f;
	bool bThrusting = false;

    /* ---------------- FUEL ---------------- */
    // Maximum fuel capacity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Fuel")
    float MaxFuel = 100.f;

    // Current fuel amount
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Fuel")
    float CurrentFuel = 0.f;

    // Fuel consumption rate (units per second) while thrusting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Fuel")
    float FuelConsumptionRate = 10.f;

	/* ------------ BOOST ---------------- */
	// Original thrust force (cached when boost is first enabled)
	float CachedNormalThrustForce = 0.f;

	// Boost multiplier applied to ThrustForce
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Boost")
	float BoostMultiplier = 1.5f;

	// Whether boost is currently active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Boost")
	bool bBoostActive = false;

	// Camera FOV when not boosting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Boost")
	float NormalFOV = 90.f;

	// Camera FOV when boosting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Boost")
	float BoostFOV = 110.f;

	// Speed of FOV transition (higher = faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Boost")
	float FOVTransitionSpeed = 5.f;
};
