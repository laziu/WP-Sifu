// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMoveComponent.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerMoveComponent::UPlayerMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(InputLook, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
}

void UPlayerMoveComponent::OnRegister()
{
	Super::OnRegister();

	// Character rotation: follow the camera/controller yaw instead of movement direction
	auto Character = CastChecked<ACharacter>(GetOwner());
	Character->bUseControllerRotationPitch = false;
	Character->bUseControllerRotationYaw = true;
	Character->bUseControllerRotationRoll = false;

	auto Movement = Character->GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->RotationRate = FRotator(0., 540., 0.);
	Movement->JumpZVelocity = 600.;
	Movement->AirControl = 0.2;
}

void UPlayerMoveComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputMove, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputMove);
	EIC->BindAction(InputLook, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputLook);
	EIC->BindAction(InputRun, ETriggerEvent::Started, this, &UPlayerMoveComponent::OnInputRunStarted);
	EIC->BindAction(InputRun, ETriggerEvent::Completed, this, &UPlayerMoveComponent::OnInputRunStopped);
	EIC->BindAction(InputRun, ETriggerEvent::Canceled, this, &UPlayerMoveComponent::OnInputRunStopped);
}

void UPlayerMoveComponent::OnInputMove(const FInputActionValue& Value)
{
	const auto Input = Value.Get<FVector2D>();

	const auto Character = CastChecked<ACharacter>(GetOwner());
	const FRotationMatrix Rotation(FRotator(0., Character->GetControlRotation().Yaw, 0.));
	Character->AddMovementInput(Rotation.GetUnitAxis(EAxis::X), Input.X);
	Character->AddMovementInput(Rotation.GetUnitAxis(EAxis::Y), Input.Y);
}

void UPlayerMoveComponent::OnInputLook(const FInputActionValue& Value)
{
	const auto Input = Value.Get<FVector2D>();

	const auto Pawn = CastChecked<APawn>(GetOwner());
	Pawn->AddControllerYawInput(Input.X);
	Pawn->AddControllerPitchInput(Input.Y);
}

void UPlayerMoveComponent::OnInputRunStarted()
{
	const auto Character = CastChecked<ACharacter>(GetOwner());
	const auto Movement = Character->GetCharacterMovement();

	Character->bUseControllerRotationYaw = false;
	Movement->bOrientRotationToMovement = true;
	Movement->MaxWalkSpeed = RunSpeed;
}

void UPlayerMoveComponent::OnInputRunStopped()
{
	const auto Character = CastChecked<ACharacter>(GetOwner());
	const auto Movement = Character->GetCharacterMovement();

	Character->bUseControllerRotationYaw = true;
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = WalkSpeed;
}
