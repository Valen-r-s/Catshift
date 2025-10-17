// MeteorActors.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeteorActors.generated.h"

class UStaticMeshComponent;
class UDecalComponent;
class UMaterialInterface;
class USoundBase;

// =========================
//   AMeteorProjectile
// =========================
UCLASS()
class CATSHIFT_API AMeteorProjectile : public AActor
{
	GENERATED_BODY()
public:
	AMeteorProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float GravitySpeed = 3200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float ExplosionDamage = 35.f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float ExplosionRadius = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float LifeAfterImpact = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor|Audio")
	USoundBase* ExplosionSFX;

	// Lo setea el Boss al spawnear
	UPROPERTY(Transient)
	FVector TargetGround = FVector::ZeroVector;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	bool bImpacted = false;
};

// =========================
//     AMeteorWarning
// =========================
UCLASS()
class CATSHIFT_API AMeteorWarning : public AActor
{
	GENERATED_BODY()
public:
	AMeteorWarning();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDecalComponent* Decal;

	UPROPERTY(EditDefaultsOnly, Category = "Warning")
	UMaterialInterface* WarningMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	float WarningTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning")
	float Radius = 300.f;

protected:
	virtual void BeginPlay() override;
};
