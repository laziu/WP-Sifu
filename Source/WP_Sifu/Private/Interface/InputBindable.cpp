// Fill out your copyright notice in the Description page of Project Settings.

#include "InputBindable.h"

#include "EnhancedInputComponent.h"

void FInputBindingHelper::BindAll(AActor* Actor, UInputComponent* InputComponent)
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);

		for (UActorComponent* Comp : Components)
		{
			if (IInputBindable* Bindable = Cast<IInputBindable>(Comp))
			{
				Bindable->SetupInputBindings(EIC);
			}
		}
	}
}
