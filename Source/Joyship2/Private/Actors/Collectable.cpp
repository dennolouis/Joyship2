#include "Actors/Collectable.h"
#include "Components/StaticMeshComponent.h"
#include "Pawns/PlayerShip.h"


ACollectable::ACollectable()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    SetRootComponent(MeshComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Start with ticking disabled; the Blueprint should enable/disable magnet by calling ActivateMagnet/DeactivateMagnet
    SetActorTickEnabled(false);
}

void ACollectable::BeginPlay()
{
    Super::BeginPlay();
}

void ACollectable::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCollected) return;

    // Use per-frame smoothing for smoother movement. Magnet activation enables ticking.
    if (!CachedPlayer) return;
    float StepTime = DeltaTime;

    FVector ToPlayer = CachedPlayer->GetActorLocation() - GetActorLocation();
    float DistSq = ToPlayer.SizeSquared();

    // If within collect distance and auto-collect is enabled, collect immediately
    if (bAllowRangeCollect && DistSq <= CollectDistance * CollectDistance)
    {
        APlayerShip* PS = Cast<APlayerShip>(CachedPlayer);
        Collect(PS);
        return;
    }
    // If within magnet radius, smoothly move toward the player each frame
    if (DistSq <= MagnetRadius * MagnetRadius)
    {
        float Dist = FMath::Sqrt(DistSq);
        if (Dist > KINDA_SMALL_NUMBER)
        {
            FVector Dir = ToPlayer / Dist;

            // Determine stop distance: if auto-range-collect is disabled, stop at CollectDistance
            float StopDistance = bAllowRangeCollect ? 0.f : CollectDistance;
            FVector DesiredLoc = CachedPlayer->GetActorLocation() - Dir * StopDistance;

            // Smoothly interpolate actor location toward desired location. Use sweep=false to avoid collision jitter.
            FVector NewLoc = FMath::VInterpTo(GetActorLocation(), DesiredLoc, DeltaTime, AttractionSpeed);
            SetActorLocation(NewLoc, false);
        }
    }
}

// Note: overlap functions are intended to be implemented in Blueprints by calling
// ActivateMagnet/DeactivateMagnet and Collect. Definitions removed so Blueprint
// collision handlers can be used instead.

void ACollectable::ActivateMagnet(APawn* Pawn)
{
    if (bCollected) return;
    if (!Pawn) return;
    CachedPlayer = Pawn;
    TickAccumulator = 0.f;
    SetActorTickEnabled(true);
    UE_LOG(LogTemp, Warning, TEXT("[Collectable] ActivateMagnet called for %s"), Pawn ? *Pawn->GetName() : TEXT("None"));

    // Start a fail-safe timer to ensure collection is attempted after MagnetFailSafeDelay
    if (MagnetFailSafeDelay > 0.f)
    {
        GetWorldTimerManager().ClearTimer(MagnetFailSafeTimer);
        GetWorldTimerManager().SetTimer(MagnetFailSafeTimer, this, &ACollectable::AttemptFailSafeCollect, MagnetFailSafeDelay, false);
    }
}

void ACollectable::DeactivateMagnet(APawn* Pawn)
{
    if (!Pawn) return;
    if (Pawn == CachedPlayer)
    {
        CachedPlayer = nullptr;
        SetActorTickEnabled(false);
        UE_LOG(LogTemp, Warning, TEXT("[Collectable] DeactivateMagnet called for %s"), Pawn ? *Pawn->GetName() : TEXT("None"));
        // Clear fail-safe timer
        GetWorldTimerManager().ClearTimer(MagnetFailSafeTimer);
    }
}

void ACollectable::Collect(APlayerShip* Collector)
{
    if (bCollected) return;

    bCollected = true;

    UE_LOG(LogTemp, Warning, TEXT("[Collectable] Collected by %s"), Collector ? *Collector->GetName() : TEXT("None"));

    // Trigger Blueprint hook
    OnCollected(Collector);

    // Default: destroy the actor
    Destroy();
}

void ACollectable::AttemptFailSafeCollect()
{
    if (bCollected) return;
    if (!CachedPlayer) return;
    APlayerShip* PS = Cast<APlayerShip>(CachedPlayer);
    UE_LOG(LogTemp, Warning, TEXT("[Collectable] AttemptFailSafeCollect called for %s"), CachedPlayer ? *CachedPlayer->GetName() : TEXT("None"));
    Collect(PS);
}
