// Fill out your copyright notice in the Description page of Project Settings.


#include "DSDronePawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/UnrealMathUtility.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtils.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/FileManagerGeneric.h"

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

	static ConstructorHelpers::FObjectFinder<UInputAction> InputScreenShotAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/IA_TakeScreenShot.IA_TakeScreenShot'"));
	if (InputScreenShotAction.Object)
	{
		TakeScreenShotAction = InputScreenShotAction.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeTargetAction(TEXT("/Script/EnhancedInput.InputAction'/Game/DroneSimulator/IA_ChangeTarget.IA_ChangeTarget'"));
	if (InputChangeTargetAction.Object)
	{
		ChangeTargetAction = InputChangeTargetAction.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/DroneSimulator/IMC_Default.IMC_Default'"));
	if (InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetRef(TEXT("/Script/Engine.TextureRenderTarget2D'/Game/DroneSimulator/NewTextureRenderTarget2D.NewTextureRenderTarget2D'"));
	if (RenderTargetRef.Object)
	{
		RenderTarget = RenderTargetRef.Object;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture2D"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetWorldLocation(FVector(0.f, 0.f, -15.f));

	RotationRadius = 10000.f;
	DroneSpeed = 1000.f;
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

	CenterPosition = GetActorLocation();

	UpdateDroneSpeed();

	CurrentRotationRate = 0.0f;

	if (RenderTarget)
	{
		SceneCapture->TextureTarget = RenderTarget;
	}

	static FName Tag(TEXT("DroneTarget"));
	if (TargetActor == nullptr)
	{
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->ActorHasTag(Tag))
			{
				TargetActorList.Add(*ActorItr);
			}
		}
		if (TargetActorList.Num() != 0)
		{
			CurrentTargetIndex = 0;
			TargetActor = TargetActorList[CurrentTargetIndex];
		}
	}
}

// Called every frame
void ADSDronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveDrone(DeltaTime);
	LookCamera(TargetActor);
}

// Called to bind functionality to input
void ADSDronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(TakeScreenShotAction, ETriggerEvent::Started, this, &ADSDronePawn::TakeScreenShot);
	EnhancedInputComponent->BindAction(ChangeTargetAction, ETriggerEvent::Triggered, this, &ADSDronePawn::ChangeTarget);
}

void ADSDronePawn::TakeScreenShot()
{
	static int8 ScreenShotCount = 0;
	UE_LOG(LogTemp, Log, TEXT("Take Screen shot here"));

	if (RenderTarget == nullptr)
	{
		return;
	}

	FString FilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("TestImage%d.png"), ScreenShotCount++));
	FPaths::MakeStandardFilename(FilePath);
	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*FilePath);
	if (RawFileWriterAr == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return;
	}
	bool ImageSavedOK = FImageUtils::ExportRenderTarget2DAsPNG(RenderTarget, *RawFileWriterAr);
	RawFileWriterAr->Close();

	/*FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags readPixelFlags(RCM_UNorm);

	TArray<FColor> OutBMP;
	OutBMP.AddUninitialized(RenderTarget->GetSurfaceWidth() * RenderTarget->GetSurfaceHeight());
	Resource->ReadPixels(OutBMP, readPixelFlags);

	for (FColor& color : OutBMP)
	{
		color.A = 255;
	}

	FIntPoint DestSize(RenderTarget->GetSurfaceWidth(), RenderTarget->GetSurfaceHeight());
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(DestSize.X, DestSize.Y, OutBMP, CompressedBitmap);

	FString FullPath = FPaths::Combine("C:\\", "test.png");

	bool ImageSavedOK = FFileHelper::SaveArrayToFile(CompressedBitmap, *FullPath);*/

	if (ImageSavedOK)
	{
		UE_LOG(LogTemp, Log, TEXT("Save successed"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Save failed"));
	}
}

void ADSDronePawn::ChangeTarget()
{
	UE_LOG(LogTemp, Log, TEXT("Change Target!!"));
	if (CurrentTargetIndex != -1)
	{
		CurrentTargetIndex = (CurrentTargetIndex + 1) % TargetActorList.Num();
		TargetActor = TargetActorList[CurrentTargetIndex];
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

void ADSDronePawn::LookCamera(AActor* Target)
{
	if (Target == nullptr)
	{
		return;
	}

	FVector Direction = Target->GetActorLocation() - this->GetActorLocation();
	Direction.Normalize();

	// This code does same thing below... but I didn't know that and did it myself...
	//FRotator LookRotator = Direction.Rotation();

	FVector XYDirection = FVector(Direction.X, Direction.Y, 0.f);
	
	// value of pitch angle cosine
	float XYLength = XYDirection.Length();
	XYDirection.Normalize();

	float PitchRad = FMath::Atan2(Direction.Z, XYLength);
	float YawRad = FMath::Atan2(XYDirection.Y, XYDirection.X);

	float Pitch = FMath::RadiansToDegrees(PitchRad);
	float Yaw = FMath::RadiansToDegrees(YawRad);

	//FRotator LookAtRotator = FLookFromMatrix(this->GetActorLocation(), Direction, FVector::UpVector).Rotator();
	FollowCamera->SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
	//FollowCamera->SetWorldRotation(LookRotator);
	SceneCapture->SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
}

void ADSDronePawn::UpdateDroneSpeed()
{
	float Diameter = RotationRadius * 2.0f * PI;
	float RotatingTime = Diameter / DroneSpeed;
	AngleSpeed = (2.0f * PI) / RotatingTime;
}

