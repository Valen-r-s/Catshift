// BossHealthWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthWidget.generated.h"

class UProgressBar;
class UTextBlock;
class ABossCharacter;


UCLASS()
class CATSHIFT_API UBossHealthWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void SetBoss(ABossCharacter* InBoss);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* BossNameText = nullptr;

	// << NUEVO: nombre fijo para mostrar en HUD >>
	UPROPERTY(EditDefaultsOnly, Category = "BossHUD")
	FText FixedBossName = NSLOCTEXT("BossHUD", "FixedBossName", "Demon King");

private:
	UPROPERTY()
	ABossCharacter* Boss = nullptr;

	UFUNCTION()
	void OnBossHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void OnBossDied();
};

