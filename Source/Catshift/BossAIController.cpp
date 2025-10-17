// BossAIController.cpp
#include "BossAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABossAIController::ABossAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Boss = Cast<ABossCharacter>(InPawn);
}

void ABossAIController::EnsurePlayerRef()
{
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	}
}

void ABossAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Boss) return;
	if (Boss->bInIntro) return;

	EnsurePlayerRef();
	if (!PlayerPawn) return;

	// Ritmo de decisiones
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextThinkTime) return;
	NextThinkTime = Now + DecisionInterval;

	DecideAndAct();
}

void ABossAIController::DecideAndAct()
{
	const float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), Boss->GetActorLocation());

	// Mientras ataca, sólo mirar al jugador
	if (Boss->bIsAttacking)
	{
		const FRotator Look = UKismetMathLibrary::FindLookAtRotation(Boss->GetActorLocation(), PlayerPawn->GetActorLocation());
		Boss->SetActorRotation(FRotator(0, Look.Yaw, 0));
		return;
	}

	// Si está en recuperación, no atacar: muévete para buscar rango/posición
	if (Boss->IsRecovering())
	{
		MoveToActor(PlayerPawn, ChaseAcceptanceRadius);
		return;
	}

	// Elegir ataque respetando distancias/CDs
	EBossAttack Chosen = Boss->RandomWeightedAttack(Dist, PlayerPawn);

	if (Chosen == EBossAttack::None)
	{
		// Sin ataque viable -> acercarse/posicionarse
		MoveToActor(PlayerPawn, ChaseAcceptanceRadius);
		return;
	}

	// Si no está en rango ideal, primero reposiciona
	if (Dist < Boss->IdealMinDistance(Chosen) || Dist > Boss->IdealMaxDistance(Chosen))
	{
		MoveToIdeal(Dist, Chosen);
		return;
	}

	// En rango -> ataca
	StopMovement();
	Boss->PlayAttack(Chosen, PlayerPawn);
}

void ABossAIController::MoveToIdeal(const float Dist, const EBossAttack Attack)
{
	// Objetivo: quedarse en el medio del rango ideal del ataque
	float TargetRadius = FMath::Clamp((Boss->IdealMinDistance(Attack) + Boss->IdealMaxDistance(Attack)) * 0.5f, 200.f, 800.f);
	if (Attack == EBossAttack::Swipe) // para carga, un poco más lejos ayuda
	{
		TargetRadius = FMath::Clamp(TargetRadius + 150.f, 300.f, 1200.f);
	}

	const FVector ToBoss = (Boss->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
	const FVector Goal = PlayerPawn->GetActorLocation() + ToBoss * TargetRadius;

	MoveToLocation(Goal, ChaseAcceptanceRadius * 0.6f);
}
