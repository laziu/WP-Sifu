// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyDeathHandlerComponent.h"

#include "UserExtension.h"
#include "Components/CapsuleComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"


UEnemyDeathHandlerComponent::UEnemyDeathHandlerComponent()
{
	Ext::SetObject(DeathMontage,
		TEXT("/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Enemy/SK_Servant_M_Hideout00_CasualDisciple_01/SkeletalMeshes/AM_Death.AM_Death'"));
}


void UEnemyDeathHandlerComponent::OnDeathBegin()
{
	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		// Stop AI behavior immediately
		if (auto* AIC = Cast<AAIController>(Character->GetController()))
		{
			if (auto* STC = AIC->FindComponentByClass<UStateTreeComponent>())
			{
				STC->StopLogic(TEXT("Dead"));
			}
		}

		// Stop all movement
		Character->GetCharacterMovement()->DisableMovement();
	}
}


void UEnemyDeathHandlerComponent::OnDeathComplete()
{
	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		// Disable collision so the corpse doesn't block anything
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnDeathFinished.Broadcast();
}
