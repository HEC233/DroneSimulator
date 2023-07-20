// Fill out your copyright notice in the Description page of Project Settings.


#include "DSDronePawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "DroneSimulator/DSSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "DroneSimulator/Actors/WaypointActor.h"
#include "DroneSimulator/Save/WaypointSaveGame.h"

#include "DroneSimulator/Components/DSCaptureComponent.h"

// Sets default values
ADSDronePawn::ADSDronePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube'"));
	if (StaticMeshRef.Object)
	{
		MeshComponent->SetStaticMesh(StaticMeshRef.Object);
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputLookAroundAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_LookAround.IA_LookAround'"));
	if (InputLookAroundAction.Object)
	{
		LookAroundAction = InputLookAroundAction.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputStartCaptureAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_StartCapture.IA_StartCapture'"));
	if (InputStartCaptureAction.Object)
	{
		StartCaptureAction = InputStartCaptureAction.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputDroneMoveAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_DroneMove.IA_DroneMove'"));
	if (InputDroneMoveAction.Object)
	{
		DroneMoveAction = InputDroneMoveAction.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputDroneAltitudeAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_DroneAltitude.IA_DroneAltitude'"));
	if (InputDroneAltitudeAction.Object)
	{
		DroneAltitudeAction = InputDroneAltitudeAction.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputDroneCameraZoomAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_DroneZoom.IA_DroneZoom'"));
	if (InputDroneCameraZoomAction.Object)
	{
		DroneCameraZoomAction = InputDroneCameraZoomAction.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> InputDroneSpeedChangeAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/Input/IA_DroneSpeedChange.IA_DroneSpeedChange'"));
	if (InputDroneSpeedChangeAction.Object)
	{
		DroneSpeedChangeAction = InputDroneSpeedChangeAction.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/DroneSimulator/Input/IMC_Default.IMC_Default'"));
	if (InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	CameraBoom->bUsePawnControlRotation = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CaptureComponent = CreateDefaultSubobject<UDSCaptureComponent>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(RootComponent);
	CaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	DroneSpeedArray.Add(300);
	DroneSpeedArray.Add(600);
	DroneSpeedArray.Add(1200);
	DroneSpeedArray.Add(2400);
	DroneSpeedArray.Add(6000);
	DroneSpeedIndex = 2;

	RotationRadius = 10.f;
	DroneSpeed = DroneSpeedArray[DroneSpeedIndex];
}

// Called when the game starts or when spawned
void ADSDronePawn::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	MakeWaypointActorValid();

	CurrentCaptureCount = 0;
	CurrentRotationRate = 0.0f;
}

// Called every frame
void ADSDronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyLoadData();

	DeltaTime = FMath::Max(0.0f, DeltaTime - CaptureSpan);
	CaptureSpan = 0.0f;
	if (DeltaTime == 0.0f)
	{
		return;
	}

	if (!bDroneOperation)
	{
		return;
	}
	else if (DroneMode == EDroneMode::AutoPilot)
	{
		MoveDrone(DeltaTime);
	}
	else if (DroneMode == EDroneMode::Waypoint)
	{
		MoveDroneWithWaypoint(DeltaTime);
	}

	if (bIsCapture)
	{
		if (CurrentCaptureCount < MaxCaptureCount)
		{
			TimeRecord += DeltaTime;
			CaptureTimeDuration += DeltaTime;

			if (TimeRecord >= 1.0f / CaptureSpeedPerSecond)
			{
				double StartTime = FPlatformTime::Seconds();
				CaptureComponent->TakeScreenShot(CurrentCaptureCount);
				double EndTime = FPlatformTime::Seconds();

				CaptureSpan = EndTime - StartTime;
				UE_LOG(LogTemp, Log, TEXT("Capturing Time : %f"), CaptureSpan);

				TimeRecord -= 1.0f / CaptureSpeedPerSecond;
				CurrentCaptureCount++;
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT("InGame Capture Time : %f, Current Capture %d / %d")
					, CaptureTimeDuration, CurrentCaptureCount, MaxCaptureCount));
			}
		}
	}
}

// Called to bind functionality to input
void ADSDronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ADSDronePawn::ProcessMouseInput);
	EnhancedInputComponent->BindAction(StartCaptureAction, ETriggerEvent::Triggered, this, &ADSDronePawn::StartCapture);
	EnhancedInputComponent->BindAction(DroneMoveAction, ETriggerEvent::Triggered, this, &ADSDronePawn::MoveDroneWithInput);
	EnhancedInputComponent->BindAction(DroneAltitudeAction, ETriggerEvent::Triggered, this, &ADSDronePawn::DroneAltitudeInput);
	EnhancedInputComponent->BindAction(DroneCameraZoomAction, ETriggerEvent::Triggered, this, &ADSDronePawn::CameraZoomInput);
	EnhancedInputComponent->BindAction(DroneSpeedChangeAction, ETriggerEvent::Started, this, &ADSDronePawn::DroneSpeedChange);
}

void ADSDronePawn::ProcessMouseInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	FTransform BoomTransform = CameraBoom->GetRelativeTransform();
	FRotator BoomRotator = BoomTransform.Rotator();

	BoomRotator.Yaw = BoomRotator.Yaw + LookAxisVector.X;
	BoomRotator.Pitch = FMath::Clamp(BoomRotator.Pitch + LookAxisVector.Y, -80.0f, bDroneOperation ? 80.0f : -10.0f);
	BoomTransform.SetRotation(BoomRotator.Quaternion());

	CameraBoom->SetRelativeTransform(BoomTransform);
}

void ADSDronePawn::StartCapture()
{
	if (!bDroneOperation)
	{
		return;
	}

	if (!bIsCapture)
	{
		ChangeCaptureState(true);
		CurrentCaptureCount = 0;
		CaptureTimeDuration = 0.0f;
		return;
	}
	else
	{
		ChangeCaptureState(false);
		return;
	}
}

void ADSDronePawn::MakeWaypointActorValid()
{
	if (WpActor != nullptr && WpActor->IsValidLowLevel())
	{
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypointActor::StaticClass(), OutActors);
	if (OutActors.Num() > 0)
	{
		WpActor = Cast<AWaypointActor>(OutActors[0]);
	}
	else
	{
		WpActor = GetWorld()->SpawnActor<AWaypointActor>();
	}
}

UDSSaveGame* ADSDronePawn::LoadGame()
{
	const auto SaveSlot = Cast<UDSSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSetting", 0));
	check(SaveSlot != nullptr);
	return SaveSlot;
}

void ADSDronePawn::ApplyLoadData()
{
	UDSSaveGame* DroneData = LoadGame();

	if (DroneData != nullptr)
	{
		if (DroneData->CurrentTarget != nullptr)
		{
			CenterPosition = DroneData->CurrentTarget->GetActorLocation() + FVector(0, 0, DroneData->CurrentHeight * 100.0f);

			CaptureComponent->SetTarget(DroneData->CurrentTarget);
		}

		RotationRadius = DroneData->CurrentRadius * 100.0f;
		AngleSpeed = DroneData->CurrentMoveSpeed / 60.0f;

		CaptureSpeedPerSecond = DroneData->CurrentCaptureSpeed;
		if (DroneMode == EDroneMode::Waypoint)
		{
			CaptureComponent->SetZoomRate(WpActor->GetWaypoint().ZoomRate);
		}
		else
		{
			CaptureComponent->SetZoomRate(DroneData->CurrentZoom);
		}
	}
}

void ADSDronePawn::MoveDrone(float DeltaTime)
{
	FVector NewPosition = CenterPosition;
	CurrentRotationRate = FMath::Fmod((CurrentRotationRate + AngleSpeed * DeltaTime), 2.0f * PI);
	float SinValue, CosValue;
	FMath::SinCos(&SinValue, &CosValue, CurrentRotationRate);

	NewPosition += FVector(SinValue, CosValue, 0) * RotationRadius;

	SetActorLocation(NewPosition);
}

void ADSDronePawn::MoveDroneWithWaypoint(float DeltaTime)
{
	const FWaypoint& Wp = WpActor->GetWaypoint();
	if (Wp.Points.IsEmpty()) return;

	CaptureComponent->SetLookAtPos(Wp.Points[CurrentPointIndex].Location);
	
	const FVector Direction = Wp.Points[CurrentPointIndex].Location - GetActorLocation();
	const float TargetLenght = Direction.Size();
	
	FVector NewPosition = GetActorLocation() + Direction.GetSafeNormal() * DroneSpeed * DeltaTime;
	if (TargetLenght < DroneSpeed * DeltaTime)
	{
		NewPosition = Wp.Points[CurrentPointIndex].Location;

		++CurrentPointIndex;
		if (CurrentPointIndex >= Wp.Points.Num())
		{
			CurrentPointIndex = 0;
		}
	}
	
	SetActorLocation(NewPosition);
}

void ADSDronePawn::MoveDroneWithInput(const FInputActionValue& Value)
{
	if (DroneMode == EDroneMode::TargetView || (bDroneOperation && DroneMode != EDroneMode::Manual))
	{
		return;
	}

	FVector2D Input = Value.Get<FVector2D>();

	const FRotator Rotation = CameraBoom->GetComponentRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector Movement = ForwardDirection * Input.Y + RightDirection * Input.X;

	SetActorLocation(GetActorLocation() + Movement * DroneSpeed * GetWorld()->DeltaTimeSeconds);
}

void ADSDronePawn::DroneAltitudeInput(const FInputActionValue& Value)
{
	float Input = Value.Get<float>();

	SetActorLocation(GetActorLocation() + FVector::UpVector * Input * DroneSpeed * GetWorld()->DeltaTimeSeconds);
}

void ADSDronePawn::CameraZoomInput(const FInputActionValue& Value)
{
	float Input = Value.Get<float>();

	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + Input * 10, 200, 1000);
}

void ADSDronePawn::DroneSpeedChange(const FInputActionValue& Value)
{
	float Input = Value.Get<float>();

	if (Input > 0 && DroneSpeedArray.IsValidIndex(DroneSpeedIndex + 1))
	{
		DroneSpeedIndex++;
	}
	else if (Input < 0 && DroneSpeedArray.IsValidIndex(DroneSpeedIndex - 1))
	{
		DroneSpeedIndex--;
	}
	DroneSpeed = DroneSpeedArray[DroneSpeedIndex];
}

void ADSDronePawn::UpdateDroneSpeed()
{
	float Diameter = RotationRadius * 2.0f * PI;
	float RotatingTime = Diameter / DroneSpeed;
	AngleSpeed = (2.0f * PI) / RotatingTime;
}

void ADSDronePawn::ChangeCaptureState(bool bBoolean)
{
	bIsCapture = bBoolean;
	//CaptureComponent->SetCaptureTick(!bBoolean);
}

FString ADSDronePawn::GetCaptureInfo()
{
	return FString::Printf(TEXT("캡쳐 진행시간 : %.2f\n현재 캡쳐 수 %d / %d"), CaptureTimeDuration, CurrentCaptureCount, MaxCaptureCount);
}

void ADSDronePawn::ChangeDroneMode(EDroneMode InDroneMode)
{
	if (DroneMode == EDroneMode::TargetView && InDroneMode != EDroneMode::TargetView)
	{
		SetActorLocation(PrevLocation);
		CameraBoom->TargetArmLength = PrevBoomLength;
		MeshComponent->SetVisibility(true);
	}
	else if (DroneMode != EDroneMode::TargetView && InDroneMode == EDroneMode::TargetView)
	{
		PrevLocation = GetActorLocation();
		PrevBoomLength = CameraBoom->TargetArmLength;
		MeshComponent->SetVisibility(false);
	}

	DroneMode = InDroneMode;
	bDroneOperation = DroneMode != EDroneMode::Setting && DroneMode != EDroneMode::TargetView;
	//MeshComponent->SetVisibility(bBoolean);
	CameraBoom->bDoCollisionTest = bDroneOperation;
if (DroneMode == EDroneMode::Waypoint)
	{
		UDSSaveGame* DroneData = LoadGame();
		if (WpActor->GetWaypoint().Points.Num() > 0)
		{
			SetActorLocation(WpActor->GetWaypoint().Points[0].Location);
			CurrentPointIndex = 0;
		}
	}
	
}

void ADSDronePawn::GotoCurrentTarget()
{
	UDSSaveGame* DroneData = LoadGame();
	if (DroneData->CurrentTarget!=nullptr)
	{
		SetActorLocation(DroneData->CurrentTarget->GetActorLocation());
	}
}
