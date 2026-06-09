// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorTeleported, AActor*, TeleportedActor, APortal*, ExitPortal);

UCLASS()
class JOYSHIP2_API APortal : public AActor
{
	GENERATED_BODY()

public:
	APortal();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Sphere trigger volume
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	USphereComponent* TriggerVolume;

	// Visual mesh (optional)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UStaticMeshComponent* MeshComp;

	// Portal settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float TriggerRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float TeleportCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector ExitOffset = FVector(0.0f, 0.0f, 100.0f);

	// Linked portal for teleportation destination
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	APortal* LinkedPortal;

	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Effects")
	UParticleSystem* TeleportEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Effects")
	USoundBase* TeleportSound;

	// Delegate called when an actor is teleported
	UPROPERTY(BlueprintAssignable, Category = "Portal|Events")
	FOnActorTeleported OnActorTeleported;

	// Teleport functions
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void PerformTeleportation(AActor* ActorToTeleport, APortal* ExitPortal);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SetLinkedPortal(APortal* NewLinkedPortal);

	// Check if actor is on cooldown
	bool IsActorOnCooldown(AActor* Actor) const;

private:
	// Overlap callback
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Cooldown tracking
	UPROPERTY()
	TMap<AActor*, float> TeleportCooldownMap;
};
