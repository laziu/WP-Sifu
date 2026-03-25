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

	// Block 키 입력 처리
	void StartBlock();
	void StopBlock();

protected:
	// Called when the component is initialized
	virtual void InitializeComponent() override;

	// Core logic implementation
	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) override;

private:
	// Check hit direction based on ImpactLocation and player's forward vector
	bool IsAttackFromBehind(const FVector& ImpactLocation) const;

	// Dodging/Parrying 시 Structure 데미지에 곱해지는 비율 (임시)
	static constexpr float DeflectStructureRate = -2.f;

	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;

	// --- Parry/Block ---

	FTimerHandle ParryTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float ParryWindowDuration = 0.15f;

	bool bBlockKeyHeld = false;

	void OnParryWindowExpired();
};
