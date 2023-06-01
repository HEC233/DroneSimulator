// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DSCaptureComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONESIMULATOR_API UDSCaptureComponent : public UActorComponent
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CaptureTargetActor;

public:
	void TakeScreenShot(int32 CaptureIndex = 0);
	void SetTarget(AActor* TargetActor);
	const TObjectPtr<AActor> GetTarget();
	void SetCameraPosition(FVector RelativePosition);
	void AttachCamera(USceneComponent* Parent);

	void SetCameraFOV(float FOV);

private:
	void CalculateNDCMinMax(FVector2f& OutMin, FVector2f& OutMax);
	bool ExportRenderTargetJPG(class UTextureRenderTarget2D* TexRT, FArchive& Ar);
	void LookTarget();

private:
	FName TargetFilteringName;

};
