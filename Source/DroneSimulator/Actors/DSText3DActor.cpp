// Fill out your copyright notice in the Description page of Project Settings.


#include "DSText3DActor.h"
#include "Text3DComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADSText3DActor::ADSText3DActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Text3DComponent = CreateDefaultSubobject<UText3DComponent>(TEXT("3DText"));
	Text3DComponent->SetHorizontalAlignment(EText3DHorizontalTextAlignment::Center);
	Text3DComponent->SetVerticalAlignment(EText3DVerticalTextAlignment::Center);
	Text3DComponent->SetWorldScale3D(FVector::OneVector * 5.0f);

	SetRootComponent(Text3DComponent);
}

// Called when the game starts or when spawned
void ADSText3DActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADSText3DActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Direction = GetActorLocation() - UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	SetActorRotation(Direction.ToOrientationRotator());
}

void ADSText3DActor::SetText(const FString& Text)
{
	Text3DComponent->SetText(FText::FromString(Text));
}

