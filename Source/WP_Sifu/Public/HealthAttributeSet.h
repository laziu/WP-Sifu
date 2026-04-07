// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"


// Delegate definition
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FAttributeChangedEvent,
	UAttributeSet*, AttributeSet, float, OldValue, float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepleted);


/**
 * 
 */
UCLASS()
class WP_SIFU_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

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

	// Helper functions for attributes
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, Structure);
	ATTRIBUTE_ACCESSORS_BASIC(UHealthAttributeSet, MaxStructure);

	// Events
	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnStructureChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDepleted OnHealthDepleted;
};
