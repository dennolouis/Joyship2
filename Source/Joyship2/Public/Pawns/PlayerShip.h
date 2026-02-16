#pragma once

#include "CoreMinimal.h"
#include "BaseShip.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerShip.generated.h"

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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* ------------ INPUT STATE ------------ */

	float RotationInput = 0.f;
	bool bThrusting = false;
};
