// CatshiftGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CatshiftGameMode.generated.h"

class UStartMenuWidget;
class UTutorialWidget;
class UTexture2D;
class UInputMappingContext; // Enhanced Input (opcional)

UCLASS()
class CATSHIFT_API ACatshiftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACatshiftGameMode();

protected:
	virtual void BeginPlay() override;

	/** WBP del menú (Parent Class = UStartMenuWidget) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UStartMenuWidget> StartMenuClass;

	/** WBP del tutorial (Parent Class = UTutorialWidget) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTutorialWidget> TutorialWidgetClass;

	/** Opcional: imágenes del tutorial definidas desde el GM */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TArray<TSoftObjectPtr<UTexture2D>> TutorialImages;

	/** Pausar durante menús/tutorial */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	bool bPauseDuringMenus = true;

	/** (Opcional) Enhanced Input: Mapping de gameplay para reañadir tras el tutorial */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* GameplayMapping = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	int32 GameplayMappingPriority = 0;

private:
	UPROPERTY() UStartMenuWidget* StartMenu = nullptr;
	UPROPERTY() UTutorialWidget* Tutorial = nullptr;

	void ShowStartMenu();
	void ShowTutorial();

	UFUNCTION() void HandleStartRequested();
	UFUNCTION() void HandleExitRequested();
	UFUNCTION() void HandleTutorialFinished();

	/** Reaplica InputMode y mapping de gameplay (Enhanced Input opcional) */
	void ForceRestoreGameInput();
};
