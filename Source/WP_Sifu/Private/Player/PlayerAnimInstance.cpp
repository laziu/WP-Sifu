// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		Movement = Character->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!IsValid(Character)) return;

	// Set values
	Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();

	TurnAngle = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	if (Movement->bOrientRotationToMovement)
	{
		TurnAngle = FMath::Clamp(TurnAngle, -45.f, 45.f);
	}

	constexpr float MoveThreshold = 0.01;
	bShouldMove = (GroundSpeed > MoveThreshold) &&
		(Movement->GetCurrentAcceleration().Size() > MoveThreshold);

	bIsFalling = Movement->IsFalling();
}
