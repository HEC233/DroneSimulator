// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DSCaptureComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONESIMULATOR_API UDSCaptureComponent : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDSCaptureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTextureRenderTarget2D> MyRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const AActor> CaptureTargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	FVector LookAtPos;

public:
	bool TakeScreenShot(int32 CaptureIndex = 0);
	void SetTarget(const AActor* TargetActor);
	const TObjectPtr<const AActor> GetTarget();
	void SetLookAtPos(const FVector& Pos);
	const FVector& GetLookAtPos();

	void SetCameraFOV(float FOV);
	void SetZoomRate(float InZoomRate);
	void SetTargetFilterRate(float InRate);
	void SetAdditionalAngle(float InAngle);
	const float GetAdditionalAngle();

	const TArray<FVector4>& GetTargetCoordinated();

protected:
	FMatrix GetViewProjection();

private:
	void CalculateNDCMinMax(const AActor* Target, FVector2D& OutMin, FVector2D& OutMax, bool bFilterOutScreen);
	void ConvertNDC2ImageCoord(const FVector2D& InMin, const FVector2D& InMax, FIntVector2& OutMin, FIntVector2& OutMax);
	bool CheckTargetCoordInvalid(const FVector2D& InMin, const FVector2D& InMax);
	bool ExportRenderTargetJPG(class UTextureRenderTarget2D* TexRT, FArchive& Ar);
	void LookTarget();

	FORCEINLINE void SetFinalFOV();

private:
	FName TargetFilteringName;

	float CurrentFOV = 60.0f;
	float ZoomRate = 1.0f;
	float TargetFilterRate = 0.5f;
	float AdditionalAngle = 0.0f;

	TArray<FVector4> TargetCoordinatesBuffer;
};
