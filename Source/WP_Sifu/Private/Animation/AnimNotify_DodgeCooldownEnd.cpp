// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_DodgeCooldownEnd.h"

#include "PlayerCombatInteractionComponent.h"


void UAnimNotify_DodgeCooldownEnd::Notify(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* CIC = Owner->FindComponentByClass<UPlayerCombatInteractionComponent>())
		{
			CIC->OnDodgeCooldownEnd();
		}
	}
}
