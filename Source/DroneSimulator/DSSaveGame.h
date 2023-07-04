// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DSSaveGame.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EPilotMode : uint8
{
	E_AutoMode,
	E_ManualMode,
	E_WayPointMode,
};

USTRUCT(BlueprintType)
struct FWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentZoomRate = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Points;
};

UCLASS()
class DRONESIMULATOR_API UDSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UDSSaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int CurrentTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int CurrentWeather = 0;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
    bool FogMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	EPilotMode PilotMode = EPilotMode::E_AutoMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentHeight = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentMoveSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FString CurrentTargetPreset = TEXT("프리셋1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentFOV = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float CurrentCaptureSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FWaypoint> WayPointArr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FWaypoint CurrentWayPoint;
};
