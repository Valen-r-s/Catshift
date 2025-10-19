// BossHealthWidget.cpp
#include "BossHealthWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BossCharacter.h"

void UBossHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Si ya teníamos boss asignado antes de construir, inicializa
	if (Boss && HealthBar)
	{
		HealthBar->SetPercent(Boss->GetHealthPercent());
	}
}

void UBossHealthWidget::NativeDestruct()
{
	// Limpia bindings
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

		// Inicializar
		if (HealthBar) HealthBar->SetPercent(Boss->GetHealthPercent());
		if (BossNameText) BossNameText->SetText(FText::FromString(Boss->GetName()));
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
