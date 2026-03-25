// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ComboWindow.h"

#include "PlayerComboComponent.h"


void UAnimNotifyState_ComboWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* ComboComp = Owner->FindComponentByClass<UPlayerComboComponent>())
		{
			ComboComp->OpenTransitionWindow(TransitionId);
		}
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* ComboComp = Owner->FindComponentByClass<UPlayerComboComponent>())
		{
			ComboComp->CloseTransitionWindow(TransitionId);
		}
	}
}

FString UAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("ComboWindow: %s"), *TransitionId.ToString());
}
