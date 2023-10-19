// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Controller/DSPlayerController.h"
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
	bCaptureEveryFrame = false;
	bAlwaysPersistRenderingState = true;
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
	CaptureScene();
}

bool UDSCaptureComponent::TakeScreenShot(int32 CaptureIndex)
{
	if (TextureTarget == nullptr || CaptureTargetActor == nullptr)
	{
		return false;
	}

	LookTarget();
	CaptureScene();
	const FDateTime CurrentTime = FDateTime::Now();
	const FString TimeString = CurrentTime.ToString(TEXT("%Y.%m.%d-%H.%M.%S"));

	TArray<const AActor*> Targets;

	ADSPlayerController* PC = Cast<ADSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		FConvexVolume Volume;
		GetViewFrustumBounds(Volume, GetViewProjection(), false);
		Targets.Append(PC->GetTargetsInVolume(Volume));
	}
	else
	{
		Targets.Add(CaptureTargetActor);
	}

	FString TargetName = CaptureTargetActor->GetActorNameOrLabel();
	FString TargetAbsoulteName = TargetName;

	FProperty* NameProp = CaptureTargetActor->GetClass()->FindPropertyByName(TEXT("TargetName"));
	if (NameProp)
	{
		NameProp->GetValue_InContainer(CaptureTargetActor, &TargetName);
		
	}

	NameProp = CaptureTargetActor->GetClass()->FindPropertyByName(TEXT("TargetAbsoluteName"));
	if (NameProp)
	{
		NameProp->GetValue_InContainer(CaptureTargetActor, &TargetAbsoulteName);
	}


	FString ImageFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s - %s.jpg"), CaptureIndex, *TargetAbsoulteName, *TimeString));
	FString TextFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s - %s.txt"), CaptureIndex, *TargetAbsoulteName, *TimeString));
	FPaths::MakeStandardFilename(ImageFilePath);
	FPaths::MakeStandardFilename(TextFilePath);

	FString LabelingText;

	//UE_LOG(LogTemp, Log, TEXT("==="));
	//UE_LOG(LogTemp, Log, TEXT("Filtering Start! Target Amount : %d"), Targets.Num());

	for (const AActor* Target : Targets)
	{
		NameProp = Target->GetClass()->FindPropertyByName(TEXT("TargetName"));
		if (NameProp)
		{
			NameProp->GetValue_InContainer(Target, &TargetName);
		}

		//UE_LOG(LogTemp, Log, TEXT("Target : %s"), *TargetName);

		FVector2D Min, Max;
		CalculateNDCMinMax(Target, Min, Max, false);

		if (!CheckTargetCoordInvalid(Min, Max))
		{
			continue;
		}

		FIntVector2 ImageMin, ImageMax;
		ConvertNDC2ImageCoord(Min, Max, ImageMin, ImageMax);

		int32 Width = ImageMax.X - ImageMin.X;
		int32 Height = ImageMax.Y - ImageMin.Y;
		float Altitude = GetComponentLocation().Z - Target->GetActorLocation().Z;
		float CameraAngle = -GetComponentRotation().Pitch;

		NameProp = Target->GetClass()->FindPropertyByName(TEXT("TargetAbsoluteName"));
		if (NameProp)
		{
			NameProp->GetValue_InContainer(Target, &TargetAbsoulteName);
		}

		//UE_LOG(LogTemp, Log, TEXT("Target was successfully captured"));
		LabelingText += FString::Printf(TEXT("%s,%d,%d,%d,%d,%.2f,%.2f\n"), *TargetAbsoulteName, ImageMin.X, ImageMin.Y, Width, Height, Altitude, CameraAngle);
	}

	// Save to disk
	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*ImageFilePath);
	if (RawFileWriterAr == nullptr)
	{
		//UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return false;
	}
	bool ImageSavedOK = ExportRenderTargetJPG(TextureTarget, *RawFileWriterAr);
	RawFileWriterAr->Close();
	FFileHelper::SaveStringToFile(*LabelingText, *TextFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	if (ImageSavedOK)
	{
		UE_LOG(LogTemp, Log, TEXT("Capture Save successed"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Capture Save failed"));
	}
	return ImageSavedOK;
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

void UDSCaptureComponent::SetTargetFilterRate(float InRate)
{
	TargetFilterRate = FMath::Clamp(InRate, 0.0f, 1.0f);
}

void UDSCaptureComponent::SetAdditionalAngle(float InAngle)
{
	AdditionalAngle = InAngle;
}

const float UDSCaptureComponent::GetAdditionalAngle()
{
	return AdditionalAngle;
}

const TArray<FVector4>& UDSCaptureComponent::GetTargetCoordinated()
{
	TArray<const AActor*> Targets;

	ADSPlayerController* PC = Cast<ADSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		FConvexVolume Volume;
		GetViewFrustumBounds(Volume, GetViewProjection(), false);
		Targets.Append(PC->GetTargetsInVolume(Volume));
	}
	else
	{
		Targets.Add(CaptureTargetActor);
	}

	TargetCoordinatesBuffer.Empty(Targets.Num());

	for (const AActor* Target : Targets)
	{
		FVector2D Min, Max;
		CalculateNDCMinMax(Target, Min, Max, false);

		if (!CheckTargetCoordInvalid(Min, Max))
		{
			continue;
		}

		TargetCoordinatesBuffer.Emplace(FVector4(FMath::Max(-1.0, Min.X), FMath::Max(-1.0, Min.Y), FMath::Min(1.0, Max.X), FMath::Min(1.0, Max.Y)));
	}

	return TargetCoordinatesBuffer;
}

FMatrix UDSCaptureComponent::GetViewProjection()
{
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

	return ViewProjectionMatrix;
}

void UDSCaptureComponent::CalculateNDCMinMax(const AActor* Target, FVector2D& OutMin, FVector2D& OutMax, bool bFilterOutScreen)
{
	if (Target == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Target is not exist!"));
		return;
	}

	OutMin = FVector2D(TNumericLimits<double>::Max(), TNumericLimits<double>::Max());
	OutMax = FVector2D(TNumericLimits<double>::Lowest(), TNumericLimits<double>::Lowest());

	TArray<UStaticMeshComponent*> CapturingMeshComponents;
	TArray<AActor*> AttachedActors;

	Target->GetAttachedActors(AttachedActors, true, true);
	AttachedActors.Add(const_cast<AActor*>(Target));

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
	}

	const FMatrix ViewProjectionMatrix = GetViewProjection();

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

		FVector2D InnerMin = FVector2D(TNumericLimits<double>::Max(), TNumericLimits<double>::Max());
		FVector2D InnerMax = FVector2D(TNumericLimits<double>::Lowest(), TNumericLimits<double>::Lowest());

		for (FVector& Point : Points)
		{
			FVector4 ClipCoordinate = ViewProjectionMatrix.TransformPosition(Point);
			FVector2d NDCCoordinate = FVector2d(ClipCoordinate.X, ClipCoordinate.Y) / ClipCoordinate.W;

			InnerMin.X = FMath::Min(NDCCoordinate.X, InnerMin.X);
			InnerMin.Y = FMath::Min(NDCCoordinate.Y, InnerMin.Y);

			InnerMax.X = FMath::Max(NDCCoordinate.X, InnerMax.X);
			InnerMax.Y = FMath::Max(NDCCoordinate.Y, InnerMax.Y);
		}

		if (bFilterOutScreen && (InnerMin.X > 1.0f || InnerMin.Y > 1.0f || InnerMax.X < -1.0f || InnerMax.Y < -1.0f))
		{
			continue;
		}

		OutMin.X = FMath::Min(InnerMin.X, OutMin.X);
		OutMin.Y = FMath::Min(InnerMin.Y, OutMin.Y);

		OutMax.X = FMath::Max(InnerMax.X, OutMax.X);
		OutMax.Y = FMath::Max(InnerMax.Y, OutMax.Y);
	}

	//OutMin.Y /= LocalPlayer->ViewportClient->Viewport->GetDesiredAspectRatio();
	//OutMax.Y /= LocalPlayer->ViewportClient->Viewport->GetDesiredAspectRatio();
}

void UDSCaptureComponent::ConvertNDC2ImageCoord(const FVector2D& InMin, const FVector2D& InMax, FIntVector2& OutMin, FIntVector2& OutMax)
{
	FVector2D Min;
	FVector2D Max;
	Min = (InMin / 2) + FVector2D(0.5f, 0.5f);
	Max = (InMax / 2) + FVector2D(0.5f, 0.5f);

	Min.X *= TextureTarget->SizeX;
	Max.X *= TextureTarget->SizeX;
	Min.Y *= TextureTarget->SizeY;
	Max.Y *= TextureTarget->SizeY;
	Min.Y = TextureTarget->SizeY - Min.Y;
	Max.Y = TextureTarget->SizeY - Max.Y;

	float SwapTemp = Min.Y;
	Min.Y = Max.Y;
	Max.Y = SwapTemp;

	OutMin.X = FMath::FloorToInt(Min.X);
	OutMin.Y = FMath::FloorToInt(Min.Y);
	OutMax.X = FMath::CeilToInt(Max.X);
	OutMax.Y = FMath::CeilToInt(Max.Y);
}

bool UDSCaptureComponent::CheckTargetCoordInvalid(const FVector2D& InMin, const FVector2D& InMax)
{
	if (InMin.X >= 1.0f || InMin.Y >= 1.0f || InMax.X <= -1.0f || InMax.Y <= -1.0f)
	{
		return false;
	}

	float OriginSize = (InMax - InMin).X * (InMax - InMin).Y;

	FVector2D Min = FVector2D(FMath::Clamp(InMin.X, -1, 1), FMath::Clamp(InMin.Y, -1, 1));
	FVector2D Max = FVector2D(FMath::Clamp(InMax.X, -1, 1), FMath::Clamp(InMax.Y, -1, 1));

	float ClippedSize = (Max - Min).X * (Max - Min).Y;

	if (ClippedSize < OriginSize * TargetFilterRate)
	{
		return false;
	}
	return true;
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
	//FVector Direction = GetAttachParent()->GetComponentTransform().InverseTransformPosition(LookAtPos);
	FVector Direction = LookAtPos - GetComponentLocation();
	if (Direction.IsZero())
	{
		Direction = FVector::ForwardVector;
	}
	Direction.Normalize();
	FVector LocalDirection = (Direction);

	// This code does same thing below... but I didn't know that and did it myself...
	//FRotator LookRotator = Direction.Rotation();

	//FVector XYDirection = FVector(LocalDirection.X, LocalDirection.Y, 0.f);

	//// value of pitch angle cosine
	//float XYLength = XYDirection.Length();
	//XYDirection.Normalize();

	//float PitchRad = FMath::Atan2(LocalDirection.Z, XYLength);
	//float YawRad = FMath::Atan2(XYDirection.Y, XYDirection.X);

	//float Pitch = FMath::RadiansToDegrees(PitchRad);
	//float Yaw = FMath::RadiansToDegrees(YawRad);

	//FRotator LookAtRotator = FLookFromMatrix(this->GetActorLocation(), Direction, FVector::UpVector).Rotator();
	//FollowCamera->SetWorldRotation(FRotator(Pitch, Yaw, 0.0f));
	//FollowCamera->SetWorldRotation(LookRotator);
	SetRelativeRotation(GetAttachParent()->GetComponentTransform().InverseTransformRotation((LocalDirection.Rotation() + FRotator(AdditionalAngle, 0.0f, 0.0f)).Quaternion()));
}

void UDSCaptureComponent::SetFinalFOV()
{
	FOVAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(FMath::DegreesToRadians(CurrentFOV) / ZoomRate))), 0.001f, 360.0f);
}

