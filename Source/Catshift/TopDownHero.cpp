#include "TopDownHero.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATopDownHero::ATopDownHero()
{
    PrimaryActorTick.bCanEverTick = true;

    // ===== Cámara top‑down fija =====
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 800.f;
    SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    SpringArm->bUsePawnControlRotation = false; // cámara no gira con el pawn
    SpringArm->bDoCollisionTest = false;

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(SpringArm);
    TopDownCamera->bUsePawnControlRotation = false;

    // ===== Movimiento estilo LoL =====
    bUseControllerRotationYaw = false;                          // el controlador no dicta yaw
    GetCharacterMovement()->bOrientRotationToMovement = true;   // gira hacia su movimiento
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Z);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ATopDownHero::BeginPlay()
{
    Super::BeginPlay();
}

void ATopDownHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_MoveClick)
        {
            EIC->BindAction(IA_MoveClick, ETriggerEvent::Started, this, &ATopDownHero::OnMoveClick);
        }
        if (IA_Dash)
        {
            EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &ATopDownHero::OnDash);
        }
    }
}

void ATopDownHero::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bHasMoveTarget)
    {
        const FVector FlatDelta = FVector(MoveTarget.X, MoveTarget.Y, GetActorLocation().Z) - GetActorLocation();
        if (FlatDelta.Size2D() <= AcceptableRadius)
        {
            CancelClickMove();
        }
    }
}

bool ATopDownHero::GetMouseWorldPoint(FVector& OutPoint) const
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return false;

    // 1) Primer intento: trace bajo cursor
    FHitResult Hit;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit) && Hit.bBlockingHit)
    {
        OutPoint = Hit.ImpactPoint;
        return true;
    }

    // 2) Fallback: deproyectar y cortar con plano Z del personaje
    FVector Origin, Dir;
    if (PC->DeprojectMousePositionToWorld(Origin, Dir))
    {
        const float ZPlane = GetActorLocation().Z;
        if (FMath::Abs(Dir.Z) > KINDA_SMALL_NUMBER)
        {
            const float T = (ZPlane - Origin.Z) / Dir.Z;
            OutPoint = Origin + T * Dir;
            return true;
        }
    }
    return false;
}

bool ATopDownHero::ProjectToNavMesh(const FVector& In, FVector& OutProjected, const FVector Extent) const
{
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation Proj;
        if (NavSys->ProjectPointToNavigation(In, Proj, Extent))
        {
            OutProjected = Proj.Location;
            return true;
        }
    }
    OutProjected = In;
    return false;
}

void ATopDownHero::UpdateTargetMarker(const FVector& WorldPoint)
{
    if (!TargetMarkerClass) return;

    if (!TargetMarker)
    {
        FActorSpawnParameters SP;
        SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        TargetMarker = GetWorld()->SpawnActor<AActor>(TargetMarkerClass, WorldPoint, FRotator::ZeroRotator, SP);
    }
    else
    {
        TargetMarker->SetActorLocation(WorldPoint);
        TargetMarker->SetActorHiddenInGame(false);
        TargetMarker->SetActorEnableCollision(false);
    }
}

void ATopDownHero::IssueSimpleMoveTo(const FVector& Dest)
{
    if (AController* C = GetController())
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(C, Dest);
    }
}


void ATopDownHero::CancelClickMove()
{
    bHasMoveTarget = false;

    if (AController* C = GetController())
    {
        C->StopMovement();
        // “resetea” intención de movimiento
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(C, GetActorLocation());
    }

    if (TargetMarker)
    {
        TargetMarker->SetActorHiddenInGame(true);
    }
}

void ATopDownHero::OnMoveClick()
{
    FVector MousePoint;
    if (!GetMouseWorldPoint(MousePoint))
        return;

    FVector Dest;
    ProjectToNavMesh(MousePoint, Dest);

    MoveTarget = Dest;
    bHasMoveTarget = true;

    // Actualiza marcador verde
    UpdateTargetMarker(Dest);

    // Emite movimiento por NavMesh
    IssueSimpleMoveTo(Dest);

    // Opcional: mira hacia el destino
    FaceDirectionXY((Dest - GetActorLocation()).GetSafeNormal2D());
}

void ATopDownHero::OnDash()
{
    if (bIsDashing || bDashOnCooldown) return;

    FVector MousePoint;
    if (!GetMouseWorldPoint(MousePoint)) return;

    // Cancela click-to-move
    CancelClickMove();

    // Calcula dirección hacia el mouse
    FVector Dir = MousePoint - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.Normalize()) return;

    // Activa dash
    bIsDashing = true;
    bDashOnCooldown = true;

    // Aplica impulso rápido (solo XY)
    LaunchCharacter(Dir * DashSpeed, true, false);

    // Frenar al final del dash
    GetWorldTimerManager().SetTimer(
        TimerHandle_Dash,
        [this]()
        {
            GetCharacterMovement()->StopMovementImmediately();
            bIsDashing = false;
        },
        DashDuration,
        false
    );

    // Cooldown
    GetWorldTimerManager().SetTimer(
        TimerHandle_DashCD,
        [this]() { bDashOnCooldown = false; },
        DashCooldown,
        false
    );
}


void ATopDownHero::FaceDirectionXY(const FVector& Dir)
{
    if (!Dir.IsNearlyZero())
    {
        const FRotator NewYaw = UKismetMathLibrary::MakeRotFromX(Dir).Clamp();
        FRotator Final(0.f, NewYaw.Yaw, 0.f);
        SetActorRotation(Final);
    }
}
