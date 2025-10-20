// CatshiftGameMode.cpp
#include "CatshiftGameMode.h"

#include "StartMenuWidget.h"
#include "TutorialWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

// Enhanced Input (opcional)
#include "EnhancedInputSubsystems.h"

ACatshiftGameMode::ACatshiftGameMode()
{
}

void ACatshiftGameMode::BeginPlay()
{
	Super::BeginPlay();
	ShowStartMenu();
}

void ACatshiftGameMode::ShowStartMenu()
{
	if (!StartMenuClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartMenuClass no asignado (configúralo en tu BP_CatshiftGameMode)."));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	StartMenu = CreateWidget<UStartMenuWidget>(GetWorld(), StartMenuClass);
	if (!StartMenu) { return; }

	StartMenu->OnStartClicked.AddDynamic(this, &ACatshiftGameMode::HandleStartRequested);
	StartMenu->OnExitClicked.AddDynamic(this, &ACatshiftGameMode::HandleExitRequested);

	StartMenu->AddToViewport(10);

	if (PC)
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(StartMenu->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}

	if (bPauseDuringMenus)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void ACatshiftGameMode::HandleStartRequested()
{
	if (StartMenu)
	{
		StartMenu->RemoveFromParent();
		StartMenu = nullptr;
	}

	ShowTutorial();
}

void ACatshiftGameMode::HandleExitRequested()
{
	UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0),
		EQuitPreference::Quit, false);
}

void ACatshiftGameMode::ShowTutorial()
{
	if (!TutorialWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialWidgetClass no asignado."));
		if (bPauseDuringMenus) { UGameplayStatics::SetGamePaused(this, false); }
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	Tutorial = CreateWidget<UTutorialWidget>(GetWorld(), TutorialWidgetClass);
	if (!Tutorial)
	{
		if (bPauseDuringMenus) { UGameplayStatics::SetGamePaused(this, false); }
		return;
	}

	// Pasar imágenes desde el GM si las configuraste aquí
	if (TutorialImages.Num() > 0)
	{
		TArray<UTexture2D*> RuntimeImages;
		RuntimeImages.Reserve(TutorialImages.Num());
		for (const TSoftObjectPtr<UTexture2D>& SoftTex : TutorialImages)
		{
			if (UTexture2D* Tex = SoftTex.LoadSynchronous())
			{
				RuntimeImages.Add(Tex);
			}
		}
		Tutorial->SetImages(RuntimeImages);
	}

	Tutorial->OnTutorialFinished.AddDynamic(this, &ACatshiftGameMode::HandleTutorialFinished);
	Tutorial->AddToViewport(20);

	// Foco UI sobre el tutorial (extra de seguridad)
	if (PC)
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Tutorial->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}

	if (bPauseDuringMenus)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void ACatshiftGameMode::HandleTutorialFinished()
{
	// El widget ya despausó y puso GameOnly, pero reforzamos:
	ForceRestoreGameInput();
	Tutorial = nullptr;
}

void ACatshiftGameMode::ForceRestoreGameInput()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameAndUI UI;
		UI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		UI.SetHideCursorDuringCapture(false);
		UI.SetWidgetToFocus(nullptr);
		PC->SetInputMode(UI);

		FInputModeGameOnly Game;
		Game.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(Game);

		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		PC->FlushPressedKeys();
		UWidgetBlueprintLibrary::SetFocusToGameViewport();
	}
}
