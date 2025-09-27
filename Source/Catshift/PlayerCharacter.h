// PlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/EngineTypes.h"              // FComponentReference
#include "PlayerCharacter.generated.h"

// Cambia YOURPROJECT_API por el macro real de tu módulo si ya existe (p.ej. MYGAME_API)
#ifndef YOURPROJECT_API
#define YOURPROJECT_API
#endif

class UUserWidget;
class UBoxComponent;

UCLASS()
class YOURPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	/* =================== STATS =================== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0"))
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0"))
	float CurrentEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Regen", meta = (ClampMin = "0.0"))
	float EnergyRegenPerSec = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Regen")
	bool bAutoRegenEnergy = true;

	/* (Opcional) UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI")
	UUserWidget* HUDWidgetInstance = nullptr;

	/* =================== COMBATE (MELEE) =================== */

	/** Hitbox por defecto creada en C++ (se usa si no eliges ninguna en el BP). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* MeleeHitbox_Default;

	/**
	 * Selector de componente en el BP:
	 * Elige aquí tu Box Collision (hija de la espada) y el código la usará como hitbox activa.
	 * En Details verás un picker de componentes. Solo mostrará componentes tipo BoxComponent.
	 */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowedClasses = "BoxComponent"))
	FComponentReference MeleeHitboxRef;

	/** Puntero que usará la lógica siempre (BP si hay, si no la default). */
	UPROPERTY(Transient)
	UBoxComponent* ActiveMeleeHitbox = nullptr;

	/** Daño por golpe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeDamage = 20.f;

	/** Tamaño inicial de la caja default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector MeleeBoxExtent = FVector(8.f, 40.f, 20.f);

	/** Abrir/cerrar ventana de golpe (llámalo desde notifies del montage). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartMeleeWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndMeleeWindow();

	/* =================== API STATS (helpers) =================== */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyDamageAmount(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool TryConsumeEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddEnergy(float Amount);

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const { return (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetEnergyPercent() const { return (MaxEnergy > 0.f) ? (CurrentEnergy / MaxEnergy) : 0.f; }

protected:
	/** Overlap de la hitbox activa */
	UFUNCTION()
	void OnMeleeHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void RegenerateEnergy(float DeltaTime);

	/** Evita multi-golpes en un mismo swing */
	TSet<TWeakObjectPtr<AActor>> AlreadyHit;
};
