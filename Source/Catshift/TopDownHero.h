#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

    // ===================== MOVIMIENTO POR CLICK =====================
    // Punto de destino en el mundo (click derecho)
    UPROPERTY(VisibleInstanceOnly, Category = "Movement|ClickToMove")
    FVector MoveTarget;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|ClickToMove")
    bool bHasMoveTarget = false;

    // Radio para considerar “llegó”
    UPROPERTY(EditAnywhere, Category = "Movement|ClickToMove")
    float AcceptableRadius = 60.f;

    // BP del marcador de destino (esfera verde)
    UPROPERTY(EditDefaultsOnly, Category = "Movement|ClickToMove")
    TSubclassOf<AActor> TargetMarkerClass;

    // Instancia del marcador
    UPROPERTY()
    AActor* TargetMarker = nullptr;

    UFUNCTION()
    void OnMoveClick();                 // handler de click derecho

    void IssueSimpleMoveTo(const FVector& Dest); // emite el movimiento por nav

    void CancelClickMove();             // cancela path following + oculta marcador

    // ===================== DASH (tipo "Flash") =====================
    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashSpeed = 2000.f;         // distancia fija del flash

    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashCooldown = 0.5f;          // CD


    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashDuration = 0.15f;   // cuánto dura el dash

    bool bIsDashing = false;
    FTimerHandle TimerHandle_Dash;

    bool bDashOnCooldown = false;
    FTimerHandle TimerHandle_DashCD;

    UFUNCTION()
    void OnDash();                      // handler de Space

    // ===================== UTILIDADES =====================
    // Obtiene el punto del mouse sobre el mundo (hit en geometría o intersección con plano Z del personaje)
    bool GetMouseWorldPoint(FVector& OutPoint) const;

    // Mueve/crea el marcador verde
    void UpdateTargetMarker(const FVector& WorldPoint);

    // Proyecta una ubicación a NavMesh si es posible
    bool ProjectToNavMesh(const FVector& In, FVector& OutProjected, const FVector Extent = FVector(60, 60, 200)) const;

    // Reorientar el actor hacia una dirección X-Y
    void FaceDirectionXY(const FVector& Dir);
};
