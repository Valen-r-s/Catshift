// SpecialAttackComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"          // ECollisionResponse, ECC_*
#include "Components/ActorComponent.h"
#include "SpecialAttackComponent.generated.h"

class UAnimMontage;
class UUserWidget;
class UWidgetComponent;
class ACharacter;

#ifndef YOURPROJECT_API
#define YOURPROJECT_API
#endif

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOURPROJECT_API USpecialAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpecialAttackComponent();

	/** Duración REAL de la ventana de selección (ignora time dilation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Targeting")
	float TargetingDurationSeconds = 5.f;

	/** Time dilation global durante la ventana */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Targeting")
	float GlobalTimeDilationDuringTargeting = 0.2f;

	/** Daño por mini-impacto (cada objetivo durante la secuencia) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Damage")
	float SmallHitDamage = 20.f;

	/** Daño del impacto final (a todos los seleccionados que sigan vivos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Damage")
	float FinalBurstDamage = 80.f;

	/** Distancia máxima de dash (si usas LaunchCharacter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Dash")
	float DashDistance = 900.f;

	/** Velocidad del dash (LaunchCharacter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Dash")
	float DashSpeed = 4000.f;

	/** Radio para considerar “llegué” al objetivo (para validar SmallHit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Dash")
	float ApproachRadius = 180.f;

	/** (Debug) usar teleport en vez de Launch para validar rutas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Dash")
	bool bUseTeleportDash = false;

	/** Montajes AM_R1, AM_R2, AM_R3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|Montage")
	TArray<TSoftObjectPtr<UAnimMontage>> Montages;

	/** Nombre del WidgetComponent de marcador en enemigos. Solo este se toca. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialAttack|UI")
	FName EnemyWidgetComponentName = FName("TargetMarker");

	/* ==== API BP ==== */
	UFUNCTION(BlueprintCallable, Category = "SpecialAttack")
	void StartSpecialAttack();

	UFUNCTION(BlueprintCallable, Category = "SpecialAttack")
	void HandleClickUnderCursor();

	/** Notifies llamados desde el AnimBP */
	UFUNCTION(BlueprintCallable, Category = "SpecialAttack|Notifies")
	void Notify_DashBegin();

	UFUNCTION(BlueprintCallable, Category = "SpecialAttack|Notifies")
	void Notify_SmallHit();

	UFUNCTION(BlueprintCallable, Category = "SpecialAttack|Notifies")
	void Notify_Next();

	UFUNCTION(BlueprintCallable, Category = "SpecialAttack|Notifies")
	void Notify_FinalBurst();

	/** Getters para puertas lógicas de input */
	UFUNCTION(BlueprintPure, Category = "SpecialAttack")
	bool IsTargeting() const { return bIsTargeting; }

	UFUNCTION(BlueprintPure, Category = "SpecialAttack")
	bool IsExecuting() const { return bIsExecuting; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/* ==== Estado ==== */
	bool bIsTargeting = false;
	bool bIsExecuting = false;

	double TargetingStartRealSeconds = 0.0; // reloj real (FPlatformTime)

	int32 CurrentTargetIndex = INDEX_NONE;

	UPROPERTY()
	TArray<TWeakObjectPtr<ACharacter>> SelectedTargets;

	UPROPERTY()
	TArray<TWeakObjectPtr<ACharacter>> ValidTargetsThisRun;

	/* ==== Helpers ==== */
	ACharacter* GetOwnerCharacter() const;
	bool IsEnemyCharacter(AActor* Actor) const;

	/** Busca el WidgetComponent EXACTO cuyo nombre == EnemyWidgetComponentName. Si no existe, retorna null (no toca otros). */
	UWidgetComponent* FindEnemyMarker(ACharacter* Enemy) const;

	void ShowAllEnemyMarkers(bool bShow);
	void UpdateEnemyMarker(ACharacter* Enemy, bool bSelected, int32 OrderIndex);
	void ReindexAllMarkers();

	void EndTargeting();
	void RunNextOrFinish();
	void PlayRandomMontage();
	void DashToTarget(ACharacter* Target);
	void ApplySmallHitToTarget(ACharacter* Target);
	void ApplyFinalBurst();

	/** Durante dash: ignorar colisión con Pawn y que el movimiento ignore actores Enemy */
	void SetPassThroughEnemies(bool bEnable);
	void UpdateMoveIgnoreEnemies(bool bEnable);

	/* Backup de respuesta de colisión al canal Pawn */
	ECollisionResponse SavedPawnResponse = ECR_Block;
	bool bSavedPawnResponseValid = false;

	/* Lista de actores ignorados en movimiento (para restaurar) */
	TArray<TWeakObjectPtr<AActor>> MoveIgnoredList;
};
