#include "ABGMMusicManager.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"

ABGMMusicManager::ABGMMusicManager()
{
	PrimaryActorTick.bCanEverTick = false;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BGM_Audio"));
	RootComponent = AudioComp;

	AudioComp->bAutoActivate = false;
	AudioComp->bIsUISound = false;
}

void ABGMMusicManager::BeginPlay()
{
	Super::BeginPlay();

	if (!AudioComp) return;

	AudioComp->SetVolumeMultiplier(MusicVolume);

	// >>> Cambiado: delegate Blueprint SIN parámetros
	if (!AudioComp->OnAudioFinished.IsAlreadyBound(this, &ABGMMusicManager::HandleFinished_BP))
	{
		AudioComp->OnAudioFinished.AddDynamic(this, &ABGMMusicManager::HandleFinished_BP);
	}

	bBossActive = bStartWithBoss;
	ApplyPlaylist(bBossActive ? BossSongs : ExplorationSongs, /*bImmediate=*/false);

	if (bAutoStart)
	{
		StartBGM();
	}
}

void ABGMMusicManager::SwitchToBoss(bool bImmediate)
{
	if (bBossActive && IsPlaying()) return;
	bBossActive = true;
	ApplyPlaylist(BossSongs, bImmediate);
}

void ABGMMusicManager::SwitchToExploration(bool bImmediate)
{
	if (!bBossActive && IsPlaying()) return;
	bBossActive = false;
	ApplyPlaylist(ExplorationSongs, bImmediate);
}

void ABGMMusicManager::StartBGM()
{
	if (ActiveSongs.Num() == 0) return;
	if (IsPlaying()) return;

	int32 StartIdx = 0;
	if (bStartAtRandomIndex)
	{
		StartIdx = FMath::RandRange(0, ActiveSongs.Num() - 1);
	}
	PlayIndex(StartIdx, /*bFadeIn=*/bUseFade && FadeInTime > 0.f);
}

void ABGMMusicManager::StopBGM()
{
	GetWorldTimerManager().ClearTimer(Th_AfterFade);
	GetWorldTimerManager().ClearTimer(Th_FallbackFinish);
	if (AudioComp) AudioComp->Stop();
}

void ABGMMusicManager::NextTrack()
{
	if (ActiveSongs.Num() == 0) return;

	if (bUseFade && FadeOutTime > 0.f && IsPlaying())
	{
		AudioComp->FadeOut(FadeOutTime, 0.0f);
		GetWorldTimerManager().SetTimer(
			Th_AfterFade,
			FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					PlayNextInternal();
				}),
			FadeOutTime,
			false
		);
	}
	else
	{
		if (IsPlaying()) AudioComp->Stop();
		PlayNextInternal();
	}
}

void ABGMMusicManager::ApplyPlaylist(const TArray<USoundBase*>& NewList, bool bImmediate)
{
	GetWorldTimerManager().ClearTimer(Th_AfterFade);
	GetWorldTimerManager().ClearTimer(Th_FallbackFinish);

	ActiveSongs = NewList;
	CurrentIndex = INDEX_NONE;
	LastIndex = INDEX_NONE;
	Queue.Reset();

	if (ActiveSongs.Num() == 0)
	{
		if (AudioComp) AudioComp->Stop();
		return;
	}

	RefillQueue();

	if (bImmediate)
	{
		if (bUseFade && FadeOutTime > 0.f && IsPlaying())
		{
			AudioComp->FadeOut(FadeOutTime, 0.0f);
			GetWorldTimerManager().SetTimer(
				Th_AfterFade,
				FTimerDelegate::CreateWeakLambda(this, [this]()
					{
						PlayNextInternal();
					}),
				FadeOutTime,
				false
			);
		}
		else
		{
			if (IsPlaying()) AudioComp->Stop();
			PlayNextInternal();
		}
	}
	else
	{
		if (!IsPlaying())
		{
			PlayNextInternal();
		}
	}
}

void ABGMMusicManager::RefillQueue()
{
	Queue.Reset();
	for (int32 i = 0; i < ActiveSongs.Num(); ++i)
	{
		Queue.Add(i);
	}
	Algo::RandomShuffle(Queue);

	if (bNoImmediateRepeat && LastIndex != INDEX_NONE && Queue.Num() > 1 && Queue[0] == LastIndex)
	{
		Swap(Queue[0], Queue[1]);
	}

	if (bDebugLog)
	{
		FString Seq;
		for (int32 Idx : Queue) { Seq += FString::FromInt(Idx) + TEXT(" "); }
		UE_LOG(LogTemp, Log, TEXT("[BGM] Cola: %s"), *Seq);
	}
}

void ABGMMusicManager::PlayNextInternal()
{
	if (ActiveSongs.Num() == 0) return;

	if (Queue.Num() == 0)
	{
		RefillQueue();
		if (Queue.Num() == 0) return;
	}

	const int32 NextIdx = Queue[0];
	Queue.RemoveAt(0);
	PlayIndex(NextIdx, /*bFadeIn=*/bUseFade && FadeInTime > 0.f);
}

void ABGMMusicManager::PlayIndex(int32 Index, bool bFadeIn)
{
	if (!AudioComp || !ActiveSongs.IsValidIndex(Index)) return;

	USoundBase* Sound = ActiveSongs[Index];
	if (!Sound) return;

	LastIndex = CurrentIndex;
	CurrentIndex = Index;

	GetWorldTimerManager().ClearTimer(Th_AfterFade);
	GetWorldTimerManager().ClearTimer(Th_FallbackFinish);

	AudioComp->SetSound(Sound);
	if (bFadeIn)
	{
		AudioComp->FadeIn(FadeInTime, MusicVolume);
	}
	else
	{
		AudioComp->SetVolumeMultiplier(MusicVolume);
		AudioComp->Play(0.0f);
	}

	const float Dur = GetCurrentDurationSafe(Sound);
	if (Dur > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			Th_FallbackFinish,
			FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					// Si por alguna razón no se disparó OnAudioFinished
					if (!IsPlaying())
					{
						PlayNextInternal();
					}
				}),
			Dur + 0.05f,
			false
		);
	}

	if (bDebugLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[BGM] Playing idx %d (dur=%.2fs)"), Index, Dur);
	}
}

float ABGMMusicManager::GetCurrentDurationSafe(USoundBase* Snd) const
{
	if (!Snd) return 0.f;
	const float D = Snd->GetDuration();
	return (D > 0.f) ? D : 0.f;
}

// >>> Cambiado: handler SIN parámetros para el delegate Blueprint
void ABGMMusicManager::HandleFinished_BP()
{
	GetWorldTimerManager().ClearTimer(Th_FallbackFinish);
	PlayNextInternal();
}

/* =================== ABGMMusicZone =================== */

ABGMMusicZone::ABGMMusicZone()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MusicZone_Box"));
	TriggerBox->InitBoxExtent(FVector(400.f, 400.f, 1200.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABGMMusicZone::OnBoxBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ABGMMusicZone::OnBoxEndOverlap);
}

void ABGMMusicZone::BeginPlay()
{
	Super::BeginPlay();
	TryAutoFindManager();
}

bool ABGMMusicZone::CanSwitch() const
{
	const UWorld* W = GetWorld();
	if (!W) return true;
	if (LastSwitchTime < 0.0) return true;
	return (W->GetTimeSeconds() - LastSwitchTime) >= ReTriggerCooldown;
}

void ABGMMusicZone::MarkSwitched()
{
	if (UWorld* W = GetWorld())
	{
		LastSwitchTime = W->GetTimeSeconds();
	}
}

void ABGMMusicZone::OnBoxBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!MusicManager || !OtherActor) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;

	if (bOnlyOnce && bAlreadySwitched) return;
	if (bGuardIfAlreadyActive && MusicManager->IsBossActive()) return;
	if (!CanSwitch()) return;

	MusicManager->SwitchToBoss(bImmediate);
	bAlreadySwitched = true;
	MarkSwitched();
}

void ABGMMusicZone::OnBoxEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (!MusicManager || !OtherActor || !bSwitchBackOnExit) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;

	if (bGuardIfAlreadyActive && !MusicManager->IsBossActive()) return;
	if (!CanSwitch()) return;

	MusicManager->SwitchToExploration(bImmediate);
	MarkSwitched();
}

void ABGMMusicZone::TryAutoFindManager()
{
	if (MusicManager) return;

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABGMMusicManager::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		MusicManager = Cast<ABGMMusicManager>(Found[0]);
	}
}
