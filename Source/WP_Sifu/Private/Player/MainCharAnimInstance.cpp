// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharAnimInstance.h"

#include "CameraFocusComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "PlayerAttackComponent.h"
#include "PlayerCombatInteractionComponent.h"
#include "WP_Sifu.h"

void UMainCharAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		Movement = Character->GetCharacterMovement();
	}
}

void UMainCharAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!IsValid(Character)) return;

	// Set values
	Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();

	TurnAngle = UKismetAnimationLibrary::CalculateDirection(Velocity, GetFacingDirection().Rotation());

	constexpr float MoveThreshold = 0.01;
	bShouldMove = (GroundSpeed > MoveThreshold) &&
		(Movement->GetCurrentAcceleration().Size() > MoveThreshold);

	bIsFalling = Movement->IsFalling();

	// Combat state
	if (auto* AttackComp = Character->FindComponentByClass<UPlayerAttackComponent>())
	{
		CurrentCombatStateTag = AttackComp->GetCurrentStateTag();
		bIsAttacking = AttackComp->IsAttacking();
	}

	// Defence state
	if (auto* CIC = Character->FindComponentByClass<UPlayerCombatInteractionComponent>())
	{
		DefenceState = CIC->GetDefenceState();
		HitReactionType = CIC->HitReactionType;
		HitDirection = CIC->HitDirection;
	}

	LOGW(TEXT("%s"), *UEnum::GetValueAsString(DefenceState));
}

FVector UMainCharAnimInstance::GetFacingDirection() const
{
	if (!IsValid(Character)) return FVector::ZeroVector;

	if (const auto CameraFocusComp = Cast<UCameraFocusComponent>(
		Character->GetComponentByClass(UCameraFocusComponent::StaticClass())))
	{
		return CameraFocusComp->GetFacingDirection();
	}

	if (const auto PC = Character->GetController())
	{
		const FRotator Rot = PC->GetControlRotation();
		const FRotator RotXY(0., Rot.Yaw, 0.);
		return FRotationMatrix(RotXY).GetUnitAxis(EAxis::X);
	}

	return Character->GetActorForwardVector();
}
