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

// Eventos para HUD
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldCooldownStart, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldCooldownEnd);

/**
 * Componente de Escudo plug&play (UE5.6).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CATSHIFT_API UShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComponent();

	/** Eventos de gameplay */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Events")
	FOnShieldSphereHit OnShieldSphereHit;

	/** Eventos para HUD */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Events")
	FOnShieldCooldownStart OnCooldownStart;

	UPROPERTY(BlueprintAssignable, Category = "Shield|Events")
	FOnShieldCooldownEnd OnCooldownEnd;

	/** Estado */
	UFUNCTION(BlueprintPure, Category = "Shield")
	bool IsShieldActive() const { return bShieldActive; }

	/** Cooldown (para HUD) */
	UFUNCTION(BlueprintPure, Category = "Shield|Cooldown")
	bool IsOnCooldown() const { return bOnCooldown; }

	UFUNCTION(BlueprintPure, Category = "Shield|Cooldown")
	float GetCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Shield|Cooldown")
	float GetCooldownDuration() const { return CooldownSeconds; }

	/** 0..1 = (tiempo restante / duración). Útil para barras o máscaras radiales. */
	UFUNCTION(BlueprintPure, Category = "Shield|Cooldown")
	float GetCooldownRatio() const;

	/* ==================== CONFIG ==================== */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Collision")
	bool bUsePersistentCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Collision", meta = (ClampMin = "0.0"))
	float SphereRadius = 220.f;

	/** Máximo de uso (0 = mientras se mantenga). DEFAULT: 5 s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Timing", meta = (ClampMin = "0.0"))
	float MaxHoldSeconds = 5.f;

	/** Cooldown tras terminar. DEFAULT: 6 s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Timing", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	UStaticMesh* ShieldStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	bool bAutoScaleVisual = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	bool bAlignBottomToFloor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Visual")
	float VisualZOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Audio")
	USoundBase* ShieldLoopSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Audio", meta = (ClampMin = "0.0"))
	float ShieldLoopVolume = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Shield|Audio")
	void SetShieldLoopSound(USoundBase* InSound, float InVolume = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Shield|Visual")
	void SyncVisualToRadius();

protected:
	virtual void BeginPlay() override;

	void BindInput();

	UFUNCTION() void StartShield();
	UFUNCTION() void EndShield();
	void        ForceEndShield();

	void DoSphereCast();

	void LockControls(bool bLock);

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

	/* Captura de input mientras está activo */
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
