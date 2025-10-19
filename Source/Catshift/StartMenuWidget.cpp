// StartMenuWidget.cpp
#include "StartMenuWidget.h"
#include "Components/Button.h"

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnStart)
	{
		BtnStart->OnClicked.AddDynamic(this, &UStartMenuWidget::HandleStart);
	}
	if (BtnExit)
	{
		BtnExit->OnClicked.AddDynamic(this, &UStartMenuWidget::HandleExit);
	}
}

void UStartMenuWidget::HandleStart()
{
	OnStartClicked.Broadcast();
}

void UStartMenuWidget::HandleExit()
{
	OnExitClicked.Broadcast();
}
