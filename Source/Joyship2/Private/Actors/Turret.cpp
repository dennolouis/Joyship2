#include "Actors/Turret.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/HealthComponent.h"

ATurret::ATurret()
{
    PrimaryActorTick.bCanEverTick = true;

    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    Trigger->SetCollisionProfileName(TEXT("Trigger"));
    Trigger->SetGenerateOverlapEvents(true);
    RootComponent = Trigger;

    AimMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimMesh"));
    AimMesh->SetupAttachment(RootComponent);

    Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(AimMesh);

    // Health
    UHealthComponent* HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ATurret::BeginPlay()
{
    Super::BeginPlay();

    if (Trigger)
    {
        Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATurret::OnTriggerBeginOverlap);
        Trigger->OnComponentEndOverlap.AddDynamic(this, &ATurret::OnTriggerEndOverlap);
    }
}

void ATurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetPawn)
    {
        // reset timers
        LookAccum = 0.f;
        FireAccum = 0.f;
        return;
    }

    FVector TargetLoc = TargetPawn->GetActorLocation();
    FVector AimLoc = AimMesh->GetComponentLocation();
    FVector ToTarget = (TargetLoc - AimLoc).GetSafeNormal();

    // Current forward of AimMesh
    FVector AimForward = AimMesh->GetForwardVector();

    // Compute desired rotation to look at target (only yaw and pitch if needed)
    FRotator CurrentRot = AimMesh->GetComponentRotation();
    FRotator TargetRot = ToTarget.Rotation();

    // Interpolate rotation
    FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaTime, TurnSpeed);
    AimMesh->SetWorldRotation(NewRot);

    // Check angle between forward and target
    float AngleDeg = FMath::RadiansToDegrees(acosf(FVector::DotProduct(AimMesh->GetForwardVector(), ToTarget)));
    if (AngleDeg <= AimToleranceDegrees)
    {
        LookAccum += DeltaTime;
        if (LookAccum >= LookTimeRequired)
        {
            FireAccum += DeltaTime;
            if (FireAccum >= FireInterval)
            {
                // Fire
                if (ProjectileClass && Muzzle)
                {
                    UWorld* W = GetWorld();
                    if (W)
                    {
                        FActorSpawnParameters Params;
                        Params.Owner = this;
                        FVector SpawnLoc = Muzzle->GetComponentLocation();
                        FRotator SpawnRot = AimMesh->GetComponentRotation();
                        AActor* P = W->SpawnActor<AActor>(ProjectileClass, SpawnLoc, SpawnRot, Params);
                        if (P)
                        {
                            UProjectileMovementComponent* PM = P->FindComponentByClass<UProjectileMovementComponent>();
                            if (PM)
                            {
                                PM->Velocity = AimMesh->GetForwardVector() * PM->InitialSpeed;
                            }
                        }
                    }
                }
                FireAccum = 0.f;
            }
        }
    }
    else
    {
        LookAccum = 0.f;
        FireAccum = 0.f;
    }
}

void ATurret::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    APawn* P = Cast<APawn>(OtherActor);
    if (P)
    {
        TargetPawn = P;
    }
}

void ATurret::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* P = Cast<APawn>(OtherActor);
    if (P && P == TargetPawn)
    {
        TargetPawn = nullptr;
    }
}
