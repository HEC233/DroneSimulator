// Fill out your copyright notice in the Description page of Project Settings.

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "DSPlayerController.h"

ADSPlayerController::ADSPlayerController()
{
	UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	GameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
}

void ADSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
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
