// PlayerCharacter.cpp
#include "PlayerCharacter.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
// #include "Blueprint/UserWidget.h"   // Descomenta y añade "UMG" al .Build.cs si vas a crear HUD aquí

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentHealth = MaxHealth;
	CurrentEnergy = MaxEnergy;

	/* ====== HITBOX DEFAULT (C++) ====== */
	MeleeHitbox_Default = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitbox_Default"));
	MeleeHitbox_Default->SetupAttachment(GetMesh());                 // hija del mesh
	MeleeHitbox_Default->InitBoxExtent(MeleeBoxExtent);
	MeleeHitbox_Default->SetCollisionEnabled(ECollisionEnabled::NoCollision); // se enciende en StartMeleeWindow
	MeleeHitbox_Default->SetGenerateOverlapEvents(true);
	MeleeHitbox_Default->SetCollisionObjectType(ECC_WorldDynamic);
	MeleeHitbox_Default->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeHitbox_Default->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	CurrentEnergy = FMath::Clamp(CurrentEnergy, 0.f, MaxEnergy);

	/* ====== Resolver la hitbox elegida en el BP (picker) ====== */
	UBoxComponent* PickedBox = nullptr;
	if (MeleeHitboxRef.ComponentProperty != NAME_None)
	{
		if (UActorComponent* Comp = MeleeHitboxRef.GetComponent(this))
		{
			PickedBox = Cast<UBoxComponent>(Comp);
		}
	}

	/* ====== Fallback: auto-detectar por Tag o nombre ======
	   - Asigna a tu Box (hija de la espada) el Component Tag: "MeleeHitboxBP"
		 o nómbrala "MeleeHitbox_BP" y la encontrará sola.
	*/
	if (!PickedBox)
	{
		TArray<UBoxComponent*> Boxes;
		GetComponents<UBoxComponent>(Boxes);
		for (UBoxComponent* Box : Boxes)
		{
			if (!Box || Box == MeleeHitbox_Default) continue;
			if (Box->ComponentHasTag(FName("MeleeHitboxBP")) || Box->GetName().Contains(TEXT("MeleeHitbox")))
			{
				PickedBox = Box;
				break;
			}
		}
	}

	/* ====== Selección final ====== */
	ActiveMeleeHitbox = PickedBox ? PickedBox : MeleeHitbox_Default;

	// Apaga la default si no se usa (evitar overlaps duplicados)
	if (MeleeHitbox_Default && ActiveMeleeHitbox != MeleeHitbox_Default)
	{
		MeleeHitbox_Default->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeleeHitbox_Default->SetGenerateOverlapEvents(false);
	}

	// Config base + bind del overlap en la activa
	if (ActiveMeleeHitbox)
	{
		ActiveMeleeHitbox->SetGenerateOverlapEvents(true);
		ActiveMeleeHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // OFF por defecto
		ActiveMeleeHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
		ActiveMeleeHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		ActiveMeleeHitbox->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnMeleeHitboxBeginOverlap);
	}

	/* ====== (Opcional) Crear HUD aquí si lo necesitas ======
	if (HUDWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			HUDWidgetInstance = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUDWidgetInstance) { HUDWidgetInstance->AddToViewport(); }
		}
	}
	*/
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bAutoRegenEnergy) { RegenerateEnergy(DeltaTime); }
}

/* =================== STATS HELPERS =================== */

void APlayerCharacter::ApplyDamageAmount(float Damage)
{
	if (Damage <= 0.f) return;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	// if (CurrentHealth <= 0.f) { /* muerte */ }
}

void APlayerCharacter::AddHealth(float Amount)
{
	if (FMath::IsNearlyZero(Amount)) return;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
}

bool APlayerCharacter::TryConsumeEnergy(float Amount)
{
	if (Amount <= 0.f) return true;
	if (CurrentEnergy + KINDA_SMALL_NUMBER >= Amount)
	{
		CurrentEnergy = FMath::Clamp(CurrentEnergy - Amount, 0.f, MaxEnergy);
		return true;
	}
	return false;
}

void APlayerCharacter::AddEnergy(float Amount)
{
	if (FMath::IsNearlyZero(Amount)) return;
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.f, MaxEnergy);
}

void APlayerCharacter::RegenerateEnergy(float DeltaTime)
{
	if (CurrentEnergy < MaxEnergy && EnergyRegenPerSec > 0.f)
	{
		CurrentEnergy = FMath::Clamp(CurrentEnergy + EnergyRegenPerSec * DeltaTime, 0.f, MaxEnergy);
	}
}

/* =================== COMBATE (MELEE) =================== */

void APlayerCharacter::StartMeleeWindow()
{
	AlreadyHit.Empty();
	if (ActiveMeleeHitbox)
	{
		ActiveMeleeHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // solo overlaps
	}
}

void APlayerCharacter::EndMeleeWindow()
{
	if (ActiveMeleeHitbox)
	{
		ActiveMeleeHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APlayerCharacter::OnMeleeHitboxBeginOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/
)
{
	if (!OtherActor || OtherActor == this) return;

	// Filtra por Tag "Enemy" (pon ese tag en tus BP_Enemy)
	if (!OtherActor->ActorHasTag(FName("Enemy"))) return;

	// Evita multi-golpes
	if (AlreadyHit.Contains(OtherActor)) return;

	// Aplica daño
	if (AController* InstigatorController = GetController())
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			MeleeDamage,
			InstigatorController,
			this,        // Damage Causer
			nullptr      // DamageType
		);
	}

	AlreadyHit.Add(OtherActor);
}

/* ========== AÑADIDOS (no tocan tu lógica existente) ========== */

void APlayerCharacter::SetMeleeDamageTemporarily(float NewDamage)
{
	// Guarda el original solo la primera vez que se pida temporal
	if (!bUsingTempMeleeDamage)
	{
		SavedMeleeDamage = MeleeDamage;
		bUsingTempMeleeDamage = true;
	}
	MeleeDamage = FMath::Max(0.f, NewDamage);
}

void APlayerCharacter::RestoreMeleeDamage()
{
	if (bUsingTempMeleeDamage)
	{
		MeleeDamage = SavedMeleeDamage;
		bUsingTempMeleeDamage = false;
	}
}

void APlayerCharacter::StartMeleeWindowWithDamage(float Damage)
{
	SetMeleeDamageTemporarily(Damage);
	StartMeleeWindow(); // usa tu StartMeleeWindow original
}

void APlayerCharacter::EndMeleeWindowAndRestore()
{
	EndMeleeWindow();   // tu EndMeleeWindow original
	RestoreMeleeDamage();
}
float APlayerCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Dmg = FMath::Max(0.f, DamageAmount);
	if (Dmg <= 0.f) return 0.f;

	// 1) Primero baja la vida
	ApplyDamageAmount(Dmg);   // aquí reduces CurrentHealth

	// 2) Después llama al Super para que dispare Event AnyDamage en BP
	Super::TakeDamage(Dmg, DamageEvent, EventInstigator, DamageCauser);

	return Dmg;
}

