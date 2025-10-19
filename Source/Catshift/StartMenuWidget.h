// StartMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenuWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitClicked);

UCLASS(BlueprintType, Blueprintable)
class CATSHIFT_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Menu")
	FOnStartClicked OnStartClicked;

	UPROPERTY(BlueprintAssignable, Category = "Menu")
	FOnExitClicked OnExitClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* BtnStart = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnExit = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION() void HandleStart();
	UFUNCTION() void HandleExit();
};
