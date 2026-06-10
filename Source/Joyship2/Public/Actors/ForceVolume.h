#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "ForceVolume.generated.h"

class APlayerShip;

UCLASS()
class JOYSHIP2_API AForceVolume : public AActor
{
	GENERATED_BODY()

public:
	AForceVolume();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Sphere trigger volume
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForceVolume")
	USphereComponent* TriggerVolume;

	// Arrow component for visualizing force direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForceVolume")
	UArrowComponent* ForceDirectionArrow;

	// Force settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	float TriggerRadius = 200.0f;

	// Magnitude of the force to apply (in Unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	float ForceMagnitude = 1000.0f;

	// Direction of the force (will be normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	FVector ForceDirection = FVector(1.0f, 0.0f, 0.0f);

	// If true, uses the actor's forward vector instead of ForceDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	bool bUseActorForward = false;

	// If true, uses the actor's right vector instead of ForceDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	bool bUseActorRight = false;

	// If true, uses the actor's up vector instead of ForceDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceVolume")
	bool bUseActorUp = false;

	// Blueprint-callable function to add an actor to the force trigger
	UFUNCTION(BlueprintCallable, Category = "ForceVolume")
	void AddActorToTrigger(AActor* Actor);

	// Blueprint-callable function to remove an actor from the force trigger
	UFUNCTION(BlueprintCallable, Category = "ForceVolume")
	void RemoveActorFromTrigger(AActor* Actor);

private:
	// Set of actors currently in the trigger
	UPROPERTY()
	TSet<APlayerShip*> ActorsInTrigger;

	// Overlap callbacks
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Calculates the actual force direction based on settings
	FVector GetEffectiveForceDirection() const;
};
