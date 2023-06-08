// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"

UDSCaptureComponent::UDSCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetRef(TEXT("/Script/Engine.TextureRenderTarget2D'/Game/DroneSimulator/NewTextureRenderTarget2D.NewTextureRenderTarget2D'"));
	if (RenderTargetRef.Object)
	{
		RenderTarget = RenderTargetRef.Object;
	}

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->bCaptureEveryFrame = true;

	TargetFilteringName = FName(TEXT("NoTarget"));
}


void UDSCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UDSCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LookTarget();
}

void UDSCaptureComponent::TakeScreenShot(int32 CaptureIndex)
{
	if (RenderTarget == nullptr || CaptureTargetActor == nullptr)
	{
		return;
	}

	const FDateTime CurrentTime = FDateTime::Now();
	const FString TimeString = CurrentTime.ToString(TEXT("%Y.%m.%d-%H.%M.%S"));
	FString TargetName = CaptureTargetActor->GetActorNameOrLabel();

	FProperty* NameProp = CaptureTargetActor->GetClass()->FindPropertyByName(TEXT("TargetName"));
	if (NameProp)
	{
		NameProp->GetValue_InContainer(CaptureTargetActor, &TargetName);
	}

	FString ImageFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%d - %s Image - %s.jpg"), CaptureIndex, *TargetName, *TimeString));
	FString TextFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%d - %s Image - %s.txt"), CaptureIndex, *TargetName, *TimeString));
	FPaths::MakeStandardFilename(ImageFilePath);
	FPaths::MakeStandardFilename(TextFilePath);

	//SceneCapture->CaptureScene();

	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*ImageFilePath);
	if (RawFileWriterAr == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return;
	}
	bool ImageSavedOK = ExportRenderTargetJPG(RenderTarget, *RawFileWriterAr);
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
		UE_LOG(LogTemp, Log, TEXT("Capture Save successed"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Capture Save failed"));
	}
}

void UDSCaptureComponent::SetTarget(AActor* TargetActor)
{
	CaptureTargetActor = TargetActor;
}

const TObjectPtr<AActor> UDSCaptureComponent::GetTarget()
{
	return CaptureTargetActor;
}

void UDSCaptureComponent::SetCameraPosition(FVector RelativePosition)
{
	SceneCapture->SetRelativeLocation(RelativePosition);
}

void UDSCaptureComponent::AttachCamera(USceneComponent* Parent)
{
	SceneCapture->SetupAttachment(Parent);
}

void UDSCaptureComponent::SetCameraFOV(float FOV)
{
	SceneCapture->FOVAngle = FOV;
}

void UDSCaptureComponent::CalculateNDCMinMax(FVector2f& OutMin, FVector2f& OutMax)
{
	if (CaptureTargetActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Target is not exist!"));
		return;
	}

	OutMin = FVector2f(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
	OutMax = FVector2f(TNumericLimits<float>::Min(), TNumericLimits<float>::Min());

	TArray<UActorComponent*> StaticMeshComponents;
	TArray<AActor*> AttachedActors;

	CaptureTargetActor->GetAttachedActors(AttachedActors, true, true);
	AttachedActors.Add(CaptureTargetActor);

	for (AActor* Actor : AttachedActors)
	{
		TArray<USceneComponent*> SceneComps;
		Actor->GetComponents<USceneComponent>(SceneComps);
		for (USceneComponent* Component : SceneComps)
		{
			if (Component->IsVisible())
			{
				TArray<UStaticMeshComponent*> Components;
				Actor->GetComponents<UStaticMeshComponent>(Components);
				for (UStaticMeshComponent* MeshComponent : Components)
				{
					if (!MeshComponent->ComponentHasTag(TargetFilteringName))
					{
						StaticMeshComponents.Emplace(MeshComponent);
					}
				}
			}
		}
		/*TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents<UStaticMeshComponent>(Components);
		for (UStaticMeshComponent* Component : Components)
		{
			if (!Component->ComponentHasTag(TargetFilteringName))
			{
				StaticMeshComponents.Emplace(Component);
			}
		}*/
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

bool UDSCaptureComponent::ExportRenderTargetJPG(UTextureRenderTarget2D* TexRT, FArchive& Ar)
{
	FImage Image;
	if (!FImageUtils::GetRenderTargetImage(TexRT, Image))
	{
		return false;
	}

	TArray64<uint8> CompressedData;
	if (!FImageUtils::CompressImage(CompressedData, TEXT("jpg"), Image, 100))
	{
		return false;
	}

	Ar.Serialize((void*)CompressedData.GetData(), CompressedData.GetAllocatedSize());

	return true;
}

void UDSCaptureComponent::LookTarget()
{
	if (CaptureTargetActor == nullptr)
	{
		return;
	}

	FVector Direction = CaptureTargetActor->GetActorLocation() - SceneCapture->GetComponentLocation();
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
	//FollowCamera->SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
	//FollowCamera->SetWorldRotation(LookRotator);
	SceneCapture->SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
}

