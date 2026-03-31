// Fill out your copyright notice in the Description page of Project Settings.

#include "InputBindableDefaultPawn.h"

#include "InputBindable.h"

void AInputBindableDefaultPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	FInputBindingHelper::BindAll(this, PlayerInputComponent);
}
