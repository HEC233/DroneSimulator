// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API UMinimapWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category=Minimap)
	void SetMinimapInfo(class UTexture2D* Image, float ImageXCoord, float ImageYCoord, float ImageScale);

	FORCEINLINE FVector2D WorldPos2MinimapCoord(const FVector& WorldPos);

	UFUNCTION(BlueprintCallable, Category = Minimap)
	void SetPlayerActor(AActor* InPlayerActor);

	UFUNCTION(BlueprintCallable, Category = Minimap)
	void UpdateMinimap();

	UFUNCTION(BlueprintNativeEvent)
	float GetZoomRate();
	virtual float GetZoomRate_Implementation();

protected:
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UUserWidget> WaypointWidget;
	UPROPERTY(EditAnywhere, Category = "Widget")
	TObjectPtr<UPanelWidget> MinimapPanel;

private:
	bool SetWaypointActor();

private:
	FVector2d MinimapCaptureLocation;
	float MinimapCaptureScale;

	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;
	UPROPERTY()
	TObjectPtr<class USpringArmComponent> PlayerSpringArm;
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> WaypointWidgets;

	FVector2d MinimapCoord;
	float MinimapRotate;
	FName MinimapName;
	FName WaypointIndexName;

	UPROPERTY()
	TObjectPtr<class AWaypointActor> WpActor;
};
