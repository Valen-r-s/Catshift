// ======================= ABGMMusicManager.cpp =======================
#include "ABGMMusicManager.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"

/* =================== ABGMMusicManager =================== */

ABGMMusicManager::ABGMMusicManager()
{
    PrimaryActorTick.bCanEverTick = false;

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BGM_Audio"));
    RootComponent = AudioComp;

    AudioComp->bAutoActivate = false;
    AudioComp->bAutoDestroy = false;
    AudioComp->bAllowSpatialization = false;
    AudioComp->bOverrideAttenuation = false;
    AudioComp->SetVolumeMultiplier(MusicVolume);
}

void ABGMMusicManager::OnConstruction(const FTransform& /*Transform*/)
{
    // Para comodidad en el editor
    if (ExplorationSongs.Num() == 0) { ExplorationSongs.SetNum(3); }
    if (BossSongs.Num() == 0) { BossSongs.SetNum(3); }

    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(MusicVolume);
    }
}

void ABGMMusicManager::BeginPlay()
{
    Super::BeginPlay();

    if (!AudioComp) return;

    if (!AudioComp->OnAudioFinished.IsAlreadyBound(this, &ABGMMusicManager::HandleFinished))
    {
        AudioComp->OnAudioFinished.AddDynamic(this, &ABGMMusicManager::HandleFinished);
    }

    // Selecciona playlist inicial
    if (bStartWithBoss)
    {
        bBossActive = true;
        ApplyPlaylist(BossSongs, /*bImmediate=*/false);
    }
    else
    {
        bBossActive = false;
        ApplyPlaylist(ExplorationSongs, /*bImmediate=*/false);
    }

    if (bAutoStart)
    {
        StartBGM();
    }
}

void ABGMMusicManager::HandleFinished()
{
    PlayNextInternal();
}

void ABGMMusicManager::RefillQueue()
{
    Queue.Reset();
    for (int32 i = 0; i < ActiveSongs.Num(); ++i) Queue.Add(i);
    Algo::RandomShuffle(Queue);

    if (bNoImmediateRepeat && LastIndex != -1 && Queue.Num() > 1 && Queue[0] == LastIndex)
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

void ABGMMusicManager::PlayIndex(int32 Index, bool bFadeIn /*=false*/)
{
    if (!AudioComp || !ActiveSongs.IsValidIndex(Index) || ActiveSongs[Index] == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BGM] PlayIndex inválido o asset nulo."));
        return;
    }

    CurrentIndex = LastIndex = Index;
    AudioComp->SetSound(ActiveSongs[Index]);

    if (bUseFade && bFadeIn && FadeInTime > 0.f)
    {
        AudioComp->FadeIn(FadeInTime, MusicVolume, 0.f);
    }
    else
    {
        AudioComp->SetVolumeMultiplier(MusicVolume);
        AudioComp->Play();
    }

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Log, TEXT("[BGM] Reproduciendo %d: %s"), Index, *ActiveSongs[Index]->GetName());
    }
}

void ABGMMusicManager::PlayNextInternal()
{
    if (ActiveSongs.Num() == 0) return;
    if (Queue.Num() == 0) RefillQueue();

    const int32 NextIdx = Queue[0];
    Queue.RemoveAt(0);

    PlayIndex(NextIdx, /*bFadeIn=*/false);
}

void ABGMMusicManager::ApplyPlaylist(const TArray<USoundBase*>& NewList, bool bImmediate)
{
    if (NewList.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BGM] Playlist vacía."));
        return;
    }

    ActiveSongs = NewList;
    LastIndex = -1;
    CurrentIndex = -1;
    Queue.Reset();
    RefillQueue();

    if (bImmediate)
    {
        const int32 StartIdx = FMath::RandRange(0, ActiveSongs.Num() - 1);

        if (bUseFade && FadeOutTime > 0.f)
        {
            AudioComp->FadeOut(FadeOutTime, 0.f);
            GetWorldTimerManager().SetTimer(Th_AfterFade, FTimerDelegate::CreateWeakLambda(this, [this, StartIdx]()
                {
                    PlayIndex(StartIdx, /*bFadeIn=*/bUseFade && FadeInTime > 0.f);
                }), FadeOutTime, false);
        }
        else
        {
            AudioComp->Stop();
            PlayIndex(StartIdx, /*bFadeIn=*/bUseFade && FadeInTime > 0.f);
        }
    }
    else
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[BGM] Playlist cambiada (no inmediata)."));
        }
    }
}

/* ---------- API BP ---------- */

void ABGMMusicManager::StartBGM()
{
    if (ActiveSongs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BGM] No hay canciones en la playlist activa."));
        return;
    }

    if (AudioComp->IsPlaying()) return;

    const int32 StartIdx = FMath::RandRange(0, ActiveSongs.Num() - 1);
    if (bNoImmediateRepeat && Queue.Num() > 1 && Queue[0] == StartIdx) { Swap(Queue[0], Queue[1]); }

    PlayIndex(StartIdx, /*bFadeIn=*/bUseFade && FadeInTime > 0.f);
}

void ABGMMusicManager::StopBGM()
{
    GetWorldTimerManager().ClearTimer(Th_AfterFade);
    if (AudioComp) { AudioComp->Stop(); CurrentIndex = -1; }
}

void ABGMMusicManager::NextTrack()
{
    PlayNextInternal();
}

void ABGMMusicManager::SetMusicVolume(float NewVolume)
{
    MusicVolume = FMath::Clamp(NewVolume, 0.f, 2.f);
    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(MusicVolume);
    }
    if (bDebugLog)
    {
        UE_LOG(LogTemp, Log, TEXT("[BGM] Volumen=%.2f"), MusicVolume);
    }
}

void ABGMMusicManager::SwitchToBoss(bool bImmediate)
{
    bBossActive = true;
    ApplyPlaylist(BossSongs, bImmediate);
}

void ABGMMusicManager::SwitchToExploration(bool bImmediate)
{
    bBossActive = false;
    ApplyPlaylist(ExplorationSongs, bImmediate);
}

USoundBase* ABGMMusicManager::GetCurrentSong() const
{
    return ActiveSongs.IsValidIndex(CurrentIndex) ? ActiveSongs[CurrentIndex] : nullptr;
}

/* =================== ABGMMusicZone =================== */

ABGMMusicZone::ABGMMusicZone()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    // Tamaño por defecto (ajústalo en el editor)
    TriggerBox->SetBoxExtent(FVector(300.f, 300.f, 200.f));

    // Config de Trigger
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetGenerateOverlapEvents(true);

    // Enlazar eventos de overlap
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABGMMusicZone::OnBoxBeginOverlap);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ABGMMusicZone::OnBoxEndOverlap);
}

void ABGMMusicZone::BeginPlay()
{
    Super::BeginPlay();

    if (!MusicManager && bAutoFindManager)
    {
        TryAutoFindManager();
    }
}

void ABGMMusicZone::TryAutoFindManager()
{
    if (ManagerTag != NAME_None)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(this, ManagerTag, Found);
        for (AActor* A : Found)
        {
            if (ABGMMusicManager* M = Cast<ABGMMusicManager>(A))
            {
                MusicManager = M;
                break;
            }
        }
    }

    // Si no se encontró por Tag, intenta por clase
    if (!MusicManager)
    {
        if (AActor* A = UGameplayStatics::GetActorOfClass(this, ABGMMusicManager::StaticClass()))
        {
            MusicManager = Cast<ABGMMusicManager>(A);
        }
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

    // Cambia a playlist de Boss
    MusicManager->SwitchToBoss(/*bImmediate=*/true);
    bAlreadySwitched = true;
}

void ABGMMusicZone::OnBoxEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
    UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
    if (!MusicManager || !OtherActor || !bSwitchBackOnExit) return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    // Volver a Exploración al salir
    MusicManager->SwitchToExploration(/*bImmediate=*/true);
}
