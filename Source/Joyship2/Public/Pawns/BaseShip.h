#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "BaseShip.generated.h"

UCLASS()
class JOYSHIP2_API ABaseShip : public APawn
{
	GENERATED_BODY()

public:
	ABaseShip();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/* ---------------- COMPONENTS ---------------- */

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
    UCapsuleComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	UStaticMeshComponent* ShipMesh;


	/* ---------------- HEALTH ---------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Health")
	float CurrentHealth;

	UFUNCTION(BlueprintCallable)
	virtual void ApplyDamage(float DamageAmount);

	virtual void OnShipDestroyed();

	/* ---------------- MOVEMENT ---------------- */

	// Current velocity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Movement")
	FVector Velocity = FVector::ZeroVector;

	// Forward thrust force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float ThrustForce = 1400.f;

	// Turning speed (degrees/sec)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float TurnSpeed = 140.f;

	// Drag applied each frame (closer to 1 = less drag)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float Drag = 0.985f;

	// Maximum speed clamp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float MaxSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float GravityForce = 600.f;


	/* Movement functions */
	UFUNCTION(BlueprintCallable)
	void ApplyThrust(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void RotateShip(float Input, float DeltaTime);
};
