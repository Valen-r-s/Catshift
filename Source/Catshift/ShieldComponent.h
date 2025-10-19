// ShieldComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/InputComponent.h"
#include "ShieldComponent.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UCharacterMovementComponent;
class ACharacter;
class APlayerController;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldSphereHit, const TArray<AActor*>&, HitActors);

/**
 * Componente de Escudo plug&play (UE5.6).
 * - No requiere modificar tu personaje.
 * - Acción "Shield" (Space): presiona para activar, suelta para desactivar.
 * - Invulnerabilidad y bloqueo de acciones/movimiento.
 * - Sphere cast inicial + colisión persistente opcional.
 * - Sonido en loop mientras esté activo.
 * - Autoescalado de malla visual para coincidir con SphereRadius.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CATSHIFT_API UShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComponent();

	/** Eventos */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Events")
	FOnShieldSphereHit OnShieldSphereHit;

	/** Estado */
	UFUNCTION(BlueprintPure, Category = "Shield")
	bool IsShieldActive() const { return bShieldActive; }

	/* ==================== CONFIG ==================== */

	/** Usar una esfera de colisión persistente mientras dure el escudo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Collision")
	bool bUsePersistentCollision = false;

	/** Radio del sphere cast y de la esfera persistente (si se usa). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Collision", meta = (ClampMin = "0.0"))
	float SphereRadius = 220.f;

	/** Duración máxima (0 = mientras se mantenga presionado). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Timing", meta = (ClampMin = "0.0"))
	float MaxHoldSeconds = 0.f;

	/** Cooldown tras terminar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Timing", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 0.5f;

	/** Debug de la esfera al activar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Debug")
	bool bDrawDebug = false;

	/** Malla visual opcional del escudo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	UStaticMesh* ShieldStaticMesh = nullptr;

	/** Escalar automáticamente la malla para que su radio visual == SphereRadius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	bool bAutoScaleVisual = true;

	/** Alinear el borde inferior de la malla al suelo (respecto a la cápsula). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	bool bAlignBottomToFloor = true;

	/** Offset adicional en Z para afinar visualmente. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	float VisualZOffset = 0.f;

	/** Sonido en loop mientras el escudo esté activo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Audio")
	USoundBase* ShieldLoopSound = nullptr;

	/** Volumen del loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Audio", meta = (ClampMin = "0.0"))
	float ShieldLoopVolume = 1.0f;

	/** Setear sonido/volumen por BP. */
	UFUNCTION(BlueprintCallable, Category = "Shield|Audio")
	void SetShieldLoopSound(USoundBase* InSound, float InVolume = 1.f);

	/** Recalcular escala/offset visual (llámalo si cambias SphereRadius en runtime). */
	UFUNCTION(BlueprintCallable, Category = "Shield|Visual")
	void SyncVisualToRadius();

protected:
	virtual void BeginPlay() override;

	/** Vincular input "Shield" sin tocar el personaje. */
	void BindInput();

	/** Inicia / termina el escudo. */
	UFUNCTION() void StartShield();
	UFUNCTION() void EndShield();
	void        ForceEndShield();

	/** Sphere cast inicial. */
	void DoSphereCast();

	/** Bloqueo/desbloqueo de controles. */
	void LockControls(bool bLock);

	/** Crear / destruir componentes runtime. */
	void SpawnRuntimeComponents();
	void DestroyRuntimeComponents();

	/* ==================== AUDIO ==================== */
	void StartLoopAudio();
	void StopLoopAudio();
	UFUNCTION() void OnShieldLoopFinished();

private:
	/* Propietarios / control */
	TWeakObjectPtr<ACharacter>         OwnerCharacter;
	TWeakObjectPtr<APlayerController>  PC;

	/* Colisión/visual runtime */
	UPROPERTY(Transient) USphereComponent* ShieldSphere = nullptr;
	UPROPERTY(Transient) UStaticMeshComponent* ShieldFX = nullptr;

	/* Capturador de input temporal mientras el escudo está activo. */
	UPROPERTY(Transient) UInputComponent* ShieldInputCapture = nullptr;

	/* Audio loop runtime */
	UPROPERTY(Transient) UAudioComponent* ShieldLoopAC = nullptr;

	/* Timers / estado */
	FTimerHandle TH_BindInputRetry;
	FTimerHandle TH_MaxHold;
	FTimerHandle TH_Cooldown;

	bool bShieldActive = false;
	bool bOnCooldown = false;
	bool bInputBound = false;
};
