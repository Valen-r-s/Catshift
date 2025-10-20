// TutorialWidget.cpp
#include "TutorialWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Cargar softs si llegaron por propiedad
	if (LoadedImages.Num() == 0 && Images.Num() > 0)
	{
		for (const TSoftObjectPtr<UTexture2D>& SoftTex : Images)
		{
			if (UTexture2D* Tex = SoftTex.LoadSynchronous())
			{
				LoadedImages.Add(Tex);
			}
		}
	}

	// Bind botones si existen
	if (BtnNext) { BtnNext->OnClicked.AddDynamic(this, &UTutorialWidget::ShowNext); }
	if (BtnSkip) { BtnSkip->OnClicked.AddDynamic(this, &UTutorialWidget::CloseTutorial); }

	CurrentIndex = 0;

	// Mostrar primera o cerrar si no hay
	if (LoadedImages.IsValidIndex(0)) { ShowIndex(0); }
	else { CloseTutorial(); return; }

	// Tomar foco UI
	TakeUIFocus();

	// Auto-avance (opcional)
	if (bAutoAdvance && AutoAdvanceTime > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoTimer, this, &UTutorialWidget::ShowNext, AutoAdvanceTime, true, AutoAdvanceTime);
	}
}

void UTutorialWidget::NativeDestruct()
{
	if (AutoTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoTimer);
	}
	Super::NativeDestruct();
}

void UTutorialWidget::SetImages(const TArray<UTexture2D*>& InImages)
{
	LoadedImages = InImages;
}

void UTutorialWidget::ShowIndex(int32 Index)
{
	if (!ImgTutorial) return;
	if (LoadedImages.IsValidIndex(Index))
	{
		ImgTutorial->SetBrushFromTexture(LoadedImages[Index], /*bMatchSize*/ false);
	}
}

void UTutorialWidget::ShowNext()
{
	++CurrentIndex;

	if (!LoadedImages.IsValidIndex(CurrentIndex))
	{
		CloseTutorial();
		return;
	}

	ShowIndex(CurrentIndex);
}

void UTutorialWidget::CloseTutorial()
{
	// Quita timer si estaba activo
	if (AutoTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoTimer);
	}

	// MUY IMPORTANTE: restaurar input antes de quitar la UI
	ReturnToGameInput();

	OnTutorialFinished.Broadcast();

	RemoveFromParent();
}

void UTutorialWidget::TakeUIFocus()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(Mode);

		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}
}

void UTutorialWidget::ReturnToGameInput()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		// 1) Forzar un “paso” por Game+UI para soltar cualquier captura
		{
			FInputModeGameAndUI UI;
			UI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			UI.SetHideCursorDuringCapture(false);
			UI.SetWidgetToFocus(nullptr);           // sin widget a enfocar
			PC->SetInputMode(UI);
		}

		// 2) Ir a GameOnly, evitando capturar el primer click
		{
			FInputModeGameOnly Game;
			Game.SetConsumeCaptureMouseDown(false);
			PC->SetInputMode(Game);
		}

		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
		PC->FlushPressedKeys();
		UWidgetBlueprintLibrary::SetFocusToGameViewport();
	}
}
