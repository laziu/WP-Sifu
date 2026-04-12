// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputBindable.h"
#include "InputMappingContext.h"
#include "MainCharCameraManager.h"
#include "UserExtension.h"

AMainCharPlayerController::AMainCharPlayerController()
{
	PlayerCameraManagerClass = AMainCharCameraManager::StaticClass();

	Ext::SetObject(InputMappingDefault,
	               TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Default.IMC_Default'"));
	Ext::SetObject(InputMappingCombat,
	               TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Combat.IMC_Combat'"));
}

void AMainCharPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InPawn->EnableInput(this);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InPawn->InputComponent))
	{
		for (UActorComponent* Comp : InPawn->GetComponents())
		{
			if (IInputBindable* Bindable = Cast<IInputBindable>(Comp))
			{
				Bindable->SetupInputBindings(EIC);
			}
		}
	}
}

void AMainCharPlayerController::OnUnPossess()
{
	
	if (APawn* MyPawn = GetPawn())
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(MyPawn->InputComponent))
		{
			EIC->ClearActionBindings();
		}
	}
	Super::OnUnPossess();
}

void AMainCharPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingDefault)
		{
			Subsystem->AddMappingContext(InputMappingDefault, 0);
		}
		if (InputMappingCombat)
		{
			Subsystem->AddMappingContext(InputMappingCombat, 0);
		}
	}
}
