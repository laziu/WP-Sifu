// Fill out your copyright notice in the Description page of Project Settings.


#include "DummyCombatInteractionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "HealthAttributeSet.h"


UDummyCombatInteractionComponent::UDummyCombatInteractionComponent()
{
	bWantsInitializeComponent = true;
}

void UDummyCombatInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const auto ASI = CastChecked<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComp = ASI->GetAbilitySystemComponent();
}

EAttackResponse UDummyCombatInteractionComponent::ApplyDamage(const FAttackPayload& Payload)
{
	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive, -Payload.HealthDamage);

	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetStructureAttribute(),
		EGameplayModOp::Additive, Payload.StructureDamage);

	return EAttackResponse::Hit;
}
