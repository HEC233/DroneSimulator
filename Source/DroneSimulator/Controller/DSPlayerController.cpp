// Fill out your copyright notice in the Description page of Project Settings.

#include "DSPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Actors/DSTarget.h"
#include "Kismet/GameplayStatics.h"
#include "UIs/MinimapWidgetBase.h"
#include "UIs/DSWeaponToggleWidget.h"

ADSPlayerController::ADSPlayerController()
{
	UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	GameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);

	ConstructorHelpers::FClassFinder<UDSWeaponToggleWidget> BPWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_WeaponVisibleToggle.WBP_WeaponVisibleToggle_C'"));
	if (BPWidget.Class)
	{
		ToggleWidget = BPWidget.Class;
	}
}

void ADSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);

	for (TObjectIterator<ADSTarget> It; It; ++It)
	{
		AllTargets.Add(*It);
		if (TargetsByClass.Contains(It->GetClass()))
		{
			TargetsByClass[It->GetClass()].Targets.Add(*It);
		}
		else
		{
			TargetsByClass.Add(It->GetClass(), FArrayPacker(*It));
			TargetVisibility.Add(It->GetClass(), true);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Target Class Num : %d"), TargetsByClass.Num());
}

void ADSPlayerController::ChangeUI(UIStatus UI)
{
	if (UI != UIStatus::Pause && bPaused)
	{
		SetPauseUI(false);
	}

	if (UI != UIStatus::Pause && CurrentWidgets.Num() > 0)
	{
		for (auto Widget : CurrentWidgets)
		{
			if (Widget)
			{
				Widget->RemoveFromParent();
			}
		}
		CurrentWidgets.Empty();
	}

	SetShowMouseCursor(true);
	switch (UI)
	{
	case UIStatus::Title:
		CurrentWidgets.Add(CreateWidget<UUserWidget>(GetWorld(), TitleWidget));
		SetInputMode(GameAndUI);
		break;
	case UIStatus::Setting:
		CurrentWidgets.Add(CreateWidget<UUserWidget>(GetWorld(), SettingWidget));
		{
			UUserWidget* CreatedMinimapWidget = CreateWidget<UUserWidget>(GetWorld(), MinimapWidget);
			UMinimapWidgetBase* MinimapWidgetBase = Cast<UMinimapWidgetBase>(CreatedMinimapWidget);
			if (MinimapWidgetBase)
			{
				MinimapWidgetBase->SetMinimapInfo(MinimapImage, MinimapCaptureXCoord, MinimapCaptureYCoord, MinimapCaptureScale);
			}
			CurrentWidgets.Add(CreatedMinimapWidget);
		}
		SetInputMode(GameAndUI);
		break;
	case UIStatus::MapSelect:
		CurrentWidgets.Add(CreateWidget<UUserWidget>(GetWorld(), MapSelectWidget));
		SetInputMode(GameAndUI);
		break;
	case UIStatus::CaptureMode:
		CurrentWidgets.Add(CreateWidget<UUserWidget>(GetWorld(), CaptureModeWidget));
		CurrentWidgets.Add(CreateWidget<UUserWidget>(GetWorld(), RenderTargetWidget));
		SetShowMouseCursor(false);
		SetInputMode(GameOnly);
		break;
	case UIStatus::Pause:
		SetPauseUI(!bPaused);
		break;
	default:
		break;
	}

	if (UI != UIStatus::Pause && CurrentWidgets.Num() > 0)
	{
		for (auto Widget : CurrentWidgets)
		{
			if (Widget)
			{
				Widget->AddToViewport();
			}
		}
	}

	CurrentUI = UI;
}

TArray<UUserWidget*> ADSPlayerController::CreateAndGetVisibiltyToggleWidget()
{
	TArray<UUserWidget*> Ret;

	for (auto& Target : TargetVisibility)
	{
		UDSWeaponToggleWidget* Widget = Cast<UDSWeaponToggleWidget>(CreateWidget(this, ToggleWidget, *FString::Printf(TEXT("%sToggle"), *Target.Key->GetDefaultObject()->GetFName().ToString())));

		Widget->SetToggleState(Target.Value);
		Widget->SetClass(Target.Key, Cast<ADSTarget>(TargetsByClass[Target.Key].Targets[0])->GetTargetAbsoluteName());
		Widget->OnToggleChanged.BindUObject(this, &ADSPlayerController::SetTargetVisibility);

		Ret.Add(Widget);
	}

	return Ret;
}

TArray<AActor*> ADSPlayerController::GetSortedTargetActors(AActor* PresetActor)
{
	PresetActor->GetAttachedActors(TargetArray);
	for (int32 Index = 0; Index < TargetArray.Num(); ++Index)
	{
		FProperty* NameProp = TargetArray[Index]->GetClass()->FindPropertyByName(TEXT("TargetName"));
		if (NameProp == nullptr)
		{
			TargetArray.RemoveAt(Index);
			--Index;
		}
	}
	TargetArray.Sort([](AActor& A, AActor& B) {
		FProperty* NameProp = A.GetClass()->FindPropertyByName(TEXT("TargetName"));
		FString AName;
		FString BName;
		NameProp->GetValue_InContainer(&A, &AName);
		NameProp->GetValue_InContainer(&B, &BName);

		return AName < BName;
	});

	return TargetArray;
}

void ADSPlayerController::SetPauseUI(bool bPause)
{
	SetPause(bPause);

	if (bPause)
	{
		CurrentPauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseWidget);
		CurrentPauseWidget->AddToViewport();
		SetShowMouseCursor(true);
		SetInputMode(GameAndUI);

		bPaused = true;
	}
	else
	{
		CurrentPauseWidget->RemoveFromParent();
		CurrentPauseWidget = nullptr;

		bPaused = false;
	}
}

TArray<const AActor*> ADSPlayerController::GetTargetsInVolume(const FConvexVolume& ConvexVolume)
{
	TArray<const AActor*> Result;

	for (const AActor* Target : AllTargets)
	{
		FVector Origin;
		FVector Extent;
		Target->GetActorBounds(true, Origin, Extent, true);

		if (ConvexVolume.IntersectBox(Origin, Extent))
		{
			Result.Add(Target);
		}
	}

	return Result;
}

bool ADSPlayerController::SetTargetVisibility(const TSubclassOf<AActor>& TargetClass, bool Visiblity)
{
	if (!TargetVisibility.Contains(TargetClass) || !TargetsByClass.Contains(TargetClass))
	{
		return false;
	}

	TargetVisibility[TargetClass] = Visiblity;

	for (AActor* Target : TargetsByClass[TargetClass].Targets)
	{
		Target->SetActorHiddenInGame(!Visiblity);
	}

	return true;
}
