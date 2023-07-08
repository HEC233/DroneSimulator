// Fill out your copyright notice in the Description page of Project Settings.


#include "WaypointActor.h"
#include "Kismet/GameplayStatics.h"
#include "DroneSimulator/Save/WaypointSaveGame.h"

// Sets default values
AWaypointActor::AWaypointActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWaypointActor::BeginPlay()
{
	Super::BeginPlay();

	LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	USaveGame* SaveData = UGameplayStatics::LoadGameFromSlot(*LevelName, 0);
	if (SaveData)
	{
		WaypointData = Cast<UWaypointSaveGame>(SaveData);
	}
	else
	{
		WaypointData = NewObject<UWaypointSaveGame>();
	}

	UpdateWaypoint();
}

// Called every frame
void AWaypointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

const FWaypoint& AWaypointActor::GetWaypoint()
{
	return WaypointData->Waypoints[WaypointData->WaypointIndex];
}

const TArray<FWaypoint>& AWaypointActor::GetWaypoints()
{
	return WaypointData->Waypoints;
}

bool AWaypointActor::ChangeCurrentWaypoint(int32 Index)
{
	if (WaypointData->Waypoints.IsValidIndex(Index))
	{
		WaypointData->WaypointIndex = Index;
		UpdateWaypoint();
		return true;
	}
	return false;
}

bool AWaypointActor::ChangeCurrentWaypointByName(const FString& Name)
{
	int32 Index = WaypointData->Waypoints.IndexOfByPredicate([&](const FWaypoint& Var)
		{
			return Var.Name == Name;
		});
	if (INDEX_NONE == Index)
	{
		return false;
	}
	return ChangeCurrentWaypoint(Index);
}

const FVector& AWaypointActor::GetPoint(int32 Index)
{
	check(WaypointData->Waypoints[WaypointData->WaypointIndex].Points.IsValidIndex(Index));
	return WaypointData->Waypoints[WaypointData->WaypointIndex].Points[Index].Location;
}

void AWaypointActor::AddWaypoint(const FString& WaypointName)
{
	WaypointData->Waypoints.Add(FWaypoint(TEXT("New Waypoint")));
}

void AWaypointActor::RemoveWaypoint(int32 Index)
{
	if (WaypointData->Waypoints.IsValidIndex(Index))
	{
		WaypointData->Waypoints.RemoveAt(Index);
	}
}

void AWaypointActor::AddPoint(const FString& PointName, const FVector& Location)
{
	WaypointData->Waypoints[WaypointData->WaypointIndex].Points.Add(FPoint(Location, PointName));
	UpdateWaypoint();
}

void AWaypointActor::RemovePoint(int32 Index)
{
	auto& Points = WaypointData->Waypoints[WaypointData->WaypointIndex].Points;
	if (Points.IsValidIndex(Index))
	{
		Points.RemoveAt(Index);
	}
	UpdateWaypoint();
}

void AWaypointActor::ChangePointLocation(int32 Index, const FVector& Location)
{
	auto& Points = WaypointData->Waypoints[WaypointData->WaypointIndex].Points;
	if (Points.IsValidIndex(Index))
	{
		Points[Index].Location = Location;
	}
	UpdateWaypoint();
}

void AWaypointActor::ChangePointIndex(int32 SrcIdx, int32 DescIdx)
{

	UpdateWaypoint();
}

void AWaypointActor::ChangePointName(int32 Idx, const FString& Name)
{
	auto& Points = WaypointData->Waypoints[WaypointData->WaypointIndex].Points;
	if (Points.IsValidIndex(Idx))
	{
		Points[Idx].Name = Name;
	}
}

void AWaypointActor::SetDroneOperation(bool bOperate)
{

	UpdateWaypoint();
}

void AWaypointActor::SetZoomRate(float ZoomRate)
{
	WaypointData->Waypoints[WaypointData->WaypointIndex].ZoomRate = ZoomRate;
}

float AWaypointActor::GetZoomRate()
{
	return WaypointData->Waypoints[WaypointData->WaypointIndex].ZoomRate;
}

void AWaypointActor::SaveGame()
{
	UGameplayStatics::SaveGameToSlot(WaypointData, LevelName, 0);
}

void AWaypointActor::UpdateWaypoint()
{
}

