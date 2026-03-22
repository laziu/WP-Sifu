// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MainCharCameraManager.h"
#include "UserExtension.h"

AMainCharPlayerController::AMainCharPlayerController()
{
	PlayerCameraManagerClass = AMainCharCameraManager::StaticClass();

	Ext::SetObject(InputMappingDefault,
	               TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Default.IMC_Default'"));
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
	}
}
