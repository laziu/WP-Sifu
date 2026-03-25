// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTestGameMode.h"

#include "MainCharacter.h"
#include "MainCharPlayerController.h"

APlayerTestGameMode::APlayerTestGameMode()
{
	DefaultPawnClass = AMainCharacter::StaticClass();
	PlayerControllerClass = AMainCharPlayerController::StaticClass();
}
