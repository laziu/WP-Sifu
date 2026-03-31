// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "Attackable.h"
#include "DummyEnemy.generated.h"


/**
 * Stationary target that implements IAttackable.
 * Has Health/Structure attributes identical to the Player, but performs no defence.
 */
UCLASS()
class WP_SIFU_API ADummyEnemy :
	public AActor,
	public IAbilitySystemInterface,
	public IAttackable
{
	GENERATED_BODY()

public:
	ADummyEnemy();

	// --- IAbilitySystemInterface ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// --- IAttackable ---
	virtual UCombatInteractionComponentBase* GetCombatInteractionComponent() const override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Collision)
	TObjectPtr<class UCapsuleComponent> CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Visual)
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Abilities)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UDummyCombatInteractionComponent> CombatInteractionComp;

	UPROPERTY()
	TObjectPtr<class UHealthAttributeSet> HealthAttribs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	float MaxStructure = 100.f;
};
