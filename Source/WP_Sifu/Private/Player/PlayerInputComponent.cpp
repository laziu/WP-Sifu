// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInputComponent.h"

#include "EnhancedInputComponent.h"
#include "GameplayTags.generated.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "PlayerComboComponent.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UPlayerInputComponent::UPlayerInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(InputLook, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
	Ext::SetObject(InputLightAttack, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_LightAttack.IA_LightAttack'"));
	Ext::SetObject(InputHeavyAttack, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_HeavyAttack.IA_HeavyAttack'"));
	Ext::SetObject(InputBlock, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Block.IA_Block'"));
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
	EnhancedInputComponent->BindAction(
		InputLightAttack, ETriggerEvent::Started, this, &UPlayerInputComponent::OnInputLightAttack);
	EnhancedInputComponent->BindAction(
		InputHeavyAttack, ETriggerEvent::Started, this, &UPlayerInputComponent::OnInputHeavyAttack);
	EnhancedInputComponent->BindAction(
		InputBlock, ETriggerEvent::Started, this, &UPlayerInputComponent::OnInputBlockStarted);
	EnhancedInputComponent->BindAction(
		InputBlock, ETriggerEvent::Completed, this, &UPlayerInputComponent::OnInputBlockStopped);
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
	if (auto* Combo = GetOwner()->FindComponentByClass<UPlayerComboComponent>())
		Combo->SetCombatState(GameplayTag::CombatState_Run);
}

void UPlayerInputComponent::OnInputRunStopped()
{
	SetOwnerWalkSpeed(WalkSpeed);
	if (auto* Combo = GetOwner()->FindComponentByClass<UPlayerComboComponent>())
		Combo->SetCombatState(GameplayTag::CombatState_Neutral);
}

void UPlayerInputComponent::OnInputLightAttack()
{
	if (auto* Combo = GetOwner()->FindComponentByClass<UPlayerComboComponent>())
		Combo->InputAction(GameplayTag::AttackAction_Light);
}

void UPlayerInputComponent::OnInputHeavyAttack()
{
	if (auto* Combo = GetOwner()->FindComponentByClass<UPlayerComboComponent>())
		Combo->InputAction(GameplayTag::AttackAction_Heavy);
}

void UPlayerInputComponent::OnInputBlockStarted()
{
	if (auto* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
		Combat->StartBlock();
}

void UPlayerInputComponent::OnInputBlockStopped()
{
	if (auto* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
		Combat->StopBlock();
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
