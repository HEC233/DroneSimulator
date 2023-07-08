// Fill out your copyright notice in the Description page of Project Settings.


#include "WaypointSaveGame.h"

UWaypointSaveGame::UWaypointSaveGame()
{
	Waypoints.Add(FWaypoint(TEXT("웨이포인트#1")));
	Waypoints.Add(FWaypoint(TEXT("웨이포인트#2")));
	Waypoints.Add(FWaypoint(TEXT("웨이포인트#3")));
}
