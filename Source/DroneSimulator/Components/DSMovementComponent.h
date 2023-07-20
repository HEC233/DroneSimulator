// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DSMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONESIMULATOR_API UDSMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDSMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void MoveDirection(const FVector& Direction);
	void MoveTo(const FVector& Location);

protected:
	void UpdateRotator(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere)
	float MeshRotateSpeed;

	float CurrentYaw;
	FQuat CurrentAxis;

	float DesiredYaw;
	FQuat DesiredAxis;

	FVector PrevDirection;
	FVector PrevPos;
};
