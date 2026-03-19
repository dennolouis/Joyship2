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
    // Use a simple overlap profile so it reliably generates overlaps for pawns
    AggroSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

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

void AEnemyShip::StartFollowing(AActor* Target)
{
    if (!Target) return;
    FollowTarget = Target;
    bFollowing = true;
    UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] StartFollowing called for %s"), *Target->GetName());
}

void AEnemyShip::StopFollowing()
{
    FollowTarget = nullptr;
    bFollowing = false;
    TargetLinearVelocity = FVector::ZeroVector;
    UE_LOG(LogTemp, Warning, TEXT("[EnemyShip] StopFollowing called"));
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

            // Compute rotation so that the actor's Up vector points toward the target direction
            FVector CurrentUp = GetActorUpVector();
            FVector DesiredUp = Dir;

            // Build quaternions from the two directions and slerp between them for smooth turning
            FQuat CurQuat = GetActorQuat();
            FQuat FromTo = FQuat::FindBetweenNormals(CurrentUp, DesiredUp);
            FQuat TargetQuat = FromTo * CurQuat;

            // Slerp toward target quaternion
            FQuat NewQuat = FQuat::Slerp(CurQuat, TargetQuat, FMath::Clamp(RotationSpeed * DeltaTime, 0.f, 1.f));
            FRotator NewRot = NewQuat.Rotator();
            // Apply the full rotation so the actor's Up vector aligns with the desired direction
            SetActorRotation(NewRot);

            // Move forward along actor up vector (matching ABaseShip thrust axis)
            // Project movement onto the XY plane so the enemy does not move vertically
            FVector MoveDir = GetActorUpVector();
            MoveDir.Z = 0.f;
            if (MoveDir.SizeSquared() > KINDA_SMALL_NUMBER)
            {
                MoveDir = MoveDir.GetSafeNormal();
                TargetLinearVelocity = MoveDir * ThrustForce;
            }
            else
            {
                TargetLinearVelocity = FVector::ZeroVector;
            }
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
