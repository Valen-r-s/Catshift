// BossIntroZone.cpp
#include "BossIntroZone.h"
#include "Components/BoxComponent.h"
#include "BossCharacter.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

ABossIntroZone::ABossIntroZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABossIntroZone::BeginPlay()
{
	Super::BeginPlay();
	// Usamos el evento del Actor (no del componente) para firma simple (AActor*, AActor*)
	OnActorBeginOverlap.AddDynamic(this, &ABossIntroZone::OnActorBeginOverlap_A);
}

void ABossIntroZone::OnActorBeginOverlap_A(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!BossRef || !IntroCamera) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor != PlayerPawn) return;

	// desactivar la zona para que no dispare 2 veces
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);
	PC->SetViewTargetWithBlend(IntroCamera, BlendTimeToIntro);

	BossRef->OnIntroEnded.AddDynamic(this, &ABossIntroZone::HandleBossIntroEnded);
	BossRef->PlayIntro();
}

void ABossIntroZone::HandleBossIntroEnded()
{
	if (!PC) PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	PC->SetViewTargetWithBlend(PlayerPawn, BlendTimeBack);
	PC->SetIgnoreLookInput(false);
	PC->SetIgnoreMoveInput(false);
}
