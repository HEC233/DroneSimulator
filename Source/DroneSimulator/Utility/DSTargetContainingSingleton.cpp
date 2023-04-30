// Fill out your copyright notice in the Description page of Project Settings.


#include "DSTargetContainingSingleton.h"
#include "Engine/World.h"

UDSTargetContainingSingleton::UDSTargetContainingSingleton()
{
	// TODO : regist delegate where map changing event occur
	UWorld* World = GetWorld();
	
	if (World == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("World 객체가 없음!!"))
		return;
	}

	for (AActor* Actor : World->ActiveGroupActors)
	{
		
	}
}
