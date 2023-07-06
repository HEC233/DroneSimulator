// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapWidgetBase.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanelSlot.h" 
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DroneSimulator/DSSaveGame.h"

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
		const UDSSaveGame* Save = Cast<UDSSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSetting", 0));
		int32 Idx = 0;
		for (int32 i = 0; i < Save->CurrentWayPoint.Points.Num(); ++i, ++Idx)
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
				FProperty* WaypointIndexProp = WaypointWidgets[i]->GetClass()->FindPropertyByName(WaypointIndexName);
				if (WaypointIndexProp)
				{
					WaypointIndexProp->SetValue_InContainer(WaypointWidgets[i], &i);
				}
			}

			CanvasSlot = Cast<UCanvasPanelSlot>(WaypointWidgets[i]->Slot);
			CanvasSlot->SetPosition(WorldPos2MinimapCoord(Save->CurrentWayPoint.Points[i]));
		}
		for (int32 i = Idx; i < WaypointWidgets.Num(); ++i)
		{
			WaypointWidgets[i]->RemoveFromParent();
		}
		WaypointWidgets.SetNum(Save->CurrentWayPoint.Points.Num());
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
