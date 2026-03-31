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

public: // --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Move)
	TObjectPtr<class UPlayerMoveComponent> PlayerMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class UThirdPersonCameraComponent> ThirdPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class UCameraFocusComponent> CameraFocus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Abilities)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UPlayerCombatInteractionComponent> PlayerCombatInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UPlayerAttackComponent> PlayerAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UAttackCollisionManagerComponent> AttackCollisionManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TArray<TObjectPtr<class UAttackCollisionComponent>> AttackCollisions;

	UPROPERTY()
	TObjectPtr<class UHealthAttributeSet> HealthAttributes;

public: // --- Configs ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Abilities)
	float MaxHealth = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Abilities)
	float MaxStructure = 100.f;
};
