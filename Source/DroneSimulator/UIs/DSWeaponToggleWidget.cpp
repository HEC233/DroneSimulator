// Fill out your copyright notice in the Description page of Project Settings.


#include "UIs/DSWeaponToggleWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Actors/DSTarget.h"

void UDSWeaponToggleWidget::SetClass(TSubclassOf<AActor> InClass, const FString& TargetName)
{
	TargetClass = InClass;

	UTextBlock* TextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameText")));

	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(TargetName));
	}
}

void UDSWeaponToggleWidget::SetToggleState(bool Value)
{
	UCheckBox* CheckBox = Cast<UCheckBox>(GetWidgetFromName(TEXT("CheckBox")));
	if (CheckBox)
	{
		CheckBox->SetIsChecked(Value);
	}
}
