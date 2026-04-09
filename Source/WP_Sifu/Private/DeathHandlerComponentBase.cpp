// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathHandlerComponentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "HealthAttributeSet.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"


void UDeathHandlerComponentBase::BeginPlay()
{
	Super::BeginPlay();

	if (auto* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (auto* ASC = ASI->GetAbilitySystemComponent())
		{
			if (auto* HealthAttr = ASC->GetSet<UHealthAttributeSet>())
			{
				// GetSet returns const; we need the mutable version for delegate binding
				auto* MutableHealthAttr = const_cast<UHealthAttributeSet*>(HealthAttr);
				MutableHealthAttr->OnHealthDepleted.AddDynamic(this, &UDeathHandlerComponentBase::HandleHealthDepleted);
				return;
			}
		}
	}

	LOGW(TEXT("DeathHandlerComponent: No UHealthAttributeSet found on %s"), *GetOwner()->GetName());
}

void UDeathHandlerComponentBase::HandleHealthDepleted()
{
	if (bIsDead) return;

	bIsDead = true;
	OnDeathStarted.Broadcast();
	OnDeathBegin();

	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !DeathMontage)
	{
		LOGW(TEXT("DeathHandlerComponent: Missing Character or DeathMontage on %s"), *GetOwner()->GetName());
		OnDeathComplete();
		return;
	}

	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		OnDeathComplete();
		return;
	}

	// Stop any current montage before playing death
	AnimInstance->StopAllMontages(0.2f);

	const float Duration = AnimInstance->Montage_Play(DeathMontage);
	if (Duration > 0.f)
	{
		OnDeathMontageStarted(AnimInstance);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UDeathHandlerComponentBase::OnDeathMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
	}
	else
	{
		OnDeathComplete();
	}
}

void UDeathHandlerComponentBase::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnDeathComplete();
}
