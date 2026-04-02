// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_DodgeActive.h"

#include "PlayerCombatInteractionComponent.h"


void UAnimNotifyState_DodgeActive::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* CIC = Owner->FindComponentByClass<UPlayerCombatInteractionComponent>())
		{
			CIC->OnDodgeActiveBegin();
		}
	}
}

void UAnimNotifyState_DodgeActive::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* CIC = Owner->FindComponentByClass<UPlayerCombatInteractionComponent>())
		{
			CIC->OnDodgeActiveEnd();
		}
	}
}
