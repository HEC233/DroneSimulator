// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WaypointSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FPoint
{
	GENERATED_BODY()
	FPoint() { Location = FVector::Zero(); Name = TEXT("New Point"); }
	FPoint(const FVector& InLocation, const FString& InName) : Location(InLocation), Name(InName) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
};

USTRUCT(BlueprintType)
struct FWaypoint
{
	GENERATED_BODY()
	FWaypoint() : Name(TEXT("Waypoint")) {}
	FWaypoint(FString InName) : Name(InName) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPoint> Points;
};

UCLASS()
class DRONESIMULATOR_API UWaypointSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UWaypointSaveGame();

private:
	
	friend class AWaypointActor;

	UPROPERTY()
	TArray<FWaypoint> Waypoints;

	UPROPERTY()
	int32 WaypointIndex;
};
