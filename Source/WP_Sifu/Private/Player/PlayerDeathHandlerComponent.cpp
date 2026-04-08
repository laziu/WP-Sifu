// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerDeathHandlerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "HealthAttributeSet.h"
#include "PlayerDeathScreenWidget.h"
#include "UserExtension.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


UPlayerDeathHandlerComponent::UPlayerDeathHandlerComponent()
{
	Ext::SetObject(DeathMontage,
		TEXT("/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_Death.AM_MainChar_Death'"));
	Ext::SetObject(ReviveMontage,
		TEXT("/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_Revive.AM_MainChar_Revive'"));
	Ext::SetClass(DeathScreenWidgetClass,
		TEXT("/Game/Blueprints/Widgets/WBP_DeadScreen"));
}


void UPlayerDeathHandlerComponent::OnDeathBegin()
{
	// Disable player input
	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (auto* PC = Cast<APlayerController>(Character->GetController()))
		{
			Character->DisableInput(PC);
		}
	}

	// Increment death count
	DeathCount++;

	// Slow down game time
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), DeathTimeDilation);

	// Show death screen widget
	if (DeathScreenWidgetClass)
	{
		if (auto* PC = Cast<APlayerController>(Cast<ACharacter>(GetOwner())->GetController()))
		{
			DeathScreenInstance = CreateWidget<UPlayerDeathScreenWidget>(PC, DeathScreenWidgetClass);
			if (DeathScreenInstance)
			{
				DeathScreenInstance->Show(this, DeathCount);
			}
		}
	}
}


void UPlayerDeathHandlerComponent::OnAgingComplete()
{
	DeathScreenInstance = nullptr;

	// Restore game speed
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// Restore health
	RestoreHealth();

	// Force-play revive montage (interrupts the looping death montage)
	auto* Character = Cast<ACharacter>(GetOwner());
	if (Character && ReviveMontage)
	{
		auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			const float Duration = AnimInstance->Montage_Play(ReviveMontage);
			if (Duration > 0.f)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UPlayerDeathHandlerComponent::OnReviveMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, ReviveMontage);
				return;
			}
		}
	}

	// If no revive montage, finish immediately
	OnReviveMontageEnded(nullptr, false);
}


void UPlayerDeathHandlerComponent::OnReviveMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsDead = false;

	// Re-enable player input
	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (auto* PC = Cast<APlayerController>(Character->GetController()))
		{
			Character->EnableInput(PC);
		}
	}

	OnDeathFinished.Broadcast();
}


void UPlayerDeathHandlerComponent::RestoreHealth()
{
	if (auto* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (auto* ASC = ASI->GetAbilitySystemComponent())
		{
			if (auto* HealthAttr = ASC->GetSet<UHealthAttributeSet>())
			{
				ASC->SetNumericAttributeBase(
					UHealthAttributeSet::GetHealthAttribute(),
					HealthAttr->GetMaxHealth());

				// Reset structure to 0 on revive
				ASC->SetNumericAttributeBase(
					UHealthAttributeSet::GetStructureAttribute(),
					0.f);
			}
		}
	}
}
