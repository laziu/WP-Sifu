// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMoveComponent.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerMoveComponent::UPlayerMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Character rotation: follow the camera/controller yaw instead of movement direction
	if (auto Character = Cast<ACharacter>(GetOwner()))
	{
		Character->bUseControllerRotationPitch = false;
		Character->bUseControllerRotationYaw = true;
		Character->bUseControllerRotationRoll = false;
		if (auto Movement = Character->GetCharacterMovement())
		{
			Movement->bOrientRotationToMovement = false;
			Movement->RotationRate = FRotator(0., 540., 0.);
			Movement->JumpZVelocity = 600.;
			Movement->AirControl = 0.2;
		}
	}

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(InputLook, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
}

void UPlayerMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	SetOwnerWalkSpeed(WalkSpeed);
}

void UPlayerMoveComponent::SetupInputBindings(UEnhancedInputComponent* EnhancedInputComponent)
{
	EnhancedInputComponent->BindAction(InputMove, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputMove);
	EnhancedInputComponent->BindAction(InputLook, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputLook);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Started, this, &UPlayerMoveComponent::OnInputRunStarted);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Completed, this, &UPlayerMoveComponent::OnInputRunStopped);
	EnhancedInputComponent->BindAction(
		InputRun, ETriggerEvent::Canceled, this, &UPlayerMoveComponent::OnInputRunStopped);
}

void UPlayerMoveComponent::OnInputMove(const FInputActionValue& Value)
{
	const auto Owner = CastChecked<ACharacter>(GetOwner());

	const auto Input = Value.Get<FVector2D>();

	const FRotationMatrix Rotation(FRotator(0., Owner->GetControlRotation().Yaw, 0.));
	Owner->AddMovementInput(Rotation.GetUnitAxis(EAxis::X), Input.X);
	Owner->AddMovementInput(Rotation.GetUnitAxis(EAxis::Y), Input.Y);
}

void UPlayerMoveComponent::OnInputLook(const FInputActionValue& Value)
{
	const auto Pawn = CastChecked<APawn>(GetOwner());

	const auto Input = Value.Get<FVector2D>();

	Pawn->AddControllerYawInput(Input.X);
	Pawn->AddControllerPitchInput(Input.Y);
}

void UPlayerMoveComponent::OnInputRunStarted()
{
	SetOwnerWalkSpeed(RunSpeed);

	if (auto Character = Cast<ACharacter>(GetOwner()))
	{
		Character->bUseControllerRotationYaw = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void UPlayerMoveComponent::OnInputRunStopped()
{
	SetOwnerWalkSpeed(WalkSpeed);

	if (auto Character = Cast<ACharacter>(GetOwner()))
	{
		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void UPlayerMoveComponent::SetOwnerWalkSpeed(double NewSpeed) const
{
	if (const auto Owner = Cast<ACharacter>(GetOwner()))
	{
		if (auto* MovementComponent = Owner->GetCharacterMovement())
		{
			MovementComponent->MaxWalkSpeed = NewSpeed;
		}
	}
}
