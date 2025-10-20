// WaveSpawner.cpp

#include "WaveSpawner.h"
#include "Components/BoxComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

AWaveSpawner::AWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Caja de aparición
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(Root);
	SpawnArea->SetBoxExtent(FVector(400.f, 400.f, 100.f)); // ajusta a gusto
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision); // guía visual

	// Zona de inicio por overlap (opcional)
	StartZone = CreateDefaultSubobject<UBoxComponent>(TEXT("StartZone"));
	StartZone->SetupAttachment(Root);
	StartZone->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	StartZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StartZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	StartZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	StartZone->OnComponentBeginOverlap.AddDynamic(this, &AWaveSpawner::OnStartZoneBeginOverlap);
}

void AWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartOnBeginPlay)
	{
		StartWaves();
	}
}

void AWaveSpawner::OnStartZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoStartOnOverlap || bStarted || !OtherActor)
	{
		return;
	}

	// Si es un Pawn (jugador o IA) lo tomamos como disparador
	if (Cast<APawn>(OtherActor))
	{
		StartWaves();
	}
}

void AWaveSpawner::StartWaves()
{
	if (bStarted) return;

	bStarted = true;
	WaveIndex = 0;
	bWaveClearHandled = false;

	// Si TotalWaves == 0 -> infinito; si > 0, se respetará el conteo.
	StartNextWave();
}

void AWaveSpawner::BuildSpawnQueue()
{
	SpawnQueue.Reset();
	const int32 SafeA = FMath::Max(0, CountA);
	const int32 SafeB = FMath::Max(0, CountB);
	const int32 TotalCount = SafeA + SafeB;

	if (TotalCount <= 0)
	{
		RemainingToSpawn = 0;
		QueueIndex = 0;
		return;
	}

	SpawnQueue.Reserve(TotalCount);

	for (int32 i = 0; i < SafeA; ++i) { SpawnQueue.Add(0); } // Tipo A
	for (int32 i = 0; i < SafeB; ++i) { SpawnQueue.Add(1); } // Tipo B

	// Barajar sin dependencias: Fisher–Yates
	if (bShuffleQueue && SpawnQueue.Num() > 1)
	{
		for (int32 i = SpawnQueue.Num() - 1; i > 0; --i)
		{
			const int32 j = FMath::RandRange(0, i);
			SpawnQueue.Swap(i, j);
		}
	}

	QueueIndex = 0;
	RemainingToSpawn = SpawnQueue.Num();
}

void AWaveSpawner::StartNextWave()
{
	// ¿Hemos terminado?
	if (TotalWaves > 0 && WaveIndex >= TotalWaves)
	{
		if (bOnlyOnce) { return; }
		else { WaveIndex = 0; } // ciclo infinito
	}

	bWaveClearHandled = false; // reset del guard

	// Prepara la cola
	BuildSpawnQueue();

	// Si esta oleada no tiene nada que spawnear
	if (RemainingToSpawn <= 0)
	{
		// Si además no hay vivos ? se considera “cleared”
		if (AliveCount <= 0)
		{
			HandleWaveCleared();
			return;
		}

		// Si hay vivos de antes, esperar a que mueran
		GetWorldTimerManager().SetTimer(TimerHandle_SpawnTick, this, &AWaveSpawner::SpawnTick, SpawnInterval, false);
		return;
	}

	// Arranca el ciclo de aparición (limpia timers previos)
	GetWorldTimerManager().ClearTimer(TimerHandle_SpawnTick);
	SpawnTick();
}

void AWaveSpawner::SpawnTick()
{
	// Condición de spawn: queda algo por generar y (no hay límite o el vivo actual está por debajo del límite)
	const bool bCanSpawnMore = (RemainingToSpawn > 0) && (MaxSimultaneous == 0 || AliveCount < MaxSimultaneous);

	if (bCanSpawnMore)
	{
		// Toma el tipo
		const uint8 Type = SpawnQueue.IsValidIndex(QueueIndex) ? SpawnQueue[QueueIndex] : 0;
		++QueueIndex;
		--RemainingToSpawn;

		// Clase a spawnear
		TSubclassOf<APawn> ClassToSpawn = (Type == 0) ? EnemyAClass : EnemyBClass;
		if (!ClassToSpawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("WaveSpawner: Clase de enemigo %s no asignada."),
				(Type == 0) ? TEXT("A") : TEXT("B"));
		}
		else
		{
			const FTransform SpawnTM = MakeRandomSpawnTransform();

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			APawn* Spawned = GetWorld()->SpawnActor<APawn>(ClassToSpawn, SpawnTM, Params);
			if (Spawned)
			{
				++AliveCount;

				// Asegura AIController aunque el BP no esté bien configurado
				if (!Spawned->GetController())
				{
					Spawned->SpawnDefaultController();
				}

				Spawned->OnDestroyed.AddDynamic(this, &AWaveSpawner::OnEnemyDestroyed);
			}
		}

		// Programa el próximo intento de spawn
		GetWorldTimerManager().SetTimer(TimerHandle_SpawnTick, this, &AWaveSpawner::SpawnTick, SpawnInterval, false);
		return;
	}

	// Si ya no queda nada por spawnear, esperamos a que mueran los vivos para cerrar la oleada
	if (RemainingToSpawn <= 0)
	{
		if (AliveCount <= 0)
		{
			HandleWaveCleared();
			return;
		}

		// Aún quedan vivos -> reintenta luego para comprobar finalización
		GetWorldTimerManager().SetTimer(TimerHandle_SpawnTick, this, &AWaveSpawner::SpawnTick, SpawnInterval, false);
		return;
	}

	// Hay límite simultáneo y estamos en cap: reintentar tras un intervalo
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnTick, this, &AWaveSpawner::SpawnTick, SpawnInterval, false);
}

void AWaveSpawner::OnEnemyDestroyed(AActor* DestroyedActor)
{
	AliveCount = FMath::Max(0, AliveCount - 1);

	// Si ya no queda cola y no queda nadie vivo, cerrar y programar siguiente paso
	if (RemainingToSpawn <= 0 && AliveCount <= 0)
	{
		HandleWaveCleared();
	}
}

FTransform AWaveSpawner::MakeRandomSpawnTransform() const
{
	// Caja en espacio mundo
	const FTransform AreaTM = SpawnArea->GetComponentTransform();
	const FVector Extent = SpawnArea->GetScaledBoxExtent();

	// Punto local aleatorio (ignoramos Z del box)
	const FVector LocalRand(
		FMath::FRandRange(-Extent.X, Extent.X),
		FMath::FRandRange(-Extent.Y, Extent.Y),
		0.f // para no elevarlo dentro del box
	);

	// A mundo
	FVector WorldPos = AreaTM.TransformPosition(LocalRand);

	// Proyección a NavMesh (opcional)
	if (bUseNavmeshProjection)
	{
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation Projected;
			if (NavSys->ProjectPointToNavigation(WorldPos, Projected, NavProjectExtent))
			{
				WorldPos = Projected.Location;
			}
		}
	}

	// Forzar altura a 0 para que no queden flotando
	WorldPos.Z = 0.f;

	// Rotación aleatoria en yaw
	const FRotator YawRot(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
	return FTransform(YawRot, WorldPos, FVector(1.f));
}

FTransform AWaveSpawner::MakeSpawnAreaCenterTransform() const
{
	// Centro del box en mundo
	FVector WorldPos = SpawnArea ? SpawnArea->GetComponentLocation() : GetActorLocation();

	// (Opcional) proyectar a nav; luego fijamos Z igualmente
	if (bUseNavmeshProjection)
	{
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation Projected;
			if (NavSys->ProjectPointToNavigation(WorldPos, Projected, NavProjectExtent))
			{
				WorldPos = Projected.Location;
			}
		}
	}

	// Forzar altura a 0
	WorldPos.Z = 0.f;

	// Alinea yaw al del box (o del actor si faltara)
	const float Yaw = SpawnArea ? SpawnArea->GetComponentRotation().Yaw : GetActorRotation().Yaw;
	return FTransform(FRotator(0.f, Yaw, 0.f), WorldPos, FVector(1.f));
}

void AWaveSpawner::HandleWaveCleared()
{
	// Evitar manejarlo dos veces
	if (bWaveClearHandled) return;
	bWaveClearHandled = true;

	// Recompensas en el centro
	SpawnRewards();

	// ¿Detener spawner al limpiar?
	if (bStopWhenCleared)
	{
		bStarted = false;
		GetWorldTimerManager().ClearTimer(TimerHandle_SpawnTick);
		GetWorldTimerManager().ClearTimer(TimerHandle_WaveDelay);
		return;
	}

	// Continuar con la siguiente oleada
	++WaveIndex;
	GetWorldTimerManager().SetTimer(TimerHandle_WaveDelay, this, &AWaveSpawner::StartNextWave, DelayBetweenWaves, false);
}

void AWaveSpawner::SpawnRewards()
{
	if (RewardCount <= 0 || !RewardClass) return;

	const FTransform CenterT = MakeSpawnAreaCenterTransform();

	for (int32 i = 0; i < RewardCount; ++i)
	{
		FActorSpawnParameters Params;
		// EXACTAMENTE en el centro (no ajustar por colisiones)
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(RewardClass, CenterT, Params);
	}
}
