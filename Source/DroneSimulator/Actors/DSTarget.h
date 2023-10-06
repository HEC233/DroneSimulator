// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DSTarget.generated.h"

UCLASS()
class DRONESIMULATOR_API ADSTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADSTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE FString GetTargetName() { return TargetName; }
	FORCEINLINE FString GetTargetAbsoluteName() { return TargetAbsoluteName; }

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString TargetName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString TargetAbsoluteName;
};
