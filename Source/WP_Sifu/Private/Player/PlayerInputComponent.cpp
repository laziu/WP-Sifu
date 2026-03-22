// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInputComponent.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UPlayerInputComponent::UPlayerInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(InputLook, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
}

void UPlayerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	SetOwnerWalkSpeed(WalkSpeed);
}

void UPlayerInputComponent::SetupInputBindings(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent) return;

	EnhancedInputComponent->BindAction(InputMove, ETriggerEvent::Triggered, this, &UPlayerInputComponent::OnInputMove);
	EnhancedInputComponent->BindAction(InputLook, ETriggerEvent::Triggered, this, &UPlayerInputComponent::OnInputLook);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Started, this, &UPlayerInputComponent::OnInputRunStarted);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Completed, this, &UPlayerInputComponent::OnInputRunStopped);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Canceled, this, &UPlayerInputComponent::OnInputRunStopped);
}

void UPlayerInputComponent::OnInputMove(const FInputActionValue& Value)
{
	const auto Owner = CastChecked<ACharacter>(GetOwner());

	const auto Input = Value.Get<FVector2D>();

	const FRotationMatrix Rotation(FRotator(0., Owner->GetControlRotation().Yaw, 0.));
	Owner->AddMovementInput(Rotation.GetUnitAxis(EAxis::X), Input.X);
	Owner->AddMovementInput(Rotation.GetUnitAxis(EAxis::Y), Input.Y);
}

void UPlayerInputComponent::OnInputLook(const FInputActionValue& Value)
{
	const auto Pawn = CastChecked<APawn>(GetOwner());

	const auto Input = Value.Get<FVector2D>();

	Pawn->AddControllerYawInput(Input.X);
	Pawn->AddControllerPitchInput(Input.Y);
}

void UPlayerInputComponent::OnInputRunStarted()
{
	SetOwnerWalkSpeed(RunSpeed);
}

void UPlayerInputComponent::OnInputRunStopped()
{
	SetOwnerWalkSpeed(WalkSpeed);
}

void UPlayerInputComponent::SetOwnerWalkSpeed(double NewSpeed) const
{
	if (const auto Owner = Cast<ACharacter>(GetOwner()))
	{
		if (auto* MovementComponent = Owner->GetCharacterMovement())
		{
			MovementComponent->MaxWalkSpeed = NewSpeed;
		}
	}
}
