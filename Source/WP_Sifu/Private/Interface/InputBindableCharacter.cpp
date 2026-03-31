// Fill out your copyright notice in the Description page of Project Settings.

#include "InputBindableCharacter.h"

#include "InputBindable.h"

void AInputBindableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	FInputBindingHelper::BindAll(this, PlayerInputComponent);
}
