// Fill out your copyright notice in the Description page of Project Settings.


#include "DSGameMode.h"

ADSGameMode::ADSGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/DroneSimulator.DSDronePawn"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}
}
