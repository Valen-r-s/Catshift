// MeteorActors.cpp
#include "MeteorActors.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

/////////////////////////////
//   AMeteorProjectile
/////////////////////////////
AMeteorProjectile::AMeteorProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->SetSimulatePhysics(true);
	Mesh->OnComponentHit.AddDynamic(this, &AMeteorProjectile::OnMeshHit);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AMeteorProjectile::BeginPlay()
{
	Super::BeginPlay();

	FVector Dir = FVector(0, 0, -1);
	if (!TargetGround.IsNearlyZero())
	{
		Dir = (TargetGround - GetActorLocation()).GetSafeNormal();
	}
	Mesh->SetPhysicsLinearVelocity(Dir * GravitySpeed);
}

void AMeteorProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMeteorProjectile::OnMeshHit(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, FVector, const FHitResult& Hit)
{
	if (bImpacted) return;
	bImpacted = true;

	UGameplayStatics::ApplyRadialDamage(
		this, ExplosionDamage, Hit.ImpactPoint, ExplosionRadius,
		nullptr, TArray<AActor*>(), this, nullptr, true);

	// Sonido de explosión
	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, Hit.ImpactPoint);
	}

	SetLifeSpan(LifeAfterImpact);
}

/////////////////////////////
//     AMeteorWarning
/////////////////////////////
AMeteorWarning::AMeteorWarning()
{
	PrimaryActorTick.bCanEverTick = false;

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	SetRootComponent(Decal);
	Decal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // plano al suelo
	Decal->DecalSize = FVector(64.f, Radius, Radius);
	// No hay colisión en UDecalComponent
}

void AMeteorWarning::BeginPlay()
{
	Super::BeginPlay();

	if (WarningMaterial)
	{
		Decal->SetDecalMaterial(WarningMaterial);
	}
	Decal->DecalSize = FVector(64.f, Radius, Radius);

	SetLifeSpan(WarningTime + 0.1f);
}
