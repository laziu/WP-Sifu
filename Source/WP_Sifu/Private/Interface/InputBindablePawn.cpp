// Fill out your copyright notice in the Description page of Project Settings.

#include "InputBindablePawn.h"

#include "InputBindable.h"

void AInputBindablePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	FInputBindingHelper::BindAll(this, PlayerInputComponent);
}
