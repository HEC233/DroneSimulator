// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaypointActor.generated.h"

struct FPoint;
struct FWaypoint;

UCLASS()
class DRONESIMULATOR_API AWaypointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaypointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category=Waypoint)
	const FWaypoint& GetWaypoint();

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	const TArray<FWaypoint>& GetWaypoints();

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	bool ChangeCurrentWaypoint(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	bool ChangeCurrentWaypointByName(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	const FVector& GetPoint(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void AddWaypoint(const FString& WaypointName);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void RemoveWaypoint(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void AddPoint(const FString& PointName, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void RemovePoint(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void ChangePointLocation(int32 Index, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	bool ChangePointIndex(int32 SrcIdx, int32 DestIdx);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void ChangePointName(int32 Idx, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void SetDroneOperation(bool bOperate);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void SetZoomRate(float ZoomRate);

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	float GetZoomRate();

	UFUNCTION(BlueprintCallable, Category = Waypoint)
	void SaveGame();

protected:
	void UpdateWaypoint();

private:
	UPROPERTY()
	TObjectPtr<class UWaypointSaveGame> WaypointData;
	UPROPERTY()
	TArray<TObjectPtr<class ADSText3DActor>> WaypointActors;
	FString LevelName;
};
