// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"
#include "Actors/DSTarget.h"
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

USTRUCT()
struct FArrayPacker
{
	GENERATED_BODY()

	FArrayPacker() {}
	FArrayPacker(class ADSTarget* InitialElement) { Targets.Add(InitialElement); }

	TArray<class ADSTarget*> Targets;
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

	UFUNCTION(BlueprintCallable, Category = "UI")
	TArray<UUserWidget*> CreateAndGetVisibiltyToggleWidget(ETargetUsageType TargetType);

	UFUNCTION(BlueprintCallable, Category="Logic")
	TArray<AActor*> GetSortedTargetActors(AActor* PresetActor);
	void SetPauseUI(bool bPause);

	TArray<const AActor*> GetTargetsInVolume(const FConvexVolume& ConvexVolume);

	UFUNCTION(BlueprintCallable, Category = "Logic")
	bool SetTargetVisibility(const TSubclassOf<AActor>& TargetClass, bool Visiblity);

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
	UPROPERTY()
	TArray<AActor*> AllTargets;

	UPROPERTY(EditAnywhere, Category = "Logic")
	TMap<TSubclassOf<AActor>, FArrayPacker> TargetsByClass;
	UPROPERTY(BlueprintReadOnly, Category = "Logic")
	TMap<TSubclassOf<AActor>, bool> TargetVisibility;

	UPROPERTY()
	TSubclassOf<class UDSWeaponToggleWidget> ToggleWidget;

	UPROPERTY(BlueprintReadOnly)
	uint8 bPaused = false;
private:
	TObjectPtr<UUserWidget> CurrentPauseWidget;

	FInputModeUIOnly UIOnly;
	FInputModeGameOnly GameOnly;
	FInputModeGameAndUI GameAndUI;
};
