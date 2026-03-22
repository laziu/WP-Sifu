// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

	// Movement velocity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	FVector Velocity;

	// Movement speed in XY plane
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	float GroundSpeed;

	// Degree between the velocity and the actor rotation;
	// clamped between ±45˚ to prevent backward animation when turning around
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	float TurnAngle;

	// if ground speed is above a small threshold
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bShouldMove;

	// Movement component falling state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	bool bIsFalling;
};
