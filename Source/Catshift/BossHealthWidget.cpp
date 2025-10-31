// BossHealthWidget.cpp
#include "BossHealthWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BossCharacter.h"

void UBossHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Inicializa barra y nombre fijo
	if (Boss && HealthBar)
	{
		HealthBar->SetPercent(Boss->GetHealthPercent());
	}
	if (BossNameText)
	{
		BossNameText->SetText(FixedBossName); // << SIEMPRE "Demon King"
	}
}

void UBossHealthWidget::NativeDestruct()
{
	if (Boss)
	{
		Boss->OnHealthChanged.RemoveDynamic(this, &UBossHealthWidget::OnBossHealthChanged);
		Boss->OnBossDied.RemoveDynamic(this, &UBossHealthWidget::OnBossDied);
	}
	Super::NativeDestruct();
}

void UBossHealthWidget::SetBoss(ABossCharacter* InBoss)
{
	if (Boss)
	{
		Boss->OnHealthChanged.RemoveDynamic(this, &UBossHealthWidget::OnBossHealthChanged);
		Boss->OnBossDied.RemoveDynamic(this, &UBossHealthWidget::OnBossDied);
	}

	Boss = InBoss;

	if (Boss)
	{
		Boss->OnHealthChanged.AddDynamic(this, &UBossHealthWidget::OnBossHealthChanged);
		Boss->OnBossDied.AddDynamic(this, &UBossHealthWidget::OnBossDied);

		if (HealthBar) HealthBar->SetPercent(Boss->GetHealthPercent());

		// << ANTES usabas GetName(); cámbialo por el nombre fijo >>
		if (BossNameText) BossNameText->SetText(FixedBossName);
	}
}

void UBossHealthWidget::OnBossHealthChanged(float NewHealth, float MaxHealth)
{
	if (HealthBar && MaxHealth > 0.f)
	{
		HealthBar->SetPercent(NewHealth / MaxHealth);
	}
}

void UBossHealthWidget::OnBossDied()
{
	RemoveFromParent();
}
