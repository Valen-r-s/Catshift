#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TopDownHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;

/**
 * Character top‑down con WASD, click‑to‑move y dash
 */
UCLASS()
class CATSHIFT_API ATopDownHero : public ACharacter
{
    GENERATED_BODY()

public:
    ATopDownHero();

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ---------- Cámara ----------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* TopDownCamera;

public:
    // ---------- Input Actions (asignar en BP_TopDownHero) ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_Move = nullptr;       // Axis2D

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_ClickMove = nullptr;  // Digital

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_Dash = nullptr;       // Digital

protected:
    // ---------- Callbacks de input ----------
    UFUNCTION() void Input_Move(const FInputActionValue& Value);
    UFUNCTION() void Input_ClickMove();
    UFUNCTION() void Input_Dash();

    // ---------- Click-to-move helper ----------
    void MoveToWorldPoint(const FVector& WorldPoint);

    // ---------- Dash ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashStrength = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashDuration = 0.10f;

    bool bIsDashing = false;
    FTimerHandle Timer_StopDash;

    void StopDash();
};
