#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "ABGMMusicManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CATSHIFT_API ABGMMusicManager : public AActor
{
	GENERATED_BODY()

public:
	ABGMMusicManager();

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void SwitchToBoss(bool bImmediate);

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void SwitchToExploration(bool bImmediate);

	UFUNCTION(BlueprintPure, Category = "BGM")
	bool IsBossActive() const { return bBossActive; }

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void StartBGM();

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void StopBGM();

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void NextTrack();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM")
	UAudioComponent* AudioComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM|Playlists")
	TArray<USoundBase*> ExplorationSongs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM|Playlists")
	TArray<USoundBase*> BossSongs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Volume", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MusicVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Fade")
	bool bUseFade = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Fade", meta = (EditCondition = "bUseFade", ClampMin = "0.0", UIMin = "0.0"))
	float FadeOutTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Fade", meta = (EditCondition = "bUseFade", ClampMin = "0.0", UIMin = "0.0"))
	float FadeInTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Behavior")
	bool bStartAtRandomIndex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Behavior")
	bool bNoImmediateRepeat = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Behavior")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Behavior")
	bool bStartWithBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Debug")
	bool bDebugLog = false;

private:
	UPROPERTY(Transient)
	TArray<USoundBase*> ActiveSongs;

	bool bBossActive = false;
	int32 CurrentIndex = INDEX_NONE;
	int32 LastIndex = INDEX_NONE;

	TArray<int32> Queue;

	FTimerHandle Th_AfterFade;
	FTimerHandle Th_FallbackFinish;

	void ApplyPlaylist(const TArray<USoundBase*>& NewList, bool bImmediate);
	void PlayIndex(int32 Index, bool bFadeIn);
	void PlayNextInternal();
	void RefillQueue();

	// >>> Cambiado: handler SIN parámetros para OnAudioFinished (Blueprint)
	UFUNCTION()
	void HandleFinished_BP();

	bool IsPlaying() const { return AudioComp && AudioComp->IsPlaying(); }
	float GetCurrentDurationSafe(USoundBase* Snd) const;
};

UCLASS(BlueprintType, Blueprintable)
class CATSHIFT_API ABGMMusicZone : public AActor
{
	GENERATED_BODY()

public:
	ABGMMusicZone();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music Zone")
	UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
	bool bImmediate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
	bool bOnlyOnce = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
	bool bSwitchBackOnExit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
	bool bGuardIfAlreadyActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ReTriggerCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Refs")
	ABGMMusicManager* MusicManager = nullptr;

private:
	bool bAlreadySwitched = false;
	double LastSwitchTime = -1.0;

	bool CanSwitch() const;
	void  MarkSwitched();

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TryAutoFindManager();
};
