#include "Weapons/Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(8.f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComp;

    CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 5.f;
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifeTime);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (ProjectileMovement) ProjectileMovement->StopMovementImmediately();

        UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
    }

    Destroy();
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[Projectile] OnOverlap called Other=%s Comp=%s"), OtherActor ? *OtherActor->GetName() : TEXT("None"), OtherComp ? *OtherComp->GetName() : TEXT("None"));
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (OtherComp->GetCollisionEnabled() == ECollisionEnabled::QueryOnly)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[Projectile] Overlap ignored: OtherComp is QueryOnly"));
            return;
        }

        if (ProjectileMovement) ProjectileMovement->StopMovementImmediately();
        UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
        Destroy();
    }
}
