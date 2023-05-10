// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DSSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class DRONESIMULATOR_API UDSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int CurrentTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int CurrentWeather = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	bool AutoPilot = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentMoveSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentFOV = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentCaptureSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TObjectPtr<AActor> CurrentTarget = nullptr;
};
