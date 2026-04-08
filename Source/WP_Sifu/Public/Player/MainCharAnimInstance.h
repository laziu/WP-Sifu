// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "CombatInteractionComponentBase.h"
#include "PlayerCombatInteractionComponent.h"
#include "MainCharAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class WP_SIFU_API UMainCharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Character)
	TObjectPtr<class ACharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Character)
	TObjectPtr<class UCharacterMovementComponent> Movement;

	/// Movement velocity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	FVector Velocity;

	/// Movement speed in XY plane
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	float GroundSpeed;

	/// Degree between the velocity and the actor rotation;
	/// clamped between ±45˚ to prevent backward animation when turning around
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	float TurnAngle;

	/// if ground speed is above a small threshold
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bShouldMove;

	/// Movement component falling state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bIsFalling;

	/// Whether the character is currently in combat stance (enemies nearby / locked on)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bInCombatStance;

	/// Attack ready state: in combat stance, not attacking, not running, not falling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bIsAttackReady;

protected: // --- Combat ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	FGameplayTag CurrentCombatStateTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	bool bIsAttacking = false;

protected: // --- Defence ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Defence")
	EDefenceState DefenceState = EDefenceState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Defence")
	EHitReactionType HitReactionType = EHitReactionType::None;

	/// Hit direction in character-local space (X=Right, Y=Forward) for blend space
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Defence")
	FVector2D HitDirection = FVector2D::ZeroVector;

protected: // --- Death ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Death")
	bool bIsDead = false;

	UFUNCTION(BlueprintPure, Category="Camera")
	FVector GetFacingDirection() const;
};
