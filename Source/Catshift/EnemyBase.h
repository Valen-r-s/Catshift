// EnemyBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UBoxComponent;

UCLASS()
class AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

	/** Hitbox del arma / mano (ajústala al socket en el BP) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* AttackHitbox;

	/** Daño base del ataque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 20.f;

	/** Anti-multigolpe dentro de la misma ventana */
	TSet<TWeakObjectPtr<AActor>> AlreadyHit;

	/** Overlap de la hitbox */
	UFUNCTION()
	void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:
	/** Llamar desde notify de inicio (AnimNotify_EnableAttackHitbox) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableAttackHitbox();

	/** Llamar desde notify de fin (AnimNotify_DisableAttackHitbox) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DisableAttackHitbox();

	/** (Opcional) Cambiar daño desde BP o IA */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetAttackDamage(float NewDamage) { AttackDamage = FMath::Max(0.f, NewDamage); }

	/** (Opcional) Limpia la lista de golpeados si quieres llamarlo desde otro lado */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearAlreadyHit() { AlreadyHit.Empty(); }
};
