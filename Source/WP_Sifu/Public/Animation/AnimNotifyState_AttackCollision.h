// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_AttackCollision.generated.h"


/**
 * Placed on a montage to enable or disable collision detection during the
 * attack window.
 * Uses AttackTag to find the matching CollisionComponent via
 * UAttackCollisionManagerComponent.
 */
UCLASS(meta=(DisplayName="Attack Collision"))
class WP_SIFU_API UAnimNotifyState_AttackCollision : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/// Attack source tag activated by this notify (Attack.Source.Hand.R, Attack.Source.Default, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attack)
	FGameplayTag AttackTag;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                         float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                       const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
