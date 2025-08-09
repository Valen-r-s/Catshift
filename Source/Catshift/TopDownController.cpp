#include "TopDownController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ATopDownController::BeginPlay()
{
    Super::BeginPlay();

    // Cursor visible para top‑down con click
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;

    // Aplicar el Mapping Context al jugador local
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsys =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            if (IMC_Default)
            {
                Subsys->AddMappingContext(IMC_Default, /*Priority*/0);
            }
        }
    }
}
