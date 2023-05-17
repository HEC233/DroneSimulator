// Fill out your copyright notice in the Description page of Project Settings.


#include "DSDronePawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtils.h"
#include "EngineUtils.h"
#include "DroneSimulator/DSSaveGame.h"
#include "HAL/FileManager.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Math/NumericLimits.h"
#include "Math/TranslationMatrix.h"
#include "Engine/LocalPlayer.h"

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
	SceneCapture->TextureTarget = RenderTarget;

	RotationRadius = 10000.f;
	DroneSpeed = 1000.f;
}

// Called when the game starts or when spawned
void ADSDronePawn::BeginPlay()
{
	Super::BeginPlay();

	// APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	// if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	// {
	// 	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	// }
	
	ApplyLoadData();
	
	CurrentCaptureCount = 0;

	CurrentRotationRate = 0.0f;

	// static FName Tag(TEXT("DroneTarget"));
	// if (TargetActor == nullptr)
	// {
	// 	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	// 	{
	// 		if (ActorItr->ActorHasTag(Tag))
	// 		{
	// 			TargetActorList.Add(*ActorItr);
	// 		}
	// 	}
	// 	if (TargetActorList.Num() != 0)
	// 	{
	// 		CurrentTargetIndex = 0;
	// 		TargetActor = TargetActorList[CurrentTargetIndex];
	// 	}
	// }
}

// Called every frame
void ADSDronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyLoadData();
	
	MoveDrone(DeltaTime);
	LookCamera(TargetActor);

	if (IsCapture)
	{
		if (CurrentCaptureCount < MaxCaptureCount)
		{
			TimeRecord += DeltaTime;
		
			if (TimeRecord <= 60.0f / CaptureSpeedPerMinute)
			{
				TakeScreenShot();
		
				TimeRecord -= 60.0f / CaptureSpeedPerMinute;
				CurrentCaptureCount++;
			}
		}	
	}
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
	if (RenderTarget == nullptr || TargetActor == nullptr)
	{
		return;
	}

	const FDateTime CurrentTime = FDateTime::UtcNow();
	const FString TimeString = CurrentTime.ToString(TEXT("%Y.%m.%d-%H.%M.%S"));
	const FString TargetName = TargetActor->GetActorLabel();
	
	FString ImageFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("%s Image - %s.png"), *TargetName, *TimeString));
	FString TextFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("%s Image - %s.txt"), *TargetName, *TimeString));
	FPaths::MakeStandardFilename(ImageFilePath);
	FPaths::MakeStandardFilename(TextFilePath);

	SceneCapture->CaptureScene();

	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*ImageFilePath);
	if (RawFileWriterAr == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return;
	}
	bool ImageSavedOK = FImageUtils::ExportRenderTarget2DAsPNG(RenderTarget, *RawFileWriterAr);
	RawFileWriterAr->Close();

	FVector2f Min, Max;
	CalculateNDCMinMax(Min, Max);

	Min = (Min / 2) + FVector2f(0.5f, 0.5f);
	Max = (Max / 2) + FVector2f(0.5f, 0.5f);

	Min.X *= RenderTarget->SizeX;
	Max.X *= RenderTarget->SizeX;
	Min.Y *= RenderTarget->SizeY;
	Max.Y *= RenderTarget->SizeY;
	Min.Y = RenderTarget->SizeY - Min.Y;
	Max.Y = RenderTarget->SizeY - Max.Y;

	float SwapTemp = Min.Y;
	Min.Y = Max.Y;
	Max.Y = SwapTemp;

	/*FVector2f Center = (Min + Max) / 2.0f;
	Min = Center + (Min - Center) * BoxSizeMultiplier;
	Max = Center + (Max - Center) * BoxSizeMultiplier;*/

	UE_LOG(LogTemp, Log, TEXT("Min: %s, Max: %s"), *Min.ToString(), *Max.ToString());
	FString LabelingText = FString::Printf(TEXT("%s,%d,%d,%d,%d"), *TargetName, 
		FMath::FloorToInt(Min.X), FMath::FloorToInt(Min.Y), FMath::CeilToInt(Max.X - Min.X), FMath::CeilToInt(Max.Y - Min.Y));
	FFileHelper::SaveStringToFile(*LabelingText, *TextFilePath);

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

void ADSDronePawn::CalculateNDCMinMax(FVector2f& OutMin, FVector2f& OutMax)
{
	if (TargetActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Target not exist!!!"));
		return;
	}

	OutMin = FVector2f(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
	OutMax = FVector2f(TNumericLimits<float>::Min(), TNumericLimits<float>::Min());

	TArray<UActorComponent*> StaticMeshComponents;
	TArray<AActor*> AttachedActors;

	TargetActor->GetAttachedActors(AttachedActors, true, true);

	for (AActor* Actor : AttachedActors)
	{
		StaticMeshComponents += Actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
	}

	//APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	//ULocalPlayer* const LocalPlayer = PlayerController->GetLocalPlayer();

	//FSceneViewProjectionData ProjectionData;
	//if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, /*out*/ ProjectionData))
	//{
	//	ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
	//}

	FMatrix ViewProjectionMatrix = FMatrix::Identity;
	const FTransform SceneCaptureTransform = SceneCapture->GetComponentTransform();
	FMinimalViewInfo ViewInfo;
	SceneCapture->GetCameraView(0.0f, ViewInfo);
	const FMatrix ViewMatrix = FTranslationMatrix(-SceneCaptureTransform.GetLocation()) * FInverseRotationMatrix(SceneCaptureTransform.Rotator()) * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	const FMatrix ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
	ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;

	for (UActorComponent* ActorComponent : StaticMeshComponents)
	{
		UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ActorComponent);

		FBox BoundingBox = StaticMeshComp->GetStaticMesh()->GetBoundingBox();

		TArray<FVector> Points;
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Min.X, BoundingBox.Min.Y, BoundingBox.Min.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Min.X, BoundingBox.Min.Y, BoundingBox.Max.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Min.X, BoundingBox.Max.Y, BoundingBox.Min.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Min.X, BoundingBox.Max.Y, BoundingBox.Max.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Max.X, BoundingBox.Min.Y, BoundingBox.Min.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Max.X, BoundingBox.Min.Y, BoundingBox.Max.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Max.X, BoundingBox.Max.Y, BoundingBox.Min.Z)));
		Points.Add(StaticMeshComp->GetRenderMatrix().TransformPosition(FVector(BoundingBox.Max.X, BoundingBox.Max.Y, BoundingBox.Max.Z)));

		for (FVector& Point : Points)
		{
			FVector4 ClipCoordinate = ViewProjectionMatrix.TransformPosition(Point);
			FVector NDCCoordinate = FVector(ClipCoordinate.X, ClipCoordinate.Y, ClipCoordinate.Z) / ClipCoordinate.W;

			OutMin.X = FMath::Min(NDCCoordinate.X, OutMin.X);
			OutMin.Y = FMath::Min(NDCCoordinate.Y, OutMin.Y);

			OutMax.X = FMath::Max(NDCCoordinate.X, OutMax.X);
			OutMax.Y = FMath::Max(NDCCoordinate.Y, OutMax.Y);
		}
	}

	//OutMin.Y /= LocalPlayer->ViewportClient->Viewport->GetDesiredAspectRatio();
	//OutMax.Y /= LocalPlayer->ViewportClient->Viewport->GetDesiredAspectRatio();
}

UDSSaveGame* ADSDronePawn::LoadGame()
{
	const auto SaveSlot = Cast<UDSSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSetting",0));
	return SaveSlot;
}

void ADSDronePawn::ApplyLoadData()
{
	UDSSaveGame* DroneData = LoadGame();

	CenterPosition = GetActorLocation();

	if (DroneData != nullptr)
	{
		if (DroneData->CurrentTarget != nullptr)
		{
			TargetActor = DroneData->CurrentTarget;
			CenterPosition = DroneData->CurrentTarget->GetActorLocation() + FVector(0, 0, DroneData->CurrentHeight);
		}

		RotationRadius = DroneData->CurrentRadius;
		DroneSpeed = DroneData->CurrentMoveSpeed;
		SceneCapture->FOVAngle = DroneData->CurrentFOV;

		CaptureSpeedPerMinute = DroneData->CurrentCaptureSpeed;
	}

	UpdateDroneSpeed();
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

