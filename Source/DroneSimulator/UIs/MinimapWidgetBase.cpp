// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapWidgetBase.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanelSlot.h" 
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DroneSimulator/Actors/WaypointActor.h"
#include "DroneSimulator/Save/WaypointSaveGame.h"

void UMinimapWidgetBase::SetMinimapInfo(UTexture2D* Image, float ImageXCoord, float ImageYCoord, float ImageScale)
{
	MinimapName = FName(TEXT("MinimapImage"));
	WaypointIndexName = FName(TEXT("WaypointIndex"));
	FProperty* MinimapProp = GetClass()->FindPropertyByName(MinimapName);
	if (MinimapProp)
	{
		UImage* MinimapImage;
		MinimapProp->GetValue_InContainer(this, &MinimapImage);
		MinimapImage->SetBrushFromTexture(Image);
	}
	MinimapCaptureLocation = FVector2D(ImageXCoord, ImageYCoord);
	MinimapCaptureScale = ImageScale;

	FProperty* MinimapPanelProp = GetClass()->FindPropertyByName(TEXT("MinimapCanvasPanel"));
	if (MinimapPanelProp)
	{
		MinimapPanelProp->GetValue_InContainer(this, &MinimapPanel);
	}
	check(MinimapPanel != nullptr);
}

FVector2D UMinimapWidgetBase::WorldPos2MinimapCoord(const FVector& WorldPos)
{
	float XPos = WorldPos.X - MinimapCaptureLocation.X;
	float YPos = WorldPos.Y - MinimapCaptureLocation.Y;

	FVector2D UnrotatedCoord = (FVector2D(YPos, -XPos) / MinimapCaptureScale * 2048) - MinimapCoord;
	UnrotatedCoord *= GetZoomRate();

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(MinimapRotate));
	FVector2D RotatedCoord = FVector2D(Cos * UnrotatedCoord.X - Sin * UnrotatedCoord.Y, Sin * UnrotatedCoord.X + Cos * UnrotatedCoord.Y);

	return RotatedCoord;
}

void UMinimapWidgetBase::SetPlayerActor(AActor* InPlayerActor)
{
	PlayerActor = InPlayerActor;
	PlayerSpringArm = Cast<USpringArmComponent>(PlayerActor->GetComponentByClass(USpringArmComponent::StaticClass()));
}

void UMinimapWidgetBase::UpdateMinimap()
{
	if (nullptr == WpActor)
	{
		if (SetWaypointActor() == false)
		{
			return;
		}
	}
	if (PlayerSpringArm)
	{
		MinimapRotate = -PlayerSpringArm->GetComponentRotation().Yaw;
	}
	if (PlayerActor)
	{
		float XPos = PlayerActor->GetActorLocation().X - MinimapCaptureLocation.X;
		float YPos = PlayerActor->GetActorLocation().Y - MinimapCaptureLocation.Y;

		MinimapCoord = FVector2d(YPos, -XPos) / MinimapCaptureScale * 2048;
	}
	if (MinimapPanel)
	{
		int32 Idx = 0;
		for (int32 i = 0; i < WpActor->GetWaypoint().Points.Num(); ++i, ++Idx)
		{
			UCanvasPanelSlot* CanvasSlot;
			if (!WaypointWidgets.IsValidIndex(i))
			{
				WaypointWidgets.Emplace(CreateWidget<UUserWidget>(GetWorld(), WaypointWidget));
				MinimapPanel->AddChild(WaypointWidgets[i]);
				CanvasSlot = Cast<UCanvasPanelSlot>(WaypointWidgets[i]->Slot);
				CanvasSlot->SetAnchors(FAnchors(0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5, 0.5));
				CanvasSlot->SetSize(FVector2D(30.0, 30.0));
			}

			CanvasSlot = Cast<UCanvasPanelSlot>(WaypointWidgets[i]->Slot);
			CanvasSlot->SetPosition(WorldPos2MinimapCoord(WpActor->GetWaypoint().Points[i].Location));
		}
		for (int32 i = Idx; i < WaypointWidgets.Num(); ++i)
		{
			WaypointWidgets[i]->RemoveFromParent();
		}
		WaypointWidgets.SetNum(WpActor->GetWaypoint().Points.Num());

		if (WaypointWidgets.Num() > 0)
		{
			FProperty* WaypointIndexProp = WaypointWidgets[0]->GetClass()->FindPropertyByName(WaypointIndexName);
			if (WaypointIndexProp)
			{
				for (int32 idx = 0; idx < WaypointWidgets.Num(); ++idx)
				{
					WaypointIndexProp->SetValue_InContainer(WaypointWidgets[idx], &idx);
				}
			}
		}
	}

	FProperty* MinimapProp = GetClass()->FindPropertyByName(MinimapName);
	if (MinimapProp)
	{
		UImage* MinimapImage;
		MinimapProp->GetValue_InContainer(this, &MinimapImage);

		FVector2d Pivot = MinimapCoord / 2048 + (FVector2d::One() * 0.5f);
		MinimapImage->SetRenderTransformPivot(Pivot);
		MinimapImage->SetRenderTransformAngle(MinimapRotate);
		MinimapImage->SetRenderTranslation(-MinimapCoord);
	}
}

float UMinimapWidgetBase::GetZoomRate_Implementation()
{
	return 1.0f;
}

bool UMinimapWidgetBase::SetWaypointActor()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypointActor::StaticClass(), OutActors);
	if (OutActors.Num() > 0)
	{
		WpActor = Cast<AWaypointActor>(OutActors[0]);
		return true;
	}
	return false;
}
