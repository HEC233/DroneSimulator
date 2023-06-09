// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "DSDronePawn.generated.h"

UENUM(BlueprintType)
enum class EDroneMode : uint8
{
	Setting,
	AutoPilot,
	Manual,
	Waypoint
};

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
	void ChangeDroneMode(EDroneMode InDroneMode);
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void GotoCurrentTarget();

protected:
	void ProcessMouseInput(const FInputActionValue& Value);
	void StartCapture();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDSCaptureComponent> CaptureComponent;

	// Input
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAroundAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> StartCaptureAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneMoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneAltitudeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneCameraZoomAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneSpeedChangeAction;

	// Properties
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	FVector CenterPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	float RotationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	float DroneSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	TArray<float> DroneSpeedArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	int32 DroneSpeedIndex;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	int MaxCaptureCount = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	float CaptureSpeedPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	float BoxSizeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	bool bIsCapture = false;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capture, Meta = (AllowPrivateAccess = "true"))
	// bool bDroneManualMove = false;

	bool bDroneOperation = false;

	// 0 ~ 2pi
	float CurrentRotationRate;
	float AngleSpeed;
	int32 CurrentCaptureCount;
	float TimeRecord;
	float CaptureSpan;
	float CaptureTimeDuration;

	// Waypoint
	int32 CurrentPointIndex = 0;

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AWaypointActor> WpActor;

public:
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void MakeWaypointActorValid();

	// SaveGame
private:
	UFUNCTION()
	class UDSSaveGame* LoadGame();

	UFUNCTION(BlueprintCallable, Category="Data")
	void ApplyLoadData();


	// movement functions
private:
	void MoveDrone(float DeltaTime);
	void MoveDroneWithWaypoint(float DeltaTime);
	void MoveDroneWithInput(const FInputActionValue& Value);
	void DroneAltitudeInput(const FInputActionValue& Value);
	void CameraZoomInput(const FInputActionValue& Value);
	void DroneSpeedChange(const FInputActionValue& Value);

	void UpdateDroneSpeed();

	EDroneMode DroneMode;

	// AirSim
private:
	UPROPERTY()
	class ASimModeBase* simmode_;
};
