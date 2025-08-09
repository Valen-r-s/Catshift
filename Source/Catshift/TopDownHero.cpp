#include "TopDownHero.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"

ATopDownHero::ATopDownHero()
{
    PrimaryActorTick.bCanEverTick = true;

    // ------- Cámara top‑down -------
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 800.f;
    SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bDoCollisionTest = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(SpringArm);
    TopDownCamera->FieldOfView = 90.f;

    // ------- Movimiento -------
    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = true;    // mirar hacia donde se mueve
    Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    Move->MaxWalkSpeed = 500.f;

    bUseControllerRotationYaw = false;
}

void ATopDownHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)      EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ATopDownHero::Input_Move);
        if (IA_ClickMove) EIC->BindAction(IA_ClickMove, ETriggerEvent::Started, this, &ATopDownHero::Input_ClickMove);
        if (IA_Dash)      EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &ATopDownHero::Input_Dash);
    }
}

// ------- WASD (Axis2D) -------
void ATopDownHero::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddMovementInput(FVector::ForwardVector, Axis.Y); // adelante/atrás
    AddMovementInput(FVector::RightVector, Axis.X); // derecha/izquierda
}

// ------- Click‑to‑move -------
void ATopDownHero::Input_ClickMove()
{
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit))
        {
            MoveToWorldPoint(Hit.ImpactPoint);
        }
    }
}

void ATopDownHero::MoveToWorldPoint(const FVector& WorldPoint)
{
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation Projected;
        if (NavSys->ProjectPointToNavigation(WorldPoint, Projected))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Projected.Location);
        }
    }
}

// ------- Dash (Space) -------
void ATopDownHero::Input_Dash()
{
    if (bIsDashing) return;
    bIsDashing = true;

    FVector Dir = GetVelocity().GetSafeNormal();
    if (Dir.IsNearlyZero())
        Dir = GetActorForwardVector();

    LaunchCharacter(Dir * DashStrength, /*XYOverride*/true, /*ZOverride*/true);
    GetWorldTimerManager().SetTimer(Timer_StopDash, this, &ATopDownHero::StopDash, DashDuration, false);
}

void ATopDownHero::StopDash()
{
    GetCharacterMovement()->StopMovementImmediately();
    bIsDashing = false;
}
