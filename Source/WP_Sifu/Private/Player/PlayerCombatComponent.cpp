// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "HealthAttributeSet.h"


UPlayerCombatComponent::UPlayerCombatComponent()
{
	bWantsInitializeComponent = true;
}

void UPlayerCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const auto ASI = CastChecked<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComp = ASI->GetAbilitySystemComponent();
}

EAttackResponse UPlayerCombatComponent::ApplyDamage(const FAttackPayload& Payload)
{
	switch (DefenceState)
	{
	case EDefenceState::None:
		{
			// full damage
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetHealthAttribute(),
				EGameplayModOp::Additive, -Payload.HealthDamage);

			if (IsAttackFromBehind(Payload.ImpactLocation))
			{
				// back attack: structure=100%
				const float MaxStructure = AbilitySystemComp->GetNumericAttribute(
					UHealthAttributeSet::GetMaxStructureAttribute());
				AbilitySystemComp->SetNumericAttributeBase(
					UHealthAttributeSet::GetStructureAttribute(), MaxStructure);
			}
			else
			{
				AbilitySystemComp->ApplyModToAttribute(
					UHealthAttributeSet::GetStructureAttribute(),
					EGameplayModOp::Additive, Payload.StructureDamage);
			}

			return EAttackResponse::Hit;
		}

	case EDefenceState::Blocking:
		{
			// no health damage, full structure damage
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive, Payload.StructureDamage);

			return EAttackResponse::Block;
		}

	case EDefenceState::Dodging:
	case EDefenceState::Parrying:
		{
			// no health damage, reduce structure
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive, Payload.StructureDamage * DeflectStructureRate);

			return DefenceState == EDefenceState::Parrying
				       ? EAttackResponse::Parry
				       : EAttackResponse::Dodge;
		}

	case EDefenceState::Invincible:
	default:
		return EAttackResponse::Ignore;
	}
}

bool UPlayerCombatComponent::IsAttackFromBehind(const FVector& ImpactLocation) const
{
	// false when ImpactLocation is not set (e.g., for unblockable attacks where location is irrelevant)
	if (ImpactLocation.IsZero())
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	const FVector ToImpact = (ImpactLocation - Owner->GetActorLocation()).GetSafeNormal();
	const FVector Forward = Owner->GetActorForwardVector();

	// Dot product < 0 means the attack comes from behind (angle > 90 degrees)
	return FVector::DotProduct(Forward, ToImpact) < 0.f;
}
