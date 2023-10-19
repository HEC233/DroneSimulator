// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RenderTargetViewWidget.generated.h"

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API URenderTargetViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	URenderTargetViewWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void Update();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditAnywhere, Category = Widget, meta=(AllowPrivateAccess="true"))
	TSubclassOf<UUserWidget> BboxWidgetType;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> BboxWidgets;

	UPROPERTY()
	TObjectPtr<class UPanelWidget> PanelWidget;
};
