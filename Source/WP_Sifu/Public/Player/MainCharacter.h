// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputBindableCharacter.h"
#include "AbilitySystemInterface.h"
#include "Attackable.h"
#include "MainCharacter.generated.h"

UCLASS()
class WP_SIFU_API AMainCharacter :
	public AInputBindableCharacter,
	public IAbilitySystemInterface,
	public IAttackable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// IAttackable
	virtual UCombatInteractionComponentBase* GetCombatInteractionComponent() const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UPlayerMoveComponent> PlayerMoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class UThirdPersonCameraComponent> ThirdPersonCameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class UCameraFocusComponent> LockOnComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Abilities)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UPlayerCombatInteractionComponent> CombatInteractionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UPlayerAttackComponent> AttackComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UAttackCollisionManagerComponent> AttackCollisionManagerComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TArray<TObjectPtr<class UAttackCollisionComponent>> LimbCollisions;

	UPROPERTY()
	TObjectPtr<class UHealthAttributeSet> HealthAttribs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Abilities)
	float MaxHealth = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Abilities)
	float MaxStructure = 100.f;
};
