// Fill out your copyright notice in the Description page of Project Settings.


#include "DSTarget.h"

// Sets default values
ADSTarget::ADSTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADSTarget::BeginPlay()
{
	Super::BeginPlay();
	TArray<UActorComponent*> Comps = GetComponentsByClass(USceneComponent::StaticClass());
	for (const auto& Comp : Comps)
	{
		if (Comp->ComponentHasTag(TEXT("Slider1")))
		{
			USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
			if (SceneComp)
			{
				Slider1Object = SceneComp;
			}
		}

		if (Comp->ComponentHasTag(TEXT("Slider2")))
		{
			USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
			if (SceneComp)
			{
				Slider2Object = SceneComp;
			}
		}
	}
}

// Called every frame
void ADSTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADSTarget::UpdateState_Implementation()
{
	if (Slider1Object.Get())
	{
		FRotator Rot = Slider1Object->GetRelativeRotation();
		switch (Slider1Axis)
		{
		case ESliderAxis::Yaw:
			Slider1Object->SetRelativeRotation(FRotator(Rot.Pitch, Slider1Value, Rot.Roll));
			break;
		case ESliderAxis::Pitch:
			Slider1Object->SetRelativeRotation(FRotator(Slider1Value, Rot.Yaw, Rot.Roll));
			break;
		case ESliderAxis::Roll:
			Slider1Object->SetRelativeRotation(FRotator(Rot.Pitch, Rot.Yaw, Slider1Value));
			break;
		}
	}

	if (Slider2Object.Get())
	{
		FRotator Rot = Slider2Object->GetRelativeRotation();
		switch (Slider2Axis)
		{
		case ESliderAxis::Yaw:
			Slider2Object->SetRelativeRotation(FRotator(Rot.Pitch, Slider2Value, Rot.Roll));
			break;
		case ESliderAxis::Pitch:
			Slider2Object->SetRelativeRotation(FRotator(Slider2Value, Rot.Yaw, Rot.Roll));
			break;
		case ESliderAxis::Roll:
			Slider2Object->SetRelativeRotation(FRotator(Rot.Pitch, Rot.Yaw, Slider2Value));
			break;
		}
	}
}

