// EnemyBase.cpp
#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Crear hitbox y adjuntarla al Mesh (luego en el BP la pegas a un socket del arma/mano)
	AttackHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	AttackHitbox->SetupAttachment(GetMesh());
	AttackHitbox->SetBoxExtent(FVector(8.f, 12.f, 12.f));

	// Colisión: apagada por defecto; solo queremos Overlap a Pawn cuando se encienda
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitbox->SetGenerateOverlapEvents(true);
	AttackHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	AttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Tag útil por si luego quieres buscarla por tag
	AttackHitbox->ComponentTags.AddUnique(FName("AttackHitbox"));
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (AttackHitbox)
	{
		AttackHitbox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnAttackOverlapBegin);
	}
}

void AEnemyBase::EnableAttackHitbox()
{
	// Nueva ventana de golpe: resetea anti-multigolpe
	AlreadyHit.Empty();

	if (AttackHitbox)
	{
		AttackHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Overlap ON
	}
}

void AEnemyBase::DisableAttackHitbox()
{
	if (AttackHitbox)
	{
		AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Overlap OFF
	}
}

void AEnemyBase::OnAttackOverlapBegin(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!OtherActor || OtherActor == this) return;

	// Solo cuando la ventana está activa
	if (!AttackHitbox || AttackHitbox->GetCollisionEnabled() == ECollisionEnabled::NoCollision) return;

	// Evitar múltiples impactos en la misma ventana
	if (AlreadyHit.Contains(OtherActor)) return;

	// Filtra para golpear únicamente al jugador: pawn controlado por jugador
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (!Pawn->IsPlayerControlled()) return;

		UGameplayStatics::ApplyDamage(Pawn, AttackDamage, GetController(), this, UDamageType::StaticClass());
		AlreadyHit.Add(Pawn);
	}
}
