// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "DSDronePawn.generated.h"

UCLASS()
class DRONESIMULATOR_API ADSDronePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADSDronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// About Pawn Camera
public:
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void ChangeCaptureState(bool bBoolean);
	UFUNCTION(BlueprintCallable, Category = "Capture")
	FString GetCaptureInfo();
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void ChangeDroneMode(bool bBoolean);

protected:
	void ProcessMouseInput(const FInputActionValue& Value);
	void TakeScreenShot();
	void ChangeTarget();
	void StartCapture();
	void CalculateNDCMinMax(FVector2f& OutMin, FVector2f& OutMax);

	// Mesh
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	// Camera
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	// Input
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TakeScreenShotAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeTargetAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAroundAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> StartCaptureAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneMoveAction;

	// Properties
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	FVector CenterPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	float RotationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	float DroneSpeed;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	int MaxCaptureCount = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	float CaptureSpeedPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	float BoxSizeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	bool bIsCapture = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	bool bDroneManualMove = false;

	bool bDroneMode = false;

	// 0 ~ 2pi
	float CurrentRotationRate;
	float AngleSpeed;
	int32 CurrentCaptureCount;
	float TimeRecord;
	float CaptureSpan;
	float CaptureTimeDuration;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CaptureTargetActor;

	// SaveGame
private:
	UFUNCTION()
	class UDSSaveGame* LoadGame();

	UFUNCTION()
	void ApplyLoadData();

	
	// movment functions
private:
	void MoveDrone(float DeltaTime);
	void MoveDroneWithInput(const FInputActionValue& Value);
	void LookTarget(AActor* Target);
	void UpdateDroneSpeed();

	// AirSim
private:
	UPROPERTY()
	class ASimModeBase* simmode_;
};
