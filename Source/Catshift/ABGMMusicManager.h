// ======================= ABGMMusicManager.h =======================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#include "ABGMMusicManager.generated.h" // <-- ÚLTIMO include SIEMPRE

/* =================================================================
 *  A B G M M u s i c M a n a g e r   (Actor)
 *  - Maneja 2 playlists: Exploración y Boss
 *  - Rotación aleatoria sin repetir inmediata
 *  - FadeIn/FadeOut opcional
 *  - Controlable desde Blueprint
 * ================================================================*/
UCLASS(Blueprintable, ClassGroup = (Audio))
class CATSHIFT_API ABGMMusicManager : public AActor
{
    GENERATED_BODY()

public:
    ABGMMusicManager();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Componente que reproduce la música */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM|Runtime")
    UAudioComponent* AudioComp;

    /** Playlist de exploración (3 temas) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Playlists")
    TArray<USoundBase*> ExplorationSongs;

    /** Playlist de Boss (3 temas) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Playlists")
    TArray<USoundBase*> BossSongs;

    /** Volumen maestro (0.0–2.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
    float MusicVolume = 0.8f;

    /** Iniciar automáticamente */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings")
    bool bAutoStart = true;

    /** Evita repetir inmediatamente la misma pista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings")
    bool bNoImmediateRepeat = true;

    /** Inicia con la playlist de Boss */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings")
    bool bStartWithBoss = false;

    /** Usar FadeIn/FadeOut al cambiar pista/playlist */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings")
    bool bUseFade = true;

    /** Duraciones de fade (s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings", meta = (EditCondition = "bUseFade", ClampMin = "0.0", UIMin = "0.0"))
    float FadeOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Settings", meta = (EditCondition = "bUseFade", ClampMin = "0.0", UIMin = "0.0"))
    float FadeInTime = 1.0f;

    /** Logs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM|Debug")
    bool bDebugLog = false;

    /** Playlist activa (copia de Exploration o Boss) */
    UPROPERTY(Transient)
    TArray<USoundBase*> ActiveSongs;

    /** ¿Está activa la playlist de Boss? */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM|Runtime")
    bool bBossActive = false;

private:
    UFUNCTION()
    void HandleFinished();

    void RefillQueue();
    void PlayIndex(int32 Index, bool bFadeIn = false);
    void PlayNextInternal();
    void ApplyPlaylist(const TArray<USoundBase*>& NewList, bool bImmediate);

    /** Cola barajada de índices de ActiveSongs */
    TArray<int32> Queue;

    int32 LastIndex = -1;
    int32 CurrentIndex = -1;

    /** Timer para coordinar FadeOut ? nueva pista */
    FTimerHandle Th_AfterFade;

public:
    /** Inicia la reproducción (elige aleatoria si no suena nada) */
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void StartBGM();

    /** Detiene la reproducción */
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void StopBGM();

    /** Siguiente pista */
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void NextTrack();

    /** Ajusta volumen en runtime */
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void SetMusicVolume(float NewVolume);

    /** Cambiar a Boss */
    UFUNCTION(BlueprintCallable, Category = "BGM|Playlists")
    void SwitchToBoss(bool bImmediate);

    /** Cambiar a Exploración */
    UFUNCTION(BlueprintCallable, Category = "BGM|Playlists")
    void SwitchToExploration(bool bImmediate);

    /** Info */
    UFUNCTION(BlueprintPure, Category = "BGM|Info")
    int32 GetCurrentIndex() const { return CurrentIndex; }

    UFUNCTION(BlueprintPure, Category = "BGM|Info")
    USoundBase* GetCurrentSong() const;

    UFUNCTION(BlueprintPure, Category = "BGM|Info")
    bool IsBossActive() const { return bBossActive; }
};

/* =================================================================
 *  A B G M M u s i c Z o n e   (Actor)
 *  - Trigger que conmuta playlists al entrar/salir
 *  - Puede autoencontrar el manager por Tag ("MusicManager")
 * ================================================================*/
UCLASS(Blueprintable)
class CATSHIFT_API ABGMMusicZone : public AActor
{
    GENERATED_BODY()

public:
    ABGMMusicZone();

protected:
    virtual void BeginPlay() override;

    /** Volumen de colisión (Trigger) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music Zone")
    UBoxComponent* TriggerBox;

    /** Referencia al Music Manager (asigna en el nivel o auto-busca por Tag) */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Music Zone", meta = (ExposeOnSpawn = "true"))
    ABGMMusicManager* MusicManager = nullptr;

    /** Buscar automáticamente el manager por Tag si no se asigna */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Find")
    bool bAutoFindManager = true;

    /** Tag a buscar para el Music Manager */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Find")
    FName ManagerTag = TEXT("MusicManager");

    /** Cambiar inmediatamente (FadeOut/FadeIn lo maneja el manager) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
    bool bImmediate = true;

    /** Volver a Exploración al salir del trigger */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
    bool bSwitchBackOnExit = true;

    /** Solo hacer el cambio una vez (ignora siguientes entradas) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Zone|Behavior")
    bool bOnlyOnce = false;

private:
    bool bAlreadySwitched = false;

    UFUNCTION()
    void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void TryAutoFindManager();
};
