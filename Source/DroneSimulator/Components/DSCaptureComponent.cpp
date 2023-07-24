// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "DroneSimulator/Controller/DSPlayerController.h"
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

void UDSCaptureComponent::TakeScreenShot(int32 CaptureIndex)
{
	if (TextureTarget == nullptr || CaptureTargetActor == nullptr)
	{
		return;
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

	FString ImageFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s Image - %s.jpg"), CaptureIndex, *TargetName, *TimeString));
	FString TextFilePath = FPaths::Combine(FPlatformMisc::ProjectDir(), *FString::Printf(TEXT("Captures\\%06d - %s Image - %s.txt"), CaptureIndex, *TargetName, *TimeString));
	FPaths::MakeStandardFilename(ImageFilePath);
	FPaths::MakeStandardFilename(TextFilePath);


	FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*ImageFilePath);
	if (RawFileWriterAr == nullptr)
	{
		//UE_LOG(LogTemp, Log, TEXT("Problem Occured"));
		return;
	}
	bool ImageSavedOK = ExportRenderTargetJPG(TextureTarget, *RawFileWriterAr);
	RawFileWriterAr->Close();
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

		if (Min.X >= 1.0f || Min.Y >= 1.0f || Max.X <= -1.0f || Max.Y <= -1.0f)
		{
			//UE_LOG(LogTemp, Log, TEXT("Target was filtered because out of image Min : %s, Max : %s"), *Min.ToString(), *Max.ToString());
			continue;
		}

		float OriginSize = (Max - Min).X * (Max - Min).Y;

		Min = FVector2D(FMath::Clamp(Min.X, -1, 1), FMath::Clamp(Min.Y, -1, 1));
		Max = FVector2D(FMath::Clamp(Max.X, -1, 1), FMath::Clamp(Max.Y, -1, 1));

		float ClippedSize = (Max - Min).X * (Max - Min).Y;

		if (ClippedSize < OriginSize * TargetFilterRate)
		{
			//UE_LOG(LogTemp, Log, TEXT("Target was filtered because lack of size OriginSize : %f, ClippedSize : %f"), OriginSize, ClippedSize);
			continue;
		}

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

		int32 MinX = FMath::FloorToInt(Min.X);
		int32 MinY = FMath::FloorToInt(Min.Y);
		int32 MaxX = FMath::CeilToInt(Max.X);
		int32 MaxY = FMath::CeilToInt(Max.Y);

		NameProp = Target->GetClass()->FindPropertyByName(TEXT("TargetAbsoluteName"));
		if (NameProp)
		{
			NameProp->GetValue_InContainer(Target, &TargetAbsoulteName);
		}

		//UE_LOG(LogTemp, Log, TEXT("Target was successfully captured"));
		LabelingText += FString::Printf(TEXT("%s,%d,%d,%d,%d\n"), *TargetAbsoulteName, MinX, MinY, MaxX - MinX, MaxY - MinY);
	}
	FFileHelper::SaveStringToFile(*LabelingText, *TextFilePath, FFileHelper::EEncodingOptions::ForceUTF8);

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

void UDSCaptureComponent::SetTargetFilterRate(float InRate)
{
	TargetFilterRate = FMath::Clamp(InRate, 0.0f, 1.0f);
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
	FVector Direction = GetAttachParent()->GetComponentTransform().InverseTransformPosition(LookAtPos);
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
	SetRelativeRotation(LocalDirection.Rotation());
}

void UDSCaptureComponent::SetFinalFOV()
{
	FOVAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(FMath::DegreesToRadians(CurrentFOV) / ZoomRate))), 0.001f, 360.0f);
}

