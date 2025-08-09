#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h" // <- necesario por FTimerHandle
#include "TopDownHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;

UCLASS()
class ATopDownHero : public ACharacter
{
    GENERATED_BODY()

public:
    ATopDownHero();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ===================== COMPONENTES =====================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* TopDownCamera;

    // ===================== INPUTS (Enhanced Input) =====================
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_MoveClick;        // Click derecho

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_Dash;             // Space

    // --- Movimiento continuo con RMB ---
    UPROPERTY(EditAnywhere, Category = "Movement|ClickToMove")
    bool bStopOnRMBRelease = false; // Si true: se detiene al soltar. Si false: conserva último destino (LoL).

    bool bRMBHeld = false;

    // ===================== MOVIMIENTO POR CLICK =====================
    UPROPERTY(VisibleInstanceOnly, Category = "Movement|ClickToMove")
    FVector MoveTarget;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|ClickToMove")
    bool bHasMoveTarget = false;

    UPROPERTY(EditAnywhere, Category = "Movement|ClickToMove")
    float AcceptableRadius = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement|ClickToMove")
    TSubclassOf<AActor> TargetMarkerClass;

    UPROPERTY()
    AActor* TargetMarker = nullptr;

    // RMB hold handlers
    UFUNCTION()
    void OnMoveClickStarted();

    UFUNCTION()
    void OnMoveClickTriggered();

    UFUNCTION()
    void OnMoveClickCompleted();

    void IssueSimpleMoveTo(const FVector& Dest);
    void CancelClickMove();

    // ===================== DASH (desplazamiento rápido) =====================
    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashSpeed = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashDuration = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashCooldown = 0.5f;

    bool bIsDashing = false;
    bool bDashOnCooldown = false;

    FTimerHandle TimerHandle_Dash;
    FTimerHandle TimerHandle_DashCD;

    UFUNCTION()
    void OnDash();

    // ===================== UTILIDADES =====================
    bool GetMouseWorldPoint(FVector& OutPoint) const;
    void UpdateTargetMarker(const FVector& WorldPoint);
    bool ProjectToNavMesh(const FVector& In, FVector& OutProjected, const FVector Extent = FVector(60, 60, 200)) const;
    void FaceDirectionXY(const FVector& Dir);
};
