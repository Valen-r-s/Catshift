// BossIntroZone.cpp
#include "BossIntroZone.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

#include "BossCharacter.h"
#include "BossHealthWidget.h" // Para el cast opcional a UBossHealthWidget

ABossIntroZone::ABossIntroZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
}

void ABossIntroZone::BeginPlay()
{
	Super::BeginPlay();

	// Vincula al evento de overlap del Actor
	OnActorBeginOverlap.AddDynamic(this, &ABossIntroZone::OnActorBeginOverlap_A);
}

void ABossIntroZone::OnActorBeginOverlap_A(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!BossRef || !IntroCamera) return;

	// Solo el jugador activa
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || OtherActor != PlayerPawn) return;

	// Evitar reentradas
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Cachear PlayerController
	PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	// === HUD === crea el widget y lo muestra
	if (BossHealthWidgetClass && !BossHealthWidget)
	{
		BossHealthWidget = CreateWidget<UUserWidget>(PC, BossHealthWidgetClass);
		if (BossHealthWidget)
		{
			BossHealthWidget->AddToViewport(100);

			// Si este widget hereda de UBossHealthWidget, pásale el BossRef
			if (UBossHealthWidget* Typed = Cast<UBossHealthWidget>(BossHealthWidget))
			{
				Typed->SetBoss(BossRef);
			}
			// Si es un UserWidget puro, el propio WBP debe resolver BossRef/binds en su Event Construct.
		}
	}

	// === Cámara de intro + bloqueo de input ===
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);
	PC->SetViewTargetWithBlend(IntroCamera, BlendTimeToIntro);

	// Al terminar la intro, devolvemos control/cámara
	BossRef->OnIntroEnded.AddDynamic(this, &ABossIntroZone::HandleBossIntroEnded);

	// ¡Reproduce la intro del boss!
	BossRef->PlayIntro();
}

void ABossIntroZone::HandleBossIntroEnded()
{
	// Recuperar PlayerController si hiciera falta
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC) return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		PC->SetViewTargetWithBlend(PlayerPawn, BlendTimeBack);
	}

	PC->SetIgnoreLookInput(false);
	PC->SetIgnoreMoveInput(false);

	// La barra de vida sigue visible durante el combate; se removerá sola cuando el boss muera (si tu WBP lo maneja).
}
