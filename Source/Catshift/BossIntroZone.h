// BossIntroZone.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossIntroZone.generated.h"

class UBoxComponent;
class ACameraActor;
class ABossCharacter;
class UUserWidget;
class UBossHealthWidget;
class APlayerController;

UCLASS()
class CATSHIFT_API ABossIntroZone : public AActor
{
	GENERATED_BODY()

public:
	ABossIntroZone();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnActorBeginOverlap_A(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleBossIntroEnded();

public:
	// Zona de trigger
	UPROPERTY(VisibleAnywhere, Category = "IntroZone")
	UBoxComponent* Box;

	// Referencia al Boss en la escena
	UPROPERTY(EditInstanceOnly, Category = "Boss")
	ABossCharacter* BossRef = nullptr;

	// Cámara de introducción
	UPROPERTY(EditInstanceOnly, Category = "Intro")
	ACameraActor* IntroCamera = nullptr;

	// Tiempos de blend de cámara
	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BlendTimeToIntro = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BlendTimeBack = 0.6f;

private:
	// === UI ===
	// Permite asignar cualquier UserWidget desde la instancia en el nivel
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> BossHealthWidgetClass;

	UPROPERTY()
	UUserWidget* BossHealthWidget = nullptr;

	// Cache de PlayerController
	UPROPERTY()
	APlayerController* PC = nullptr;
};
