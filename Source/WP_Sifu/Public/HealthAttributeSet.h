// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class WP_SIFU_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	// Health starts at MaxHealth, and is reduced by damage. Health=0 -> die
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Health)
	FGameplayAttributeData Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Health)
	FGameplayAttributeData MaxHealth;

	// Structure starts at 0, and is increased by damage. Structure=MaxStructure -> break
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Health)
	FGameplayAttributeData Structure;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Health)
	FGameplayAttributeData MaxStructure;

	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, Structure);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, MaxStructure);
};
