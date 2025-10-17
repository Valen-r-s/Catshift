// BossIntroZone.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossIntroZone.generated.h"

class UBoxComponent;
class ABossCharacter;
class ACameraActor;

UCLASS()
class CATSHIFT_API ABossIntroZone : public AActor
{
	GENERATED_BODY()

public:
	ABossIntroZone();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Box;

	UPROPERTY(EditInstanceOnly, Category = "BossIntro")
	ABossCharacter* BossRef;

	UPROPERTY(EditInstanceOnly, Category = "BossIntro")
	ACameraActor* IntroCamera;

	UPROPERTY(EditDefaultsOnly, Category = "BossIntro")
	float BlendTimeToIntro = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "BossIntro")
	float BlendTimeBack = 1.0f;

	UFUNCTION()
	void OnActorBeginOverlap_A(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleBossIntroEnded();

private:
	UPROPERTY()
	APlayerController* PC = nullptr;
};
