// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthAttributeSet.h"

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// Clamp the value
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStructureAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStructure());
	}

	Super::PreAttributeChange(Attribute, NewValue);
}

void UHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		OnHealthChanged.Broadcast(this, OldValue, NewValue);
		if (NewValue <= 0.f && OldValue > 0.f)
		{
			OnHealthDepleted.Broadcast();
		}
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		const float value = GetHealth();
		OnHealthChanged.Broadcast(this, value, value);
	}
	else if (Attribute == GetStructureAttribute())
	{
		OnStructureChanged.Broadcast(this, OldValue, NewValue);
	}
	else if (Attribute == GetMaxStructureAttribute())
	{
		const float value = GetStructure();
		OnStructureChanged.Broadcast(this, value, value);
	}
}
