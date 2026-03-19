// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatComponentBase.h"
#include "PlayerCombatComponent.generated.h"


UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerCombatComponent : public UCombatComponentBase
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

protected:
	// Called when the component is initialized
	virtual void InitializeComponent() override;

	// Core logic implementation
	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) override;

private:
	// Check hit direction based on ImpactLocation and player's forward vector
	bool IsAttackFromBehind(const FVector& ImpactLocation) const;

	// Dodging/Parrying 시 Structure 데미지에 곱해지는 비율 (임시)
	static constexpr float DeflectStructureRate = 0.25f;

	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;
};
