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
	CaptureComponent->AttachCamera(RootComponent);
	CaptureComponent->SetCameraPosition(FVector(0.f, 0.f, -15.f));

	RotationRadius = 10.f;
	DroneSpeed = 15.f;
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

	ApplyLoadData();
	ChangeDroneMode(false);

	CurrentCaptureCount = 0;

	CurrentRotationRate = 0.0f;

	/*FInputModeGameAndUI GameAndUI;
	GameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	PlayerController->SetInputMode(GameAndUI);*/
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

	if (bDroneMode)
	{
		MoveDrone(DeltaTime);
		MoveDroneWithWaypoint(DeltaTime);
	}
	// else
	// {
	// 	if (CaptureComponent->GetTarget())
	// 	{
	// 		SetActorLocation(CaptureComponent->GetTarget()->GetActorTransform().GetLocation());
	// 	}
	// }

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
}

void ADSDronePawn::ProcessMouseInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	FTransform BoomTransform = CameraBoom->GetRelativeTransform();
	FRotator BoomRotator = BoomTransform.Rotator();

	BoomRotator.Yaw = BoomRotator.Yaw + LookAxisVector.X;
	BoomRotator.Pitch = FMath::Clamp(BoomRotator.Pitch + LookAxisVector.Y, -80.0f, bDroneMode ? 80.0f : -10.0f);
	BoomTransform.SetRotation(BoomRotator.Quaternion());

	CameraBoom->SetRelativeTransform(BoomTransform);
}

void ADSDronePawn::StartCapture()
{
	if (!bDroneMode)
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

UDSSaveGame* ADSDronePawn::LoadGame()
{
	const auto SaveSlot = Cast<UDSSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSetting", 0));
	return SaveSlot;
}

void ADSDronePawn::ApplyLoadData()
{
	UDSSaveGame* DroneData = LoadGame();

	if (DroneData != nullptr)
	{
		if (DroneData->CurrentTarget != nullptr && DroneData->CurrentTarget != CaptureComponent->GetTarget())
		{
			CenterPosition = DroneData->CurrentTarget->GetActorLocation() + FVector(0, 0, DroneData->CurrentHeight * 100.0f);

			CaptureComponent->SetTarget(DroneData->CurrentTarget);
		}

		RotationRadius = DroneData->CurrentRadius * 100.0f;
		AngleSpeed = DroneData->CurrentMoveSpeed / 60.0f;

		CaptureSpeedPerSecond = DroneData->CurrentCaptureSpeed;
		//bDroneManualMove = !DroneData->AutoPilot;

		CaptureComponent->SetCameraFOV(DroneData->CurrentFOV);
	}

	//UpdateDroneSpeed();
}

void ADSDronePawn::MoveDrone(float DeltaTime)
{
	UDSSaveGame* DroneData = LoadGame();
	if (DroneData->PilotMode != EPilotMode::E_AutoMode) return;

	FVector NewPosition = CenterPosition;
	CurrentRotationRate = FMath::Fmod((CurrentRotationRate + AngleSpeed * DeltaTime), 2.0f * PI);
	float SinValue, CosValue;
	FMath::SinCos(&SinValue, &CosValue, CurrentRotationRate);

	NewPosition += FVector(SinValue, CosValue, 0) * RotationRadius;

	SetActorLocation(NewPosition);
}

void ADSDronePawn::MoveDroneWithWaypoint(float DeltaTime)
{
	//TODO: 현재 선택된 웨이포인트의 각 포인트들을 순서대로 이동해야함
	
	UDSSaveGame* DroneData = LoadGame();
	if (DroneData->PilotMode != EPilotMode::E_WayPointMode) return;
	
	DroneData->CurrentWayPoint.Points; //현재 웨이포인트의 각 포인트(FVector) 들의 배열
}

void ADSDronePawn::MoveDroneWithInput(const FInputActionValue& Value)
{
	UDSSaveGame* DroneData = LoadGame();
	if (bDroneMode && DroneData->PilotMode != EPilotMode::E_ManualMode) return;
	
	FVector2D Input = Value.Get<FVector2D>();

	const FRotator Rotation = CameraBoom->GetComponentRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector Movement = ForwardDirection * Input.Y + RightDirection * Input.X;

	SetActorLocation(GetActorLocation() + Movement * DroneSpeed);
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
}

FString ADSDronePawn::GetCaptureInfo()
{
	return FString::Printf(TEXT("캡쳐 진행시간 : %.2f\n현재 캡쳐 수 %d / %d"), CaptureTimeDuration, CurrentCaptureCount, MaxCaptureCount);
}

void ADSDronePawn::ChangeDroneMode(bool bBoolean)
{
	bDroneMode = bBoolean;
	//MeshComponent->SetVisibility(bBoolean);
	CameraBoom->TargetArmLength = 400.0f; //bBoolean ? 400.0f : 2000.0f;
	CameraBoom->bDoCollisionTest = bBoolean;

	if (bDroneMode)
	{
		GotoCurrentTarget();
	}
}

void ADSDronePawn::GotoCurrentTarget()
{
	UDSSaveGame* DroneData = LoadGame();
	if (DroneData->CurrentTarget!=nullptr)
	{
		SetActorLocation(DroneData->CurrentTarget->GetActorLocation() + FVector(0, 0, DroneData->CurrentHeight * 100.0f));
	}
}
