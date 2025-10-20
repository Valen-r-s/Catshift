// ShieldComponent.cpp

#include "ShieldComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h" // EKeys

UShieldComponent::UShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		PC = Cast<APlayerController>(OwnerCharacter->GetController());
	}

	// Reintenta hasta que exista el InputComponent del owner
	GetWorld()->GetTimerManager().SetTimer(TH_BindInputRetry, this, &UShieldComponent::BindInput, 0.05f, true);
}

void UShieldComponent::BindInput()
{
	if (!OwnerCharacter.IsValid())
		return;

	if (!PC.IsValid())
	{
		PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (!PC.IsValid()) return;
	}

	if (!OwnerCharacter->InputComponent)
		return;

	if (!bInputBound)
	{
		OwnerCharacter->InputComponent->BindAction("Shield", IE_Pressed, this, &UShieldComponent::StartShield);
		OwnerCharacter->InputComponent->BindAction("Shield", IE_Released, this, &UShieldComponent::EndShield);
		bInputBound = true;
	}

	GetWorld()->GetTimerManager().ClearTimer(TH_BindInputRetry);
}

void UShieldComponent::StartShield()
{
	if (bShieldActive || bOnCooldown || !OwnerCharacter.IsValid())
		return;

	bShieldActive = true;

	// Invulnerabilidad
	OwnerCharacter->SetCanBeDamaged(false);

	// Bloqueo de controles
	LockControls(true);

	// Visual/colisión + tamaño correcto
	SpawnRuntimeComponents();
	SyncVisualToRadius();

	// Sphere cast inicial
	DoSphereCast();

	// Audio
	StartLoopAudio();

	// Límite de tiempo -> al cumplirse, llamamos EndShield() (así entra cooldown)
	if (MaxHoldSeconds > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(TH_MaxHold, this, &UShieldComponent::EndShield, MaxHoldSeconds, false);
	}
}

void UShieldComponent::EndShield()
{
	if (!bShieldActive || !OwnerCharacter.IsValid())
		return;

	// Limpia tiempo de hold si lo había
	GetWorld()->GetTimerManager().ClearTimer(TH_MaxHold);

	ForceEndShield();

	// Cooldown SIEMPRE después de terminar (por suelta o por tiempo)
	if (CooldownSeconds > 0.f)
	{
		bOnCooldown = true;
		OnCooldownStart.Broadcast(CooldownSeconds);

		GetWorld()->GetTimerManager().SetTimer(TH_Cooldown, [this]()
			{
				bOnCooldown = false;
				OnCooldownEnd.Broadcast();
			}, CooldownSeconds, false);
	}
}

void UShieldComponent::ForceEndShield()
{
	if (!bShieldActive || !OwnerCharacter.IsValid())
		return;

	bShieldActive = false;

	OwnerCharacter->SetCanBeDamaged(true);
	LockControls(false);
	DestroyRuntimeComponents();
	StopLoopAudio();
}

void UShieldComponent::DoSphereCast()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector Center = Owner->GetActorLocation();

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), Center, SphereRadius, 24, FColor::Cyan, false, 1.5f, 0, 2.0f);
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> Ignore; Ignore.Add(Owner);
	TArray<AActor*> Hits;

	if (UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Center, SphereRadius,
		ObjectTypes, nullptr, Ignore, Hits))
	{
		OnShieldSphereHit.Broadcast(Hits);
	}
}

void UShieldComponent::SpawnRuntimeComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Colisión persistente (opcional)
	if (bUsePersistentCollision && !ShieldSphere)
	{
		ShieldSphere = NewObject<USphereComponent>(Owner, TEXT("ShieldSphereRuntime"));
		ShieldSphere->InitSphereRadius(SphereRadius);
		ShieldSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ShieldSphere->SetCollisionObjectType(ECC_WorldDynamic);
		ShieldSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		ShieldSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		ShieldSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		ShieldSphere->RegisterComponent();
		ShieldSphere->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ShieldSphere->SetRelativeLocation(FVector::ZeroVector);
	}

	// Visual opcional
	if (ShieldStaticMesh && !ShieldFX)
	{
		ShieldFX = NewObject<UStaticMeshComponent>(Owner, TEXT("ShieldFXRuntime"));
		ShieldFX->SetStaticMesh(ShieldStaticMesh);
		ShieldFX->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldFX->RegisterComponent();
		ShieldFX->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UShieldComponent::DestroyRuntimeComponents()
{
	if (ShieldSphere)
	{
		ShieldSphere->DestroyComponent();
		ShieldSphere = nullptr;
	}
	if (ShieldFX)
	{
		ShieldFX->DestroyComponent();
		ShieldFX = nullptr;
	}
}

void UShieldComponent::SyncVisualToRadius()
{
	// Actualizar colisión persistente
	if (ShieldSphere)
	{
		ShieldSphere->SetSphereRadius(SphereRadius, true);
	}

	// Ajustar malla visual
	if (ShieldFX && bAutoScaleVisual)
	{
		// 1) Escala uniforme para que el radio de la malla == SphereRadius
		float MeshRadius = 50.f;
		if (UStaticMesh* SM = ShieldFX->GetStaticMesh())
		{
			MeshRadius = FMath::Max(1.f, SM->GetBounds().SphereRadius);
		}
		const float UniformScale = SphereRadius / MeshRadius;
		ShieldFX->SetWorldScale3D(FVector(UniformScale));

		// 2) Offset Z (opcionalmente alinear al suelo)
		float DesiredZ = VisualZOffset;

		if (bAlignBottomToFloor)
		{
			float CapsuleHalfHeight = 0.f;
			if (OwnerCharacter.IsValid())
			{
				if (UCapsuleComponent* Cap = OwnerCharacter->GetCapsuleComponent())
				{
					CapsuleHalfHeight = Cap->GetScaledCapsuleHalfHeight();
				}
			}
			DesiredZ += (SphereRadius - CapsuleHalfHeight);
		}

		if (ShieldFX)
		{
			ShieldFX->SetRelativeLocation(FVector(0.f, 0.f, DesiredZ));
		}
		if (ShieldSphere)
		{
			ShieldSphere->SetRelativeLocation(FVector(0.f, 0.f, DesiredZ));
		}
	}
}

void UShieldComponent::LockControls(bool bLock)
{
	if (!OwnerCharacter.IsValid())
		return;

	// 1) Movement
	if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
	{
		if (bLock)
		{
			Move->StopMovementImmediately();
			Move->DisableMovement();
		}
		else
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}

	// 2) Ignorar input de movimiento/cámara
	if (PC.IsValid())
	{
		PC->SetIgnoreMoveInput(bLock);
		PC->SetIgnoreLookInput(bLock);
	}

	// 3) Capturador de input (consume todo salvo Space Release)
	if (bLock)
	{
		if (!ShieldInputCapture && PC.IsValid())
		{
			ShieldInputCapture = NewObject<UInputComponent>(OwnerCharacter.Get(), TEXT("ShieldInputCapture"));
			ShieldInputCapture->RegisterComponent();

			// Dejar soltar el escudo
			{
				FInputKeyBinding SpaceRelease(EKeys::SpaceBar, IE_Released);
				SpaceRelease.bConsumeInput = true;
				SpaceRelease.KeyDelegate.GetDelegateForManualSet().BindLambda([this]()
					{
						this->EndShield();
					});
				ShieldInputCapture->KeyBindings.Add(SpaceRelease);
			}
			// Consumir todo lo demás (press/release)
			{
				FInputKeyBinding AnyPress(EKeys::AnyKey, IE_Pressed);
				AnyPress.bConsumeInput = true;
				AnyPress.KeyDelegate.GetDelegateForManualSet().BindLambda([]() {});
				ShieldInputCapture->KeyBindings.Add(AnyPress);

				FInputKeyBinding AnyRelease(EKeys::AnyKey, IE_Released);
				AnyRelease.bConsumeInput = true;
				AnyRelease.KeyDelegate.GetDelegateForManualSet().BindLambda([]() {});
				ShieldInputCapture->KeyBindings.Add(AnyRelease);
			}

			PC->PushInputComponent(ShieldInputCapture);
		}
	}
	else
	{
		if (PC.IsValid() && ShieldInputCapture)
		{
			PC->PopInputComponent(ShieldInputCapture);
			ShieldInputCapture->DestroyComponent();
			ShieldInputCapture = nullptr;
		}
	}

	// 4) Parar montajes si activamos
	if (bLock && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->StopAllMontages(0.15f);
	}
}

/* ==================== AUDIO ==================== */

void UShieldComponent::SetShieldLoopSound(USoundBase* InSound, float InVolume)
{
	ShieldLoopSound = InSound;
	ShieldLoopVolume = FMath::Max(0.f, InVolume);
}

void UShieldComponent::StartLoopAudio()
{
	if (!ShieldLoopSound || !OwnerCharacter.IsValid())
		return;

	if (!ShieldLoopAC)
	{
		ShieldLoopAC = NewObject<UAudioComponent>(OwnerCharacter.Get(), TEXT("ShieldLoopAudio"));
		ShieldLoopAC->bAutoActivate = false;
		ShieldLoopAC->bAutoDestroy = false;
		ShieldLoopAC->SetSound(ShieldLoopSound);
		ShieldLoopAC->SetVolumeMultiplier(ShieldLoopVolume);
		ShieldLoopAC->OnAudioFinished.AddDynamic(this, &UShieldComponent::OnShieldLoopFinished);
		ShieldLoopAC->RegisterComponent();
		ShieldLoopAC->AttachToComponent(OwnerCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	ShieldLoopAC->Play(0.f);
}

void UShieldComponent::StopLoopAudio()
{
	if (ShieldLoopAC)
	{
		ShieldLoopAC->OnAudioFinished.RemoveAll(this);
		ShieldLoopAC->Stop();
		ShieldLoopAC->DestroyComponent();
		ShieldLoopAC = nullptr;
	}
}

void UShieldComponent::OnShieldLoopFinished()
{
	// Repetir manualmente si el asset no es looping
	if (bShieldActive && ShieldLoopAC)
	{
		ShieldLoopAC->Play(0.f);
	}
}

/* ==================== COOLdown getters (HUD) ==================== */

float UShieldComponent::GetCooldownRemaining() const
{
	if (!bOnCooldown || !GetWorld()) return 0.f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(TH_Cooldown);
}

float UShieldComponent::GetCooldownRatio() const
{
	const float Remaining = GetCooldownRemaining();
	return (CooldownSeconds > 0.f) ? (Remaining / CooldownSeconds) : 0.f;
}
