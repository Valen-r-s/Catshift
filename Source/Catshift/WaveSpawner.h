// WaveSpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "WaveSpawner.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(ClassGroup = (Spawning), meta = (DisplayName = "Wave Spawner"))
class CATSHIFT_API AWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWaveSpawner();

protected:
	virtual void BeginPlay() override;

	/* =======================
	   Componentes
	======================= */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	/** Volumen donde se generan los enemigos */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* SpawnArea;

	/** Zona opcional para iniciar oleadas por overlap con el jugador */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* StartZone;

	/* =======================
	   Parámetros de spawn / oleadas
	======================= */

	/** Primer tipo de enemigo (BP o clase derivada de APawn/Character) */
	UPROPERTY(EditAnywhere, Category = "Wave|Classes")
	TSubclassOf<APawn> EnemyAClass;

	/** Segundo tipo de enemigo */
	UPROPERTY(EditAnywhere, Category = "Wave|Classes")
	TSubclassOf<APawn> EnemyBClass;

	/** Cantidad de A por oleada */
	UPROPERTY(EditAnywhere, Category = "Wave|Counts", meta = (ClampMin = "0"))
	int32 CountA = 3;

	/** Cantidad de B por oleada */
	UPROPERTY(EditAnywhere, Category = "Wave|Counts", meta = (ClampMin = "0"))
	int32 CountB = 2;

	/** Barajar el orden de aparición dentro de la oleada */
	UPROPERTY(EditAnywhere, Category = "Wave|Counts")
	bool bShuffleQueue = true;

	/** Tiempo entre spawns dentro de una misma oleada */
	UPROPERTY(EditAnywhere, Category = "Wave|Timing", meta = (ClampMin = "0.0"))
	float SpawnInterval = 0.25f;

	/** Espera entre oleadas (cuando la oleada terminó y no quedan vivos) */
	UPROPERTY(EditAnywhere, Category = "Wave|Timing", meta = (ClampMin = "0.0"))
	float DelayBetweenWaves = 5.0f;

	/** Número total de oleadas (0 = infinito) */
	UPROPERTY(EditAnywhere, Category = "Wave|Timing", meta = (ClampMin = "0"))
	int32 TotalWaves = 1;

	/** Arranca automáticamente al BeginPlay */
	UPROPERTY(EditAnywhere, Category = "Wave|Start")
	bool bAutoStartOnBeginPlay = true;

	/** Arranca cuando un Pawn entra a StartZone */
	UPROPERTY(EditAnywhere, Category = "Wave|Start")
	bool bAutoStartOnOverlap = false;

	/** Ejecutar sólo una vez: al terminar las TotalWaves no vuelve a iniciar */
	UPROPERTY(EditAnywhere, Category = "Wave|Start")
	bool bOnlyOnce = false;

	/** Límite de enemigos simultáneos (0 = sin límite) */
	UPROPERTY(EditAnywhere, Category = "Wave|Limits", meta = (ClampMin = "0"))
	int32 MaxSimultaneous = 0;

	/** Proyectar el punto de spawn dentro de la NavMesh */
	UPROPERTY(EditAnywhere, Category = "Wave|Navigation")
	bool bUseNavmeshProjection = true;

	/** Extensión usada para proyectar en NavMesh */
	UPROPERTY(EditAnywhere, Category = "Wave|Navigation")
	FVector NavProjectExtent = FVector(50.f, 50.f, 200.f);

	/* =======================
	   Fin de oleada / recompensas
	======================= */

	/** Si es true, cuando mueren todos los enemigos se DETIENE el spawner. */
	UPROPERTY(EditAnywhere, Category = "Wave|End")
	bool bStopWhenCleared = true;

	/** Clase Blueprint a generar como recompensa al limpiar (puede ser cualquier Actor). */
	UPROPERTY(EditAnywhere, Category = "Wave|Rewards")
	TSubclassOf<AActor> RewardClass;

	/** Cantidad de recompensas a generar al limpiar. */
	UPROPERTY(EditAnywhere, Category = "Wave|Rewards", meta = (ClampMin = "0"))
	int32 RewardCount = 0;

	/* =======================
	   Estado interno (debug)
	======================= */
	UPROPERTY(VisibleAnywhere, Category = "Wave|Debug")
	int32 AliveCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Wave|Debug")
	int32 RemainingToSpawn = 0;

	UPROPERTY(VisibleAnywhere, Category = "Wave|Debug")
	int32 WaveIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "Wave|Debug")
	bool bStarted = false;

	/** Cola de tipos a spawnear: 0 = A, 1 = B */
	TArray<uint8> SpawnQueue;

	/** Índice actual en la cola */
	int32 QueueIndex = 0;

	/** Guard para no manejar dos veces el “wave cleared” */
	bool bWaveClearHandled = false;

	/** Timers */
	FTimerHandle TimerHandle_SpawnTick;
	FTimerHandle TimerHandle_WaveDelay;

	/* =======================
	   Lógica
	======================= */

	/** Construye la cola (A/B) para la oleada actual */
	void BuildSpawnQueue();

	/** Arranca la secuencia de oleadas */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWaves();

	/** Inicia la siguiente oleada o cierra si corresponde */
	UFUNCTION()
	void StartNextWave();

	/** Intenta spawnear un actor y reprograma el siguiente tick */
	UFUNCTION()
	void SpawnTick();

	/** Callback cuando un enemigo spawneado se destruye */
	UFUNCTION()
	void OnEnemyDestroyed(AActor* DestroyedActor);

	/** Si bAutoStartOnOverlap, arranca al detectar Pawn entrando */
	UFUNCTION()
	void OnStartZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/** Devuelve un Transform aleatorio dentro de SpawnArea (con yaw random) proyectado a NavMesh si procede */
	FTransform MakeRandomSpawnTransform() const;

	/** Transform del centro exacto del SpawnArea (con Z forzada a 0) */
	FTransform MakeSpawnAreaCenterTransform() const;

	/** Maneja el fin de oleada (todos muertos) ? recompensa + decidir si parar o seguir */
	void HandleWaveCleared();

	/** Spawnea recompensas si están configuradas (en el centro del SpawnArea) */
	void SpawnRewards();
};
