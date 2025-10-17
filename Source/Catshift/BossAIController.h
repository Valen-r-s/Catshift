// BossAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossCharacter.h"
#include "BossAIController.generated.h"

UCLASS()
class CATSHIFT_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	ABossCharacter* Boss = nullptr;

	UPROPERTY()
	APawn* PlayerPawn = nullptr;

	// Ritmo de pensamiento para no decidir cada tick
	UPROPERTY(EditDefaultsOnly, Category = "BossAI")
	float DecisionInterval = 0.25f;

	// Mientras recupera, no ataca (Boss maneja PostAttackRecovery). Aquí sólo movemos/encaramos.
	UPROPERTY(EditDefaultsOnly, Category = "BossAI")
	float ChaseAcceptanceRadius = 200.f;

	// Mantenerse a esta distancia media cuando está buscando rango
	UPROPERTY(EditDefaultsOnly, Category = "BossAI")
	float IdealMidRange = 450.f;

	double NextThinkTime = 0.0;

	void EnsurePlayerRef();
	void DecideAndAct();
	void MoveToIdeal(const float Dist, const EBossAttack Attack);
};
