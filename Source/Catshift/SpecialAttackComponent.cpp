// SpecialAttackComponent.cpp
#include "SpecialAttackComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/EngineTypes.h"      // ECC_*
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Camera/CameraActor.h"

USpecialAttackComponent::USpecialAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void USpecialAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// Fuerza SOLO el TargetMarker oculto al iniciar (no toca otros widgets)
	ShowAllEnemyMarkers(false);
}

ACharacter* USpecialAttackComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

bool USpecialAttackComponent::IsEnemyCharacter(AActor* Actor) const
{
	if (!Actor) return false;
	if (!Actor->ActorHasTag(FName("Enemy"))) return false;
	return Cast<ACharacter>(Actor) != nullptr;
}

UWidgetComponent* USpecialAttackComponent::FindEnemyMarker(ACharacter* Enemy) const
{
	if (!Enemy) return nullptr;
	if (EnemyWidgetComponentName.IsNone()) return nullptr; // estrictos: sin nombre no tocamos nada

	TArray<UWidgetComponent*> Comps;
	Enemy->GetComponents<UWidgetComponent>(Comps);
	for (UWidgetComponent* C : Comps)
	{
		if (C && C->GetFName() == EnemyWidgetComponentName)
		{
			return C; // solo exacto
		}
	}
	return nullptr; // no encontrado => NO tocar otros
}

void USpecialAttackComponent::ShowAllEnemyMarkers(bool bShow)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), EnemyActors);

	for (AActor* A : EnemyActors)
	{
		ACharacter* EnemyChar = Cast<ACharacter>(A);
		if (!EnemyChar) continue;

		if (UWidgetComponent* WidgetComp = FindEnemyMarker(EnemyChar))
		{
			WidgetComp->SetHiddenInGame(!bShow, true);

			// Si mostramos, reset visual a “no seleccionado”
			if (bShow)
			{
				if (UUserWidget* W = WidgetComp->GetUserWidgetObject())
				{
					static const FName FuncName("SetSelectedOrder");
					if (W->GetClass()->FindFunctionByName(FuncName))
					{
						struct { int32 Order; bool bSelected; } Params{ -1, false };
						W->ProcessEvent(W->GetClass()->FindFunctionByName(FuncName), &Params);
					}
				}
			}
		}
	}
}

void USpecialAttackComponent::UpdateEnemyMarker(ACharacter* Enemy, bool bSelected, int32 OrderIndex)
{
	if (!Enemy) return;

	if (UWidgetComponent* WidgetComp = FindEnemyMarker(Enemy))
	{
		if (UUserWidget* W = WidgetComp->GetUserWidgetObject())
		{
			static const FName FuncName("SetSelectedOrder");
			if (W->GetClass()->FindFunctionByName(FuncName))
			{
				struct { int32 Order; bool bSelected; } Params{ OrderIndex, bSelected };
				W->ProcessEvent(W->GetClass()->FindFunctionByName(FuncName), &Params);
			}
		}
	}
}

void USpecialAttackComponent::ReindexAllMarkers()
{
	for (int32 i = 0; i < SelectedTargets.Num(); ++i)
	{
		if (ACharacter* Enemy = SelectedTargets[i].Get())
		{
			UpdateEnemyMarker(Enemy, true, i + 1); // 1-based
		}
	}
}

void USpecialAttackComponent::StartSpecialAttack()
{
	if (bIsTargeting || bIsExecuting) return;

	SelectedTargets.Empty();
	ValidTargetsThisRun.Empty();
	CurrentTargetIndex = INDEX_NONE;

	// Slow motion global
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, GlobalTimeDilationDuringTargeting);
	}

	// Mostrar SOLO los TargetMarker
	ShowAllEnemyMarkers(true);

	// Contador en tiempo REAL (no afectado por dilation)
	bIsTargeting = true;
	TargetingStartRealSeconds = FPlatformTime::Seconds();
	SetComponentTickEnabled(true);
}

void USpecialAttackComponent::HandleClickUnderCursor()
{
	if (!bIsTargeting) return;

	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		AActor* HitActor = Hit.GetActor();
		if (IsEnemyCharacter(HitActor))
		{
			ACharacter* Enemy = Cast<ACharacter>(HitActor);

			// ¿Ya está seleccionado?
			int32 FoundIdx = INDEX_NONE;
			for (int32 i = 0; i < SelectedTargets.Num(); ++i)
			{
				if (SelectedTargets[i].Get() == Enemy)
				{
					FoundIdx = i; break;
				}
			}

			if (FoundIdx == INDEX_NONE)
			{
				SelectedTargets.Add(Enemy);
				UpdateEnemyMarker(Enemy, true, SelectedTargets.Num()); // 1-based
			}
			else
			{
				if (ACharacter* E = SelectedTargets[FoundIdx].Get())
				{
					UpdateEnemyMarker(E, false, -1);
				}
				SelectedTargets.RemoveAt(FoundIdx);
				ReindexAllMarkers();
			}
		}
	}
}

void USpecialAttackComponent::EndTargeting()
{
	// Ocultar SOLO los TargetMarker y restaurar tiempo
	ShowAllEnemyMarkers(false);

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	}

	SetComponentTickEnabled(false);
	bIsTargeting = false;

	// Preparar ejecución
	ValidTargetsThisRun = SelectedTargets;

	if (ValidTargetsThisRun.Num() == 0)
	{
		return; // nada que ejecutar
	}

	// Congelar enemigos mientras dure la secuencia
	SetEnemiesFrozen(true);

	bIsExecuting = true;
	CurrentTargetIndex = -1;
	RunNextOrFinish();
}

void USpecialAttackComponent::RunNextOrFinish()
{
	++CurrentTargetIndex;

	if (!bIsExecuting)
		return;

	if (CurrentTargetIndex >= ValidTargetsThisRun.Num())
	{
		ApplyFinalBurst();
		bIsExecuting = false;
		return;
	}

	ACharacter* Target = ValidTargetsThisRun[CurrentTargetIndex].Get();
	if (!IsValid(Target))
	{
		RunNextOrFinish();
		return;
	}

	// Orientar hacia el objetivo
	if (ACharacter* OwnerChar = GetOwnerCharacter())
	{
		const FVector Dir = Target->GetActorLocation() - OwnerChar->GetActorLocation();
		const FRotator NewRot = UKismetMathLibrary::MakeRotFromX(Dir.GetSafeNormal());
		OwnerChar->SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));
	}

	PlayRandomMontage();
}

void USpecialAttackComponent::PlayRandomMontage()
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) { RunNextOrFinish(); return; }

	if (Montages.Num() == 0) { RunNextOrFinish(); return; }

	const int32 Index = FMath::RandRange(0, Montages.Num() - 1);
	UAnimMontage* Montage = Montages[Index].IsValid() ? Montages[Index].Get() : Montages[Index].LoadSynchronous();

	if (!Montage)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("SpecialAttack: Montage null"));
		// Fallback: dash + smallhit + next
		Notify_DashBegin();
		FTimerHandle Tmp;
		GetWorld()->GetTimerManager().SetTimer(Tmp, [this]()
			{
				Notify_SmallHit();
				Notify_Next();
			}, 0.15f, false);
		return;
	}

	if (USkeletalMeshComponent* Mesh = OwnerChar->GetMesh())
	{
		if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
		{
			const float PlayRes = AnimInst->Montage_Play(Montage, 1.0f);
			if (PlayRes <= 0.f)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("SpecialAttack: Montage_Play failed (¿Slot ausente / skeleton mismatch?)"));
				// Fallback: dash + smallhit + next
				Notify_DashBegin();
				FTimerHandle Tmp;
				GetWorld()->GetTimerManager().SetTimer(Tmp, [this]()
					{
						Notify_SmallHit();
						Notify_Next();
					}, 0.15f, false);
			}
			else
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("SpecialAttack: Playing montage %s"), *Montage->GetName()));
			}
		}
	}
}

void USpecialAttackComponent::DashToTarget(ACharacter* Target)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar || !Target) return;

	const FVector OwnerLoc = OwnerChar->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();

	FVector ToTarget = TargetLoc - OwnerLoc;
	const float Dist = ToTarget.Size();
	if (Dist <= KINDA_SMALL_NUMBER) return;

	const FVector Dir = ToTarget / Dist;

	if (bUseTeleportDash)
	{
		// Teleport a ~120uu frente al enemigo (debug)
		const FVector Dest = TargetLoc - Dir * 120.f;
		OwnerChar->SetActorLocation(Dest, true);
		return;
	}

	// Impulso
	const FVector LaunchVel = Dir * DashSpeed;
	OwnerChar->LaunchCharacter(LaunchVel, true, false);
}

void USpecialAttackComponent::ApplySmallHitToTarget(ACharacter* Target)
{
	if (!IsValid(Target)) return;

	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	const float Dist = FVector::Dist(OwnerChar->GetActorLocation(), Target->GetActorLocation());
	if (Dist > ApproachRadius * 1.5f) return;

	UGameplayStatics::ApplyDamage(Target, SmallHitDamage, OwnerChar->GetController(), OwnerChar, nullptr);

	// ===== FX por golpe pequeño =====
	if (SmallHitFX.IsValid() || !SmallHitFX.ToSoftObjectPath().IsNull())
	{
		UNiagaraSystem* FX = SmallHitFX.IsValid() ? SmallHitFX.Get() : SmallHitFX.LoadSynchronous();
		if (FX)
		{
			if (USkeletalMeshComponent* EnemyMesh = Target->GetMesh())
			{
				if (SmallHitFXSocketName != NAME_None && EnemyMesh->DoesSocketExist(SmallHitFXSocketName))
				{
					UNiagaraFunctionLibrary::SpawnSystemAttached(
						FX, EnemyMesh, SmallHitFXSocketName,
						FVector::ZeroVector, FRotator::ZeroRotator,
						EAttachLocation::SnapToTarget, true);
				}
				else
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), FX, EnemyMesh->GetComponentLocation(),
						FRotator::ZeroRotator);
				}
			}
			else
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(), FX, Target->GetActorLocation(), FRotator::ZeroRotator);
			}
		}
	}
}

void USpecialAttackComponent::ApplyFinalBurst()
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	// ===== FX (y daño) en TODOS los enemigos seleccionados =====
	UNiagaraSystem* FinalFXAsset = nullptr;
	if (FinalBurstFX.IsValid() || !FinalBurstFX.ToSoftObjectPath().IsNull())
	{
		FinalFXAsset = FinalBurstFX.IsValid() ? FinalBurstFX.Get() : FinalBurstFX.LoadSynchronous();
	}

	for (const TWeakObjectPtr<ACharacter>& WeakT : ValidTargetsThisRun)
	{
		if (ACharacter* T = WeakT.Get())
		{
			UGameplayStatics::ApplyDamage(T, FinalBurstDamage, OwnerChar->GetController(), OwnerChar, nullptr);

			if (FinalFXAsset)
			{
				if (USkeletalMeshComponent* EnemyMesh = T->GetMesh())
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), FinalFXAsset, EnemyMesh->GetComponentLocation(), FRotator::ZeroRotator);
				}
				else
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), FinalFXAsset, T->GetActorLocation(), FRotator::ZeroRotator);
				}
			}
		}
	}

	// ===== Cambio de cámara de cierre =====
	StartFinalCameraFocus();

	// Asegurar restauración de colisiones/ignores y descongelar
	SetPassThroughEnemies(false);
	SetEnemiesFrozen(false);

	SelectedTargets.Empty();
	ValidTargetsThisRun.Empty();
	CurrentTargetIndex = INDEX_NONE;
}

void USpecialAttackComponent::UpdateMoveIgnoreEnemies(bool bEnable)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	UCapsuleComponent* Capsule = OwnerChar->GetCapsuleComponent();
	if (!Capsule) return;

	if (bEnable)
	{
		MoveIgnoredList.Empty();

		UWorld* World = GetWorld();
		if (!World) return;

		TArray<AActor*> Enemies;
		UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), Enemies);

		for (AActor* Enemy : Enemies)
		{
			if (!Enemy || Enemy == OwnerChar) continue;

			Capsule->IgnoreActorWhenMoving(Enemy, true);
			OwnerChar->MoveIgnoreActorAdd(Enemy);

			MoveIgnoredList.Add(Enemy);
		}
	}
	else
	{
		for (const TWeakObjectPtr<AActor>& Weak : MoveIgnoredList)
		{
			if (AActor* Enemy = Weak.Get())
			{
				Capsule->IgnoreActorWhenMoving(Enemy, false);
				OwnerChar->MoveIgnoreActorRemove(Enemy);
			}
		}
		MoveIgnoredList.Empty();
	}
}

void USpecialAttackComponent::SetPassThroughEnemies(bool bEnable)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	UCapsuleComponent* Capsule = OwnerChar->GetCapsuleComponent();
	if (!Capsule) return;

	if (bEnable)
	{
		if (!bSavedPawnResponseValid)
		{
			SavedPawnResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
			bSavedPawnResponseValid = true;
		}
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		// También ignorar a los enemigos en el sweep de movimiento
		UpdateMoveIgnoreEnemies(true);
	}
	else
	{
		if (bSavedPawnResponseValid)
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnResponse);
		}
		else
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}

		UpdateMoveIgnoreEnemies(false);
	}
}

void USpecialAttackComponent::SetEnemiesFrozen(bool bFreeze)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (bFreeze)
	{
		FrozenEnemies.Empty();

		TArray<AActor*> EnemyActors;
		UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), EnemyActors);

		for (AActor* A : EnemyActors)
		{
			ACharacter* EnemyChar = Cast<ACharacter>(A);
			if (!EnemyChar) continue;

			AAIController* AIC = Cast<AAIController>(EnemyChar->GetController());
			UBrainComponent* Brain = AIC ? AIC->BrainComponent : nullptr;

			FEnemyFreezeBackup& Backup = FrozenEnemies.Add(EnemyChar);
			if (UCharacterMovementComponent* Move = EnemyChar->GetCharacterMovement())
			{
				Backup.PrevMode = Move->MovementMode;
				Backup.PrevMaxWalkSpeed = Move->MaxWalkSpeed;

				Move->StopMovementImmediately();
				Move->DisableMovement(); // MovementMode = MOVE_None
			}

			if (AIC) { AIC->StopMovement(); }
			if (Brain)
			{
				Backup.bBrainWasRunning = Brain->IsRunning();
				Brain->StopLogic(TEXT("SpecialAttack_Freeze"));
			}
		}
	}
	else
	{
		for (auto& It : FrozenEnemies)
		{
			ACharacter* EnemyChar = It.Key.Get();
			const FEnemyFreezeBackup& Backup = It.Value;
			if (!EnemyChar) continue;

			if (UCharacterMovementComponent* Move = EnemyChar->GetCharacterMovement())
			{
				// Restaurar modo y velocidad previos
				if (Backup.PrevMode == MOVE_None) { Move->SetMovementMode(MOVE_Walking); }
				else { Move->SetMovementMode(Backup.PrevMode); }

				if (Backup.PrevMaxWalkSpeed > 0.f) { Move->MaxWalkSpeed = Backup.PrevMaxWalkSpeed; }
			}

			if (AAIController* AIC = Cast<AAIController>(EnemyChar->GetController()))
			{
				if (UBrainComponent* Brain = AIC->BrainComponent)
				{
					if (Backup.bBrainWasRunning) { Brain->RestartLogic(); }
				}
			}
		}
		FrozenEnemies.Empty();
	}
}

void USpecialAttackComponent::StartFinalCameraFocus()
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	// Si ya hay una cámara temporal previa, límpiala
	if (TempFinalCam.IsValid())
	{
		if (PC->GetViewTarget() == TempFinalCam.Get())
		{
			PC->SetViewTargetWithBlend(OwnerChar, FinalCamBlendTime);
		}
		TempFinalCam->Destroy();
		TempFinalCam = nullptr;
	}

	// Colocar cámara detrás del personaje mirando hacia él
	const FRotator OwnerYaw(0.f, OwnerChar->GetActorRotation().Yaw, 0.f);
	const FTransform Basis(OwnerYaw, OwnerChar->GetActorLocation());
	const FVector CamLoc = Basis.TransformPosition(FinalCamOffset);
	const FRotator CamRot = (OwnerChar->GetActorLocation() - CamLoc).Rotation() + FinalCamRotOffset;

	ACameraActor* Cam = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CamLoc, CamRot);
	if (!Cam) return;

	TempFinalCam = Cam;

	// Blend hacia la cámara temporal
	PC->SetViewTargetWithBlend(Cam, FinalCamBlendTime);

	// Programar regreso al jugador
	FTimerHandle H;
	World->GetTimerManager().SetTimer(H, this, &USpecialAttackComponent::EndFinalCameraFocus,
		FinalCamHoldTime, false);
}

void USpecialAttackComponent::EndFinalCameraFocus()
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
	{
		PC->SetViewTargetWithBlend(OwnerChar, FinalCamBlendTime);
	}

	// Destruir la cámara temporal un poco después para no cortar el blend
	if (UWorld* World = GetWorld())
	{
		if (TempFinalCam.IsValid())
		{
			ACameraActor* Cam = TempFinalCam.Get();
			FTimerHandle H;
			World->GetTimerManager().SetTimer(H, [Cam]()
				{
					if (IsValid(Cam)) Cam->Destroy();
				}, FinalCamBlendTime + 0.05f, false);
		}
	}
	TempFinalCam = nullptr;
}

void USpecialAttackComponent::Notify_DashBegin()
{
	if (!bIsExecuting) return;
	if (!ValidTargetsThisRun.IsValidIndex(CurrentTargetIndex)) return;

	SetPassThroughEnemies(true); // atravesar enemigos durante el dash

	if (ACharacter* Target = ValidTargetsThisRun[CurrentTargetIndex].Get())
	{
		DashToTarget(Target);
	}
}

void USpecialAttackComponent::Notify_SmallHit()
{
	if (!bIsExecuting) return;
	if (!ValidTargetsThisRun.IsValidIndex(CurrentTargetIndex)) return;

	SetPassThroughEnemies(false); // restaurar antes del impacto

	if (ACharacter* Target = ValidTargetsThisRun[CurrentTargetIndex].Get())
	{
		ApplySmallHitToTarget(Target);
	}
}

void USpecialAttackComponent::Notify_Next()
{
	SetPassThroughEnemies(false); // seguridad por si faltó SmallHit
	RunNextOrFinish();
}

void USpecialAttackComponent::Notify_FinalBurst()
{
	ApplyFinalBurst();
	bIsExecuting = false;
}

void USpecialAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTargeting)
	{
		const double Now = FPlatformTime::Seconds();
		if ((Now - TargetingStartRealSeconds) >= (double)TargetingDurationSeconds)
		{
			EndTargeting();
		}
	}
}
