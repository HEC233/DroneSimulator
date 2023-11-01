// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DSTarget.generated.h"

UENUM(BlueprintType)
enum class ETargetUIType : uint8
{
	Human = 0,
	OneSlider,
	TwoSlider,
	TwoSliderAndOutrigger,
	None
};

UENUM(BlueprintType)
enum class ESliderAxis : uint8
{
	Roll,
	Pitch,
	Yaw
};

UENUM(BlueprintType)
enum class ETargetUsageType : uint8
{
	None = 0,
	YukHangSa = 1,
	AIPower = 2,
};

UCLASS()
class DRONESIMULATOR_API ADSTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADSTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE FString GetTargetName() { return TargetName; }
	FORCEINLINE FString GetTargetAbsoluteName() { return TargetAbsoluteName; }

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	ETargetUIType UIType;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateState();
	virtual void UpdateState_Implementation();
	FORCEINLINE ETargetUsageType GetUsageType() { return UsageType; }

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	FString TargetName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	FString TargetAbsoluteName;

	UPROPERTY(BlueprintReadWrite, Category = Setting)
	int32 Pose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Setting)
	float Slider1Value;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	float Slider1Max;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	float Slider1Min;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Setting)
	float Slider2Value;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	float Slider2Max;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	float Slider2Min;

	UPROPERTY(BlueprintReadWrite, Category = Setting)
	uint8 bOutriggerSet : 1;

	UPROPERTY()
	TObjectPtr<USceneComponent> Slider1Object;
	UPROPERTY()
	TObjectPtr<USceneComponent> Slider2Object;
	UPROPERTY(EditDefaultsOnly, Category = Setting)
	ESliderAxis Slider1Axis;
	UPROPERTY(EditDefaultsOnly, Category = Setting)
	ESliderAxis Slider2Axis;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	ETargetUsageType UsageType;
};
