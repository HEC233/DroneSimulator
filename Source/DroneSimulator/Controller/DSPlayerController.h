// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"
#include "DSPlayerController.generated.h"

UENUM(BlueprintType)
enum class UIStatus : uint8
{
	Title = 0,
	MapSelect,
	Setting,
	CaptureMode,
	Pause,
};

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API ADSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADSPlayerController();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="UI")
	void ChangeUI(UIStatus UI);

	UFUNCTION(BlueprintCallable, Category="Logic")
	TArray<AActor*> GetSortedTargetActors(AActor* PresetActor);

	void SetPauseUI(bool bPause);

protected:
	UPROPERTY(BlueprintReadOnly)
	UIStatus CurrentUI;
	TArray<TObjectPtr<UUserWidget>> CurrentWidgets;

	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> TitleWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> MapSelectWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> SettingWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> PauseWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> CaptureModeWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> RenderTargetWidget;
	UPROPERTY(EditAnywhere, Category = "UI Widget")
	TSubclassOf<UUserWidget> MinimapWidget;

	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UTexture2D> MinimapImage;
	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	float MinimapCaptureXCoord;
	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	float MinimapCaptureYCoord;
	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	float MinimapCaptureScale;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> TargetArray;
	UPROPERTY(EditAnywhere, Category = "Logic")
	TSubclassOf<AActor> TargetParent;

	UPROPERTY(BlueprintReadOnly)
	uint8 bPaused = false;
private:
	TObjectPtr<UUserWidget> CurrentPauseWidget;

	FInputModeUIOnly UIOnly;
	FInputModeGameOnly GameOnly;
	FInputModeGameAndUI GameAndUI;
};
