// BossCharacter.cpp
#include "BossCharacter.h"
#include "BossAIController.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

// Logs
DEFINE_LOG_CATEGORY_STATIC(LogBoss, Log, All);

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ABossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Queremos rotar el Actor manualmente (yaw en Tick), sin que el movimiento/Controller nos pisotee
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	// Right Hitbox
	RightHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHitbox"));
	RightHitbox->SetupAttachment(GetMesh(), RightSocketName);
	RightHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	RightHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Left Hitbox
	LeftHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHitbox"));
	LeftHitbox->SetupAttachment(GetMesh(), LeftSocketName);
	LeftHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	LeftHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Fire Capsule
	FireCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("FireCapsule"));
	FireCapsule->SetupAttachment(GetMesh(), MouthSocketName);
	FireCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	FireCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	FireCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FireCapsule->InitCapsuleSize(120.f, 220.f);

	// Cooldowns por defecto
	Cooldowns.Add(EBossAttack::Jab, 2.0f);  // Puño
	Cooldowns.Add(EBossAttack::Swipe, 6.0f);  // Carga
	Cooldowns.Add(EBossAttack::Slam, 5.0f);  // Suelo
	Cooldowns.Add(EBossAttack::FireBreath, 8.0f);
	Cooldowns.Add(EBossAttack::Meteors, 12.0f);
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	RightHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnRightHitboxOverlap);
	LeftHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnLeftHitboxOverlap);

	OnTakeAnyDamage.AddDynamic(this, &ABossCharacter::OnAnyDamageTaken);
}

void ABossCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Mantener el cono del fuego orientado
	if (FireCapsule->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const FVector From = GetMesh()->GetSocketLocation(MouthSocketName);
			const FRotator Look = UKismetMathLibrary::FindLookAtRotation(From, P->GetActorLocation());
			FireCapsule->SetWorldRotation(Look);
		}
	}

	// Girar suavemente en Yaw (eje Z) hacia el jugador
	if (bAlwaysFacePlayer && !bInIntro)
	{
		if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const FVector ToPlayer2D = (P->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			const float TargetYaw = ToPlayer2D.Rotation().Yaw;
			const FRotator TargetRot(0.f, TargetYaw, 0.f);

			const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds, FaceInterpSpeed);
			SetActorRotation(NewRot);
		}
	}
}

void ABossCharacter::PlayIntro()
{
	if (IntroRoarSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, IntroRoarSFX, GetActorLocation());
	}

	if (!IntroMontage) { bInIntro = false; OnIntroEnded.Broadcast(); return; }

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		Anim->Montage_Play(IntroMontage, 1.f);
		FOnMontageEnded End;
		End.BindUObject(this, &ABossCharacter::OnMontageEnded);
		Anim->Montage_SetEndDelegate(End, IntroMontage);
	}
}

UAnimMontage* ABossCharacter::GetMontageFor(EBossAttack Attack) const
{
	switch (Attack)
	{
	case EBossAttack::Jab:        return JabMontage;
	case EBossAttack::Swipe:      return SwipeMontage;
	case EBossAttack::Slam:       return SlamMontage;
	case EBossAttack::FireBreath: return FireBreathMontage;
	case EBossAttack::Meteors:    return MeteorsMontage;
	default: return nullptr;
	}
}

bool ABossCharacter::PlayAttack(EBossAttack Attack, AActor* Target)
{
	if (bInIntro || bIsAttacking || IsRecovering()) return false;
	if (!CanUseAttack(Attack)) return false;

	UAnimMontage* Montage = GetMontageFor(Attack);

	// Encara al objetivo (solo yaw) antes de atacar
	if (Target)
	{
		const FRotator Look = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
		SetActorRotation(FRotator(0, Look.Yaw, 0));
	}

	// Caso especial: permitir Meteors aunque no haya montage
	if (Attack == EBossAttack::Meteors && !Montage)
	{
		CurrentAttack = Attack;
		MarkAttackUsed(Attack);
		TriggerMeteorWave(Target);
		RecoverUntilTime = GetWorld()->GetTimeSeconds() + PostAttackRecovery;
		return true;
	}

	if (!Montage) return false;

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		CurrentAttack = Attack;
		bIsAttacking = true;

		// Dash por código para la carga si no hay root motion
		if (Attack == EBossAttack::Swipe && bUseCodeChargeDash)
		{
			const FVector Dash = GetActorForwardVector() * ChargeDashStrength;
			LaunchCharacter(Dash, true, false);
		}

		Anim->Montage_Play(Montage, 1.f);

		// Delegate de Blending-Out (para apagar hitboxes/efectos suavemente)
		FOnMontageBlendingOutStarted BlendOut;
		BlendOut.BindUObject(this, &ABossCharacter::OnMontageBlendingOut);
		Anim->Montage_SetBlendingOutDelegate(BlendOut, Montage);

		// Delegate de fin
		FOnMontageEnded End;
		End.BindUObject(this, &ABossCharacter::OnMontageEnded);
		Anim->Montage_SetEndDelegate(End, Montage);

		// Backup meteors por si olvidas notify
		if (Attack == EBossAttack::Meteors)
		{
			TriggerMeteorWave(Target);
		}

		MarkAttackUsed(Attack);
		return true;
	}
	return false;
}

bool ABossCharacter::CanUseAttack(EBossAttack Attack) const
{
	const double Now = GetWorld()->GetTimeSeconds();
	const float* Cd = Cooldowns.Find(Attack);
	const double* Last = LastUsedTime.Find(Attack);
	if (!Cd) return true;
	if (!Last) return true;
	return (Now - *Last) >= *Cd;
}

void ABossCharacter::MarkAttackUsed(EBossAttack Attack)
{
	LastUsedTime.FindOrAdd(Attack) = GetWorld()->GetTimeSeconds();
}

void ABossCharacter::OnMontageEnded(UAnimMontage* Montage, bool)
{
	if (Montage == IntroMontage)
	{
		bInIntro = false;
		OnIntroEnded.Broadcast();
		return;
	}
	bIsAttacking = false;
	CurrentAttack = EBossAttack::None;
	DisableHitboxes();
	StopFireBreath();

	// Recuperación post-ataque
	RecoverUntilTime = GetWorld()->GetTimeSeconds() + PostAttackRecovery;
}

void ABossCharacter::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// Apaga ventanas de daño un poco antes del corte visual
	DisableHitboxes();

	// Asegura cortar el loop/daño del fuego si está mezclando hacia fuera
	if (Montage == FireBreathMontage)
	{
		StopFireBreath();
	}
}

bool ABossCharacter::IsRecovering() const
{
	return GetWorld()->GetTimeSeconds() < RecoverUntilTime;
}

void ABossCharacter::EnableHitbox(FName WhichHand)
{
	AlreadyHitActors.Reset();

	// SFX por ataque
	if (CurrentAttack == EBossAttack::Jab && PunchWhooshSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PunchWhooshSFX, GetActorLocation());
	}
	else if (CurrentAttack == EBossAttack::Swipe && ChargeWhooshSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChargeWhooshSFX, GetActorLocation());
	}
	else if (CurrentAttack == EBossAttack::Slam && SlamWhooshSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SlamWhooshSFX, GetActorLocation());
	}

	const FString S = WhichHand.ToString().ToLower();
	if (S.IsEmpty() || S.Contains(TEXT("right")))
	{
		RightHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	if (S.Contains(TEXT("left")))
	{
		LeftHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ABossCharacter::DisableHitboxes()
{
	RightHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AlreadyHitActors.Reset();
}

void ABossCharacter::OnRightHitboxOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	if (AlreadyHitActors.Contains(OtherActor)) return;

	UGameplayStatics::ApplyPointDamage(OtherActor, 16.f, GetActorForwardVector(), Hit, GetController(), this, nullptr);
	AlreadyHitActors.Add(OtherActor);
}

void ABossCharacter::OnLeftHitboxOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	if (AlreadyHitActors.Contains(OtherActor)) return;

	UGameplayStatics::ApplyPointDamage(OtherActor, 16.f, GetActorForwardVector(), Hit, GetController(), this, nullptr);
	AlreadyHitActors.Add(OtherActor);
}

void ABossCharacter::OnAnyDamageTaken(AActor*, float Damage, const UDamageType*, AController*, AActor*)
{
	Health = FMath::Max(0.f, Health - Damage);
	int32 NewPhase = Phase;

	if (Health <= 0.f)
	{
		// TODO: muerte del boss
	}
	else if (Health < 350.f) NewPhase = 3;
	else if (Health < 700.f) NewPhase = 2;

	if (NewPhase != Phase)
	{
		Phase = NewPhase;
		if (Phase == 2) { Cooldowns[EBossAttack::Meteors] = 10.f; }
		if (Phase == 3) { Cooldowns[EBossAttack::Meteors] = 8.f; }
	}
}

// ===== FUEGO =====
void ABossCharacter::StartFireBreath()
{
	FireCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (FireLoopSFX && !FireAudioComp)
	{
		FireAudioComp = UGameplayStatics::SpawnSoundAttached(FireLoopSFX, GetMesh(), MouthSocketName);
	}

	GetWorldTimerManager().SetTimer(FireTickHandle, this, &ABossCharacter::DealFireTick, FireTickInterval, true);
}

void ABossCharacter::StopFireBreath()
{
	GetWorldTimerManager().ClearTimer(FireTickHandle);
	FireCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (FireAudioComp)
	{
		FireAudioComp->Stop();
		FireAudioComp = nullptr;
	}
}

void ABossCharacter::DealFireTick()
{
	TArray<AActor*> HitActors;
	FireCapsule->GetOverlappingActors(HitActors, APawn::StaticClass());
	for (AActor* A : HitActors)
	{
		if (A && A != this)
		{
			UGameplayStatics::ApplyDamage(A, FireTickDamage, GetController(), this, nullptr);
		}
	}
}

// ===== METEORITOS =====
void ABossCharacter::TriggerMeteorWave(AActor* TargetHint)
{
	if (!MeteorWarningClass || !MeteorProjectileClass)
	{
		UE_LOG(LogBoss, Warning, TEXT("Meteor classes not assigned."));
		return;
	}

	const int32 Count = (Phase == 1) ? MeteorsCount_Phase1 : (Phase == 2 ? MeteorsCount_Phase2 : MeteorsCount_Phase3);
	const FVector Center = TargetHint ? TargetHint->GetActorLocation() : GetActorLocation();

	UE_LOG(LogBoss, Log, TEXT("TriggerMeteorWave: Phase=%d Count=%d"), Phase, Count);

	for (int32 i = 0; i < Count; ++i)
	{
		const float Radius = FMath::FRandRange(300.f, 900.f);
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const FVector Offset(FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius, 0.f);

		FVector Ground;
		if (!FindGroundBelow(Center + Offset + FVector(0, 0, 1200), Ground))
		{
			UE_LOG(LogBoss, Verbose, TEXT("FindGroundBelow failed, skipping meteor spot."));
			continue;
		}

		// Warning
		FTransform TW(FRotator::ZeroRotator, Ground);
		if (AMeteorWarning* W = GetWorld()->SpawnActorDeferred<AMeteorWarning>(MeteorWarningClass, TW, this, this))
		{
			W->WarningTime = MeteorWarningTime;
			W->FinishSpawning(TW);
		}

		// Meteor tras el warning
		FTimerHandle Tmp;
		GetWorldTimerManager().SetTimer(
			Tmp,
			FTimerDelegate::CreateUObject(this, &ABossCharacter::SpawnMeteorAt, Ground),
			MeteorWarningTime,
			false
		);
	}
}

void ABossCharacter::SpawnMeteorAt(FVector GroundPos)
{
	if (!MeteorProjectileClass) return;

	const FVector SpawnPos = GroundPos + FVector(0, 0, FMath::FRandRange(1800.f, 2800.f));
	const FTransform TM(FRotator::ZeroRotator, SpawnPos);

	if (AMeteorProjectile* P = GetWorld()->SpawnActorDeferred<AMeteorProjectile>(MeteorProjectileClass, TM, this, this))
	{
		P->TargetGround = GroundPos;
		P->FinishSpawning(TM);
	}
}

bool ABossCharacter::FindGroundBelow(const FVector& Start, FVector& OutGround, float MaxDrop) const
{
	FHitResult Hit;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(Boss_FindGround), false, this);
	const FVector End = Start - FVector(0, 0, MaxDrop);

	// 1) Visibility
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q))
	{
		OutGround = Hit.ImpactPoint;
		return true;
	}
	// 2) WorldStatic como fallback
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldStatic, Q))
	{
		OutGround = Hit.ImpactPoint;
		return true;
	}
	return false;
}

// ===== Distancias por ataque =====
float ABossCharacter::IdealMinDistance(EBossAttack A) const
{
	switch (A)
	{
	case EBossAttack::Jab:        return 100.f;   // Puño
	case EBossAttack::Swipe:      return 500.f;   // Carga
	case EBossAttack::Slam:       return 120.f;   // Suelo
	case EBossAttack::FireBreath: return 300.f;
	case EBossAttack::Meteors:    return 0.f;
	default:                      return 0.f;
	}
}
float ABossCharacter::IdealMaxDistance(EBossAttack A) const
{
	switch (A)
	{
	case EBossAttack::Jab:        return 250.f;
	case EBossAttack::Swipe:      return 1200.f;  // compacto para favorecer persecución
	case EBossAttack::Slam:       return 350.f;
	case EBossAttack::FireBreath: return 800.f;
	case EBossAttack::Meteors:    return 5000.f;
	default:                      return 99999.f;
	}
}

EBossAttack ABossCharacter::RandomWeightedAttack(float Dist, const AActor*) const
{
	TArray<EBossAttack> Pool;
	auto TryAdd = [&](EBossAttack A, int32 Weight)
		{
			if (!CanUseAttack(A)) return;
			// Meteors puede no tener montage; los demás sí deben tener
			if (A != EBossAttack::Meteors && !GetMontageFor(A)) return;
			if (Dist < IdealMinDistance(A) || Dist > IdealMaxDistance(A)) return;
			for (int32 i = 0;i < Weight;i++) Pool.Add(A);
		};

	if (Phase == 1)
	{
		TryAdd(EBossAttack::Jab, 6);
		TryAdd(EBossAttack::Slam, 3);
		TryAdd(EBossAttack::Swipe, 2);
		TryAdd(EBossAttack::Meteors, 1); // leve para test
	}
	else if (Phase == 2)
	{
		TryAdd(EBossAttack::Jab, 4);
		TryAdd(EBossAttack::Slam, 3);
		TryAdd(EBossAttack::Swipe, 4);
		TryAdd(EBossAttack::FireBreath, 3);
		TryAdd(EBossAttack::Meteors, 2);
	}
	else
	{
		TryAdd(EBossAttack::Jab, 3);
		TryAdd(EBossAttack::Slam, 3);
		TryAdd(EBossAttack::Swipe, 4);
		TryAdd(EBossAttack::FireBreath, 4);
		TryAdd(EBossAttack::Meteors, 4);
	}

	return Pool.Num() ? Pool[FMath::RandRange(0, Pool.Num() - 1)] : EBossAttack::None;
}
