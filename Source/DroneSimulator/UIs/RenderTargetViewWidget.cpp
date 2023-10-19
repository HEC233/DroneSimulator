// Fill out your copyright notice in the Description page of Project Settings.


#include "UIs/RenderTargetViewWidget.h"
#include "Pawns/DSDronePawn.h"
#include "Components/DSCaptureComponent.h"
#include "Components/CanvasPanelSlot.h" 
#include "Components/PanelWidget.h"

URenderTargetViewWidget::URenderTargetViewWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URenderTargetViewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PanelWidget = Cast<UPanelWidget>(GetWidgetFromName(TEXT("Canvas")));
	ensure(PanelWidget);
}

void URenderTargetViewWidget::Update()
{
	check(BboxWidgetType != nullptr);

	ADSDronePawn* Pawn = Cast<ADSDronePawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (!Pawn)
	{
		return;
	}
	UActorComponent* Comp = Pawn->GetComponentByClass(UDSCaptureComponent::StaticClass());
	if (!Comp)
	{
		return;
	}
	UDSCaptureComponent* CaptureComp = Cast<UDSCaptureComponent>(Comp);
	if (!CaptureComp)
	{
		return;
	}

	const TArray<FVector4> Coordinates = CaptureComp->GetTargetCoordinated();

	int32 Index;
	UCanvasPanelSlot* CanvasSlot;
	for (Index = 0; Index < Coordinates.Num(); ++Index)
	{
		if (!BboxWidgets.IsValidIndex(Index))
		{
			BboxWidgets.Emplace(CreateWidget<UUserWidget>(GetWorld(), BboxWidgetType));
			PanelWidget->AddChild(BboxWidgets[Index]);
			CanvasSlot = Cast<UCanvasPanelSlot>(BboxWidgets[Index]->Slot);
			CanvasSlot->SetAnchors(FAnchors(0.5, 0.5));
			CanvasSlot->SetAlignment(FVector2D(0.5, 0.5));
		}

		CanvasSlot = Cast<UCanvasPanelSlot>(BboxWidgets[Index]->Slot);
		UCanvasPanelSlot* CanvasCanvas = Cast<UCanvasPanelSlot>(PanelWidget->Slot);
		FVector2D Size = CanvasCanvas->GetSize() * 0.5;
		CanvasSlot->SetSize(FVector2D((Coordinates[Index].Z - Coordinates[Index].X) * Size.X, (Coordinates[Index].W - Coordinates[Index].Y) * Size.Y));
		CanvasSlot->SetPosition(FVector2D((Coordinates[Index].X + Coordinates[Index].Z) * 0.5 * Size.X, -(Coordinates[Index].Y + Coordinates[Index].W) * 0.5 * Size.Y));
	}

	while(Index < BboxWidgets.Num())
	{
		UUserWidget* Widget = BboxWidgets[Index];
		PanelWidget->RemoveChild(Widget);
		BboxWidgets.RemoveAt(Index);
	}
}
