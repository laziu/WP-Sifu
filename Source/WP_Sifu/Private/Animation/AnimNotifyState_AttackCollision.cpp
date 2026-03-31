// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_AttackCollision.h"

#include "AttackCollisionComponent.h"
#include "AttackCollisionManagerComponent.h"


void UAnimNotifyState_AttackCollision::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* Mgr = Owner->FindComponentByClass<UAttackCollisionManagerComponent>())
		{
			if (auto* Collision = Mgr->FindAttackCollision(AttackTag))
			{
				Collision->ActivateTrace();
			}
		}
	}
}

void UAnimNotifyState_AttackCollision::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* Mgr = Owner->FindComponentByClass<UAttackCollisionManagerComponent>())
		{
			if (auto* Collision = Mgr->FindAttackCollision(AttackTag))
			{
				Collision->DeactivateTrace();
			}
		}
	}
}

FString UAnimNotifyState_AttackCollision::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("AttackCollision: %s"), *AttackTag.ToString());
}
