// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DSTargetContainingSingleton.generated.h"

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API UDSTargetContainingSingleton : public UObject
{
	GENERATED_BODY()
	
public:
	UDSTargetContainingSingleton();

private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> TargetActors;

};
