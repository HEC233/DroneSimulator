// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DSWeaponToggleWidget.generated.h"

DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnVisibilityToggleChanged, const TSubclassOf<AActor>& /*TargetClass*/, bool /*Visiblity*/);

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API UDSWeaponToggleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetClass(TSubclassOf<AActor> InClass, const FString& TargetName);
	void SetToggleState(bool Value);

public:
	FOnVisibilityToggleChanged OnToggleChanged;
	
	UFUNCTION(BlueprintCallable)
	void ToggleChanged(bool Value) { check(OnToggleChanged.Execute(TargetClass, Value)); }

protected:
	UPROPERTY()
	TSubclassOf<AActor> TargetClass;
};
