#include "Pawns/EnemyShip.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HealthComponent.h"
#include "Pawns/PlayerShip.h"

AEnemyShip::AEnemyShip()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create aggro sphere
    AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
    AggroSphere->SetupAttachment(RootComponent);
    AggroSphere->InitSphereRadius(800.f);
    AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    // Allow overlap with common pawn/physics/world dynamic object types so player's physics capsule is detected
    AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AggroSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    AggroSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    AggroSphere->SetGenerateOverlapEvents(true);

    // Health component
    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void AEnemyShip::BeginPlay()
{
    Super::BeginPlay();

    if (AggroSphere)
    {
        AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyShip::OnAggroBeginOverlap);
        AggroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyShip::OnAggroEndOverlap);
    }
}

void AEnemyShip::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFollowing && FollowTarget)
    {
        FVector ToTarget = FollowTarget->GetActorLocation() - GetActorLocation();
        ToTarget.Z = 0.f; // keep movement in XY plane
        float Dist = ToTarget.Size();
        if (Dist > KINDA_SMALL_NUMBER)
        {
            FVector Dir = ToTarget.GetSafeNormal();

            // Rotate to face the target smoothly
            FRotator Current = GetActorRotation();
            FRotator TargetRot = Dir.Rotation();
            // We want the ship's up vector to face the direction used in BaseShip (thrust uses GetActorUpVector), so align Pitch/Yaw accordingly
            // Keep roll zero
            TargetRot.Pitch = 0.f;
            TargetRot.Roll = 0.f;
            FRotator NewRot = FMath::RInterpTo(Current, TargetRot, DeltaTime, 4.f);
            SetActorRotation(NewRot);

            // Move forward along actor up vector (ship's forward for this project is up)
            FVector MoveDir = GetActorUpVector();
            TargetLinearVelocity = MoveDir * ThrustForce;
        }
        else
        {
            TargetLinearVelocity = FVector::ZeroVector;
        }
    }
}

void AEnemyShip::OnAggroBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] OnAggroBeginOverlap Other=%s Comp=%s"), OtherActor ? *OtherActor->GetName() : TEXT("None"), OtherComp ? *OtherComp->GetName() : TEXT("None"));
    if (!OtherActor) return;

    // If player pawn, start following
    APlayerShip* PS = Cast<APlayerShip>(OtherActor);
    if (PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] Player entered aggro sphere: %s"), *PS->GetName());
        FollowTarget = PS;
        bFollowing = true;
    }
}

void AEnemyShip::OnAggroEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] OnAggroEndOverlap Other=%s Comp=%s"), OtherActor ? *OtherActor->GetName() : TEXT("None"), OtherComp ? *OtherComp->GetName() : TEXT("None"));
    if (!OtherActor) return;
    APlayerShip* PS = Cast<APlayerShip>(OtherActor);
    if (PS && PS == FollowTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] Player left aggro sphere: %s"), *PS->GetName());
        FollowTarget = nullptr;
        bFollowing = false;
        TargetLinearVelocity = FVector::ZeroVector;
    }
}
