// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatInteractionComponentBase.h"
#include "DummyCombatInteractionComponent.generated.h"


/**
 * Minimal combat interaction for non-defending targets (e.g. TrainingDummy).
 * Always takes full damage with no defence logic.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UDummyCombatInteractionComponent : public UCombatInteractionComponentBase
{
	GENERATED_BODY()

public:
	UDummyCombatInteractionComponent();

protected:
	virtual void InitializeComponent() override;
	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) override;

private:
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;
};
