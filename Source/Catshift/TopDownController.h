#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TopDownController.generated.h"

class UInputMappingContext;

/**
 * PlayerController: aplica el Input Mapping Context (IMC)
 */
UCLASS()
class CATSHIFT_API ATopDownController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    /** Asigna este asset en BP_TopDownController (Class Defaults) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* IMC_Default = nullptr;
};
