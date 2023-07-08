// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DSText3DActor.generated.h"

UCLASS()
class DRONESIMULATOR_API ADSText3DActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADSText3DActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UText3DComponent> Text3DComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void SetText(const FString& Text);
};
