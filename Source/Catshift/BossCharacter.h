// BossCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MeteorActors.h"
#include "BossCharacter.generated.h"

class UBoxComponent;
class UCapsuleComponent;
class UAnimMontage;
class USoundBase;
class UAudioComponent;

UENUM(BlueprintType)
enum class EBossAttack : uint8
{
	None,
	Jab,        // Puño
	Swipe,      // Carga (dash)
	Slam,       // Golpe al suelo
	FireBreath,
	Meteors
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossIntroEnded);

UCLASS()
class CATSHIFT_API ABossCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABossCharacter();

	// ====== ESTADO ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stats")
	float Health = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	int32 Phase = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool bInIntro = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	EBossAttack CurrentAttack = EBossAttack::None;

	// Pequeño respiro tras cada ataque
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Flow", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float PostAttackRecovery = 1.0f;

	// ====== ORIENTACIÓN (girar en eje Z hacia el jugador) ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Facing")
	bool bAlwaysFacePlayer = true;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Facing", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float FaceInterpSpeed = 6.f;

	// ====== HITBOXES ======
	UPROPERTY(VisibleAnywhere, Category = "Boss|Hitboxes")
	UBoxComponent* RightHitbox;

	UPROPERTY(VisibleAnywhere, Category = "Boss|Hitboxes")
	UBoxComponent* LeftHitbox;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Hitboxes")
	FName RightSocketName = "hand_r_socket";

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Hitboxes")
	FName LeftSocketName = "hand_l_socket";

	// ====== FUEGO ======
	UPROPERTY(VisibleAnywhere, Category = "Boss|Fire")
	UCapsuleComponent* FireCapsule;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Fire")
	FName MouthSocketName = "mouth_socket";

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Fire")
	float FireTickDamage = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Fire")
	float FireTickInterval = 0.12f;

	// ====== MONTAGES ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* IntroMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* JabMontage;     // Puño

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* SwipeMontage;   // Carga

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* SlamMontage;    // Golpe al suelo

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* FireBreathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Montages")
	UAnimMontage* MeteorsMontage;

	// ====== AUDIO ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
	USoundBase* IntroRoarSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
	USoundBase* PunchWhooshSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
	USoundBase* ChargeWhooshSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
	USoundBase* SlamWhooshSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
	USoundBase* FireLoopSFX;

	UPROPERTY()
	UAudioComponent* FireAudioComp = nullptr;

	// ====== METEORITOS ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	TSubclassOf<AMeteorProjectile> MeteorProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	TSubclassOf<AMeteorWarning> MeteorWarningClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	float MeteorWarningTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	int32 MeteorsCount_Phase1 = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	int32 MeteorsCount_Phase2 = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Meteors")
	int32 MeteorsCount_Phase3 = 14;

	// ====== CARGA ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Charge")
	bool bUseCodeChargeDash = true;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Charge", meta = (EditCondition = "bUseCodeChargeDash", ClampMin = "300", ClampMax = "4000"))
	float ChargeDashStrength = 1200.f;

	// ====== COOLDOWNS ======
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attacks")
	TMap<EBossAttack, float> Cooldowns;

	// ====== EVENTOS ======
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossIntroEnded OnIntroEnded;

	// ====== API ======
	UFUNCTION(BlueprintCallable) void PlayIntro();
	UFUNCTION(BlueprintCallable) bool PlayAttack(EBossAttack Attack, AActor* Target);
	UFUNCTION(BlueprintCallable) bool CanUseAttack(EBossAttack Attack) const;
	UFUNCTION(BlueprintCallable) void MarkAttackUsed(EBossAttack Attack);

	// Notifies BP
	UFUNCTION(BlueprintCallable) void EnableHitbox(FName WhichHand);
	UFUNCTION(BlueprintCallable) void DisableHitboxes();
	UFUNCTION(BlueprintCallable) void StartFireBreath();
	UFUNCTION(BlueprintCallable) void StopFireBreath();
	UFUNCTION(BlueprintCallable) void TriggerMeteorWave(AActor* TargetHint);

	// Utilidades
	EBossAttack RandomWeightedAttack(float DistanceToTarget, const AActor* Target) const;
	float IdealMinDistance(EBossAttack Attack) const;
	float IdealMaxDistance(EBossAttack Attack) const;

	UFUNCTION(BlueprintCallable) bool IsRecovering() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnRightHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnLeftHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnAnyDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

private:
	mutable TMap<EBossAttack, double> LastUsedTime;
	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
	FTimerHandle FireTickHandle;

	void DealFireTick();
	void SpawnMeteorAt(FVector GroundPos);
	bool FindGroundBelow(const FVector& Start, FVector& OutGround, float MaxDrop = 5000.f) const;

	UAnimMontage* GetMontageFor(EBossAttack Attack) const;

	// Recuperación entre ataques
	double RecoverUntilTime = 0.0;
};
