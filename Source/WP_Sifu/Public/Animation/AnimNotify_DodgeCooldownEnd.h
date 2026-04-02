// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DodgeCooldownEnd.generated.h"

UCLASS(meta=(DisplayName="Dodge Cooldown End"))
class WP_SIFU_API UAnimNotify_DodgeCooldownEnd : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("DodgeCooldownEnd"); }
};
