// TutorialWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialWidget.generated.h"

class UImage;
class UButton;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTutorialFinished);

UCLASS(BlueprintType, Blueprintable)
class CATSHIFT_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Puedes asignarlas en el editor (Soft) o por código */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Tutorial")
	TArray<TSoftObjectPtr<UTexture2D>> Images;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bAutoAdvance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float AutoAdvanceTime = 3.0f;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FTutorialFinished OnTutorialFinished;

	/** Si prefieres pasar imágenes en runtime */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SetImages(const TArray<UTexture2D*>& InImages);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ImgTutorial = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BtnNext = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BtnSkip = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	int32 CurrentIndex = 0;
	TArray<UTexture2D*> LoadedImages;
	FTimerHandle AutoTimer;

	void ShowIndex(int32 Index);

	UFUNCTION() void ShowNext();
	UFUNCTION() void CloseTutorial();

	/** Restaura el input para click-to-move */
	void ReturnToGameInput();

	/** Toma foco UI al abrirse (lo llamamos en Construct) */
	void TakeUIFocus();
};
