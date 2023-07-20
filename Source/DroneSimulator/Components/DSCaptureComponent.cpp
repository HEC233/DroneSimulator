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
		MyRenderTarget = RenderTargetRef.Object;
	}

	TargetFilteringName = FName(TEXT("NoTarget"));
}


void UDSCaptureComponent::BeginPlay()
{
	Super::BeginPlay();

	TextureTarget = MyRenderTarget;
}


void UDSCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LookTarget();
}

void UDSCaptureComponent::TakeScreenShot(int32 CaptureIndex)
{
	if (TextureTarget == nullptr || CaptureTargetActor == nullptr)
	{
		return;
	}

	LookTarget();
	const FDateTime CurrentTime = FDateTime::Now();
	const FString TimeString = CurrentTime.ToString(TEXT("%Y.%m.%d-%H.%M.%S"));
	FString TargetName = CaptureTargetActor->GetActorNameOrLabel();

	FProperty* NameProp = CaptureTargetActor->GetClass()->FindPropertyByName(TEXT("TargetName"));
	if (NameProp)
	{
		NameProp->GetValue_InContainer(CaptureTargetActor, &TargetName);
	}

	FString ImageFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s Image - %s.jpg"), CaptureIndex, *TargetName, *TimeString));
	FString TextFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s Image - %s.txt"), CaptureIndex, *TargetName, *TimeString));
	FPaths::MakeStandardFilename(ImageFilePath);
	FPaths::MakeStandardFilename(TextFilePath);

	//SceneCapture->CaptureScene();
	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*ImageFilePath);
	if (RawFileWriterAr == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return;
	}
	bool ImageSavedOK = ExportRenderTargetJPG(TextureTarget, *RawFileWriterAr);
	RawFileWriterAr->Close();

	FVector2D Min, Max;
	CalculateNDCMinMax(Min, Max);

	Min = (Min / 2) + FVector2D(0.5f, 0.5f);
	Max = (Max / 2) + FVector2D(0.5f, 0.5f);

	Min.X *= TextureTarget->SizeX;
	Max.X *= TextureTarget->SizeX;
	Min.Y *= TextureTarget->SizeY;
	Max.Y *= TextureTarget->SizeY;
	Min.Y = TextureTarget->SizeY - Min.Y;
	Max.Y = TextureTarget->SizeY - Max.Y;

	float SwapTemp = Min.Y;
	Min.Y = Max.Y;
	Max.Y = SwapTemp;

	/*FVector2f Center = (Min + Max) / 2.0f;
	Min = Center + (Min - Center) * BoxSizeMultiplier;
	Max = Center + (Max - Center) * BoxSizeMultiplier;*/

	int32 MinX = FMath::FloorToInt(Min.X);
	int32 MinY = FMath::FloorToInt(Min.Y);
	int32 MaxX = FMath::CeilToInt(Max.X);
	int32 MaxY = FMath::CeilToInt(Max.Y);

	int32 StringSharpIdx;
	if (!TargetName.FindChar(TCHAR('#'), StringSharpIdx))
	{
		StringSharpIdx = TargetName.Len();
	}
	UE_LOG(LogTemp, Log, TEXT("Min: %s, Max: %s"), *Min.ToString(), *Max.ToString());
	FString LabelingText = FString::Printf(TEXT("%s,%d,%d,%d,%d"), *TargetName.Mid(0, StringSharpIdx), MinX, MinY, MaxX - MinX, MaxY - MinY);
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

void UDSCaptureComponent::SetTarget(const AActor* TargetActor)
{
	CaptureTargetActor = (TargetActor);
	SetLookAtPos(CaptureTargetActor->GetActorLocation());
}

const TObjectPtr<const AActor> UDSCaptureComponent::GetTarget()
{
	return CaptureTargetActor;
}

void UDSCaptureComponent::SetLookAtPos(const FVector& Pos)
{
	LookAtPos = Pos;
}

const FVector& UDSCaptureComponent::GetLookAtPos()
{
	return LookAtPos;
}

void UDSCaptureComponent::SetCameraFOV(float FOV)
{
	CurrentFOV = FOV;
	SetFinalFOV();
}

void UDSCaptureComponent::SetZoomRate(float InZoomRate)
{
	ZoomRate = InZoomRate;
	SetFinalFOV();
}

void UDSCaptureComponent::SetCaptureTick(bool bValue)
{
	bCaptureEveryFrame = bValue;
}

void UDSCaptureComponent::CalculateNDCMinMax(FVector2D& OutMin, FVector2D& OutMax)
{
	if (CaptureTargetActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Target is not exist!"));
		return;
	}

	OutMin = FVector2D(TNumericLimits<double>::Max(), TNumericLimits<double>::Max());
	OutMax = FVector2D(TNumericLimits<double>::Min(), TNumericLimits<double>::Min());

	TArray<UStaticMeshComponent*> CapturingMeshComponents;
	TArray<AActor*> AttachedActors;

	CaptureTargetActor->GetAttachedActors(AttachedActors, true, true);
	AttachedActors.Add(const_cast<AActor*>(CaptureTargetActor.Get()));

	for (AActor* Actor : AttachedActors)
	{
		TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents<UStaticMeshComponent>(Components);
		for (UStaticMeshComponent* Component : Components)
		{
			if (Component->IsVisible() && !Component->ComponentHasTag(TargetFilteringName))
			{
				CapturingMeshComponents.Emplace(Component);
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
	FMinimalViewInfo ViewInfo;
	GetCameraView(0.0f, ViewInfo);
	ViewInfo.AspectRatio = TextureTarget->SizeX / TextureTarget->SizeY;
	const FMatrix ViewMatrix = FTranslationMatrix(-GetComponentLocation()) * FInverseRotationMatrix(GetComponentRotation()) * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	const FMatrix ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
	ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;

	for (UStaticMeshComponent* MeshComp : CapturingMeshComponents)
	{
		FBoxSphereBounds Bds = MeshComp->GetStaticMesh()->GetBounds();

		TArray<FVector> Points;
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(1, 1, 1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(1, 1, -1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(1, -1, 1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(1, -1, -1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(-1, 1, 1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(-1, 1, -1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(-1, -1, 1)));
		Points.Add(MeshComp->GetRenderMatrix().TransformPosition(Bds.Origin + Bds.BoxExtent * FVector(-1, -1, -1)));

		for (FVector& Point : Points)
		{
			FVector4 ClipCoordinate = ViewProjectionMatrix.TransformPosition(Point);
			FVector2d NDCCoordinate = FVector2d(ClipCoordinate.X, ClipCoordinate.Y) / ClipCoordinate.W;

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

	UE_LOG(LogTemp, Log, TEXT("Image Buffer's size : %d"), (Image.RawData.GetAllocatedSize()));

	TArray64<uint8> CompressedData;
	if (!FImageUtils::CompressImage(CompressedData, TEXT("jpg"), Image, 100))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Compressed Image Buffer's size : %d"), CompressedData.GetAllocatedSize());

	Ar.Serialize((void*)CompressedData.GetData(), CompressedData.GetAllocatedSize());

	return true;
}

void UDSCaptureComponent::LookTarget()
{
	FVector Direction = LookAtPos - GetComponentLocation();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
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
	SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
}

void UDSCaptureComponent::SetFinalFOV()
{
	FOVAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(FMath::DegreesToRadians(CurrentFOV) / ZoomRate))), 0.001f, 360.0f);
}

