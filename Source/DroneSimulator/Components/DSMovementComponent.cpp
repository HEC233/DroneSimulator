// Fill out your copyright notice in the Description page of Project Settings.


#include "DSMovementComponent.h"
#include "DroneSimulator/Pawns/DSDronePawn.h"

// Sets default values for this component's properties
UDSMovementComponent::UDSMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MeshRotateSpeed = 10;
	CurrentAxis = FQuat(0, 0, 1, 0);
	DesiredAxis = CurrentAxis;
	CurrentYaw = 0;
	DesiredYaw = CurrentYaw;
}


// Called when the game starts
void UDSMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Actor = GetOwner();
}


// Called every frame
void UDSMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ADSDronePawn* Drone = Cast<ADSDronePawn>(GetOwner());
	if (Drone)
	{
		DeltaTime = FMath::Max(0.001f, DeltaTime - Drone->GetCaptureSpan());
	}
	UpdateRotator(DeltaTime);
}

void UDSMovementComponent::MoveDirection(const FVector& Direction)
{
	Actor->SetActorLocation(Actor->GetActorLocation() + Direction * GetWorld()->GetDeltaSeconds());
}

void UDSMovementComponent::MoveTo(const FVector& Location)
{
	Actor->SetActorLocation(Location);
}

void UDSMovementComponent::UpdateRotator(float DeltaTime)
{
	FVector Direction = Actor->GetActorLocation() - PrevPos;

	Direction.Z = 0;
	float Size = Direction.Size();

	if (!FMath::IsNearlyZero(Size))
	{
		DesiredYaw = -FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
		float Angle = DesiredYaw - CurrentYaw;
		while (Angle < -180)
		{
			Angle += 360;
		}
		while (Angle > 180)
		{
			Angle -= 360;
		}

		float DeltaAngle = MeshRotateSpeed * DeltaTime;
		if (FMath::Abs(Angle) < DeltaAngle)
		{
			CurrentYaw = DesiredYaw;
		}
		else if (Angle < 0)
		{
			CurrentYaw -= DeltaAngle;
		}
		else
		{
			CurrentYaw += DeltaAngle;
		}

		Direction.Z = Size / FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(FMath::Log2(Size / DeltaTime), 2, 14)));
		Direction.Normalize();
		DesiredAxis = FQuat(Direction.X, Direction.Y, Direction.Z, 0);
	}
	else
	{
		DesiredAxis = FQuat(0, 0, 1, 0);
	}

	PrevPos = Actor->GetActorLocation();

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(CurrentYaw / 2));

	CurrentAxis = FQuat::Slerp(CurrentAxis, DesiredAxis, DeltaTime * 3);
	Actor->SetActorRotation(CurrentAxis * FQuat(0, 0, Cos, Sin));
}

