// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMoveComponent.h"

#include "CameraFocusComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "PlayerCombatInteractionComponent.h"
#include "UserExtension.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerMoveComponent::UPlayerMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(InputLook, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
}

void UPlayerMoveComponent::OnRegister()
{
	Super::OnRegister();

	auto Character = CastChecked<ACharacter>(GetOwner());
	Character->bUseControllerRotationPitch = false;
	Character->bUseControllerRotationYaw = true;
	Character->bUseControllerRotationRoll = false;

	auto Movement = Character->GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = NormalSpeed;
	Movement->RotationRate = FRotator(0., 540., 0.);
	Movement->JumpZVelocity = 600.;
	Movement->AirControl = 0.2;
}

void UPlayerMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bInCombatStance && !bIsRunning)
	{
		UpdateCombatRotation(DeltaTime);
	}
}

void UPlayerMoveComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputMove, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputMove);
	EIC->BindAction(InputLook, ETriggerEvent::Triggered, this, &UPlayerMoveComponent::OnInputLook);
	EIC->BindAction(InputRun, ETriggerEvent::Started, this, &UPlayerMoveComponent::OnInputRunStarted);
	EIC->BindAction(InputRun, ETriggerEvent::Completed, this, &UPlayerMoveComponent::OnInputRunStopped);
	EIC->BindAction(InputRun, ETriggerEvent::Canceled, this, &UPlayerMoveComponent::OnInputRunStopped);
}

void UPlayerMoveComponent::SetCombatStance(bool bNewCombatStance)
{
	if (bInCombatStance == bNewCombatStance) return;
	bInCombatStance = bNewCombatStance;

	if (bIsRunning) return; // Running overrides combat stance rotation

	auto Character = CastChecked<ACharacter>(GetOwner());
	if (bInCombatStance)
	{
		// Combat stance: disable controller rotation, rotate toward target in Tick
		Character->bUseControllerRotationYaw = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		// Normal: follow camera
		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	UpdateMovementSpeed();
}

void UPlayerMoveComponent::OnInputMove(const FInputActionValue& Value)
{
	if (auto* CIC = GetOwner()->FindComponentByClass<UPlayerCombatInteractionComponent>())
	{
		if (CIC->IsMovementBlocked())
			return;
	}

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
	bIsRunning = true;

	const auto Character = CastChecked<ACharacter>(GetOwner());
	Character->bUseControllerRotationYaw = false;
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;

	UpdateMovementSpeed();
}

void UPlayerMoveComponent::OnInputRunStopped()
{
	bIsRunning = false;

	const auto Character = CastChecked<ACharacter>(GetOwner());

	if (bInCombatStance)
	{
		// Return to combat stance rotation
		Character->bUseControllerRotationYaw = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		// Return to normal: follow camera
		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	UpdateMovementSpeed();
}

void UPlayerMoveComponent::UpdateMovementSpeed()
{
	const auto Character = CastChecked<ACharacter>(GetOwner());
	auto* Movement = Character->GetCharacterMovement();

	if (bIsRunning)
	{
		Movement->MaxWalkSpeed = RunSpeed;
	}
	else if (bInCombatStance)
	{
		Movement->MaxWalkSpeed = CombatSpeed;
	}
	else
	{
		Movement->MaxWalkSpeed = NormalSpeed;
	}
}

void UPlayerMoveComponent::UpdateCombatRotation(float DeltaTime)
{
	auto* FocusComp = GetOwner()->FindComponentByClass<UCameraFocusComponent>();
	if (!FocusComp || !FocusComp->IsLockedOn()) return;

	const FVector FacingDir = FocusComp->GetFacingDirection();
	if (FacingDir.IsNearlyZero()) return;

	auto* Character = CastChecked<ACharacter>(GetOwner());
	const FRotator CurrentRot = Character->GetActorRotation();
	const FRotator TargetRot = FacingDir.Rotation();
	const FRotator DesiredRot(CurrentRot.Pitch, TargetRot.Yaw, CurrentRot.Roll);

	Character->SetActorRotation(
		FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, CombatRotationInterpSpeed));
}
