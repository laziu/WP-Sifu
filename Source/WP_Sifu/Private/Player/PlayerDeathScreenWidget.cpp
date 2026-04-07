// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerDeathScreenWidget.h"

#include "PlayerDeathHandlerComponent.h"


void UPlayerDeathScreenWidget::Show(UPlayerDeathHandlerComponent* InOwnerComponent, int32 InDeathCount)
{
	OwnerDeathHandler = InOwnerComponent;
	DeathCount = InDeathCount;

	AddToViewport(100);
	PlayDeathSequence();
}

void UPlayerDeathScreenWidget::FinishDeathSequence()
{
	RemoveFromParent();

	if (OwnerDeathHandler)
	{
		OwnerDeathHandler->OnAgingComplete();
	}
}
