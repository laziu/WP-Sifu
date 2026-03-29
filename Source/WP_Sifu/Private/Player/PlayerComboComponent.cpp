// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerComboComponent.h"

#include "GameplayTags.generated.h"
#include "PlayerComboTransitionRow.h"
#include "PlayerAttackDamageRow.h"
#include "UserExtension.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"


UPlayerComboComponent::UPlayerComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Ext::SetObject(ComboTransitionTable,
	               TEXT("/Script/Engine.DataTable'/Game/Data/DT_PlayerComboTransition.DT_PlayerComboTransition'"));
	Ext::SetObject(AttackDamageTable,
	               TEXT("/Script/Engine.DataTable'/Game/Data/DT_PlayerAttackDamage.DT_PlayerAttackDamage'"));
}

void UPlayerComboComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStateTag = GameplayTag::Combat_State_Neutral;

	BuildTransitionLookup();
	BuildDamageLookup();
	PreloadMontages();
}

void UPlayerComboComponent::BuildTransitionLookup()
{
	TransitionLookup.Empty();
	if (!ComboTransitionTable) return;

	const TMap<FName, uint8*>& RowMap = ComboTransitionTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const auto* Row = reinterpret_cast<const FPlayerComboTransitionRow*>(Pair.Value);
		TransitionLookup.Add({Row->CurrentStateTag, Row->ActionTag}, Row);
	}
}

void UPlayerComboComponent::BuildDamageLookup()
{
	DamageLookup.Empty();
	if (!AttackDamageTable) return;

	const TMap<FName, uint8*>& RowMap = AttackDamageTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const auto* Row = reinterpret_cast<const FPlayerAttackDamageRow*>(Pair.Value);
		if (Row->StateTag.IsValid())
		{
			DamageLookup.Add(Row->StateTag, Row);
		}
	}
}

void UPlayerComboComponent::PreloadMontages()
{
	MontageCache.Empty();
	for (const auto& Pair : DamageLookup)
	{
		const TSoftObjectPtr<UAnimMontage>& SoftMontage = Pair.Value->Montage;
		if (!SoftMontage.IsNull())
		{
			MontageCache.Add(Pair.Key, SoftMontage.LoadSynchronous());
		}
	}
}

void UPlayerComboComponent::InputAction(FGameplayTag ActionTag)
{
	const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, ActionTag};
	const auto* EntryPtr = TransitionLookup.Find(Key);
	if (!EntryPtr)
	{
		return;
	}

	// TODO: Appropriate state check with Neutral, Run, Parry
	if (CurrentStateTag == GameplayTag::Combat_State_Neutral ||
		CurrentStateTag == GameplayTag::Combat_State_Neutral_Run ||
		CurrentStateTag == GameplayTag::Combat_State_Parry ||
		OpenTransitionWindows.Contains(ActionTag))
	{
		ExecuteTransition(**EntryPtr);
	}
	else
	{
		// queue input (max 1)
		BufferedActionTag = ActionTag;
	}
}

void UPlayerComboComponent::ExecuteTransition(const FPlayerComboTransitionRow& Entry)
{
	CurrentStateTag = Entry.NextStateTag;
	OpenTransitionWindows.Empty();
	BufferedActionTag.Reset();

	// Clear parry reset timer after transition (consume it)
	GetWorld()->GetTimerManager().ClearTimer(ParryResetTimerHandle);

	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	auto CachedMontage = MontageCache.Find(CurrentStateTag);
	if (!CachedMontage || !*CachedMontage) return;

	ActiveMontage = *CachedMontage;

	// Playing a new montage on the same slot blends out the previous one automatically
	AnimInstance->Montage_Play(ActiveMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UPlayerComboComponent::OnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveMontage);
}

void UPlayerComboComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Guard against stale delegates firing after a new montage has already started
	if (Montage == ActiveMontage)
	{
		ActiveMontage = nullptr;
		ResetToNeutral();
	}
}

void UPlayerComboComponent::ResetToNeutral()
{
	CurrentStateTag = GameplayTag::Combat_State_Neutral;
	OpenTransitionWindows.Empty();
	BufferedActionTag.Reset();
	GetWorld()->GetTimerManager().ClearTimer(ParryResetTimerHandle);

	if (ActiveMontage)
	{
		if (auto* Character = Cast<ACharacter>(GetOwner()))
		{
			if (auto* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				if (AnimInstance->Montage_IsPlaying(ActiveMontage))
				{
					AnimInstance->Montage_Stop(0.25f, ActiveMontage);
				}
			}
		}
		ActiveMontage = nullptr;
	}
}

void UPlayerComboComponent::SetCombatState(FGameplayTag NewStateTag)
{
	if (IsAttacking()) return;

	CurrentStateTag = NewStateTag;

	if (NewStateTag == GameplayTag::Combat_State_Parry)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ParryResetTimerHandle, this,
			&UPlayerComboComponent::ResetToNeutral,
			ParryStateDuration, false);
	}
}

bool UPlayerComboComponent::IsAttacking() const
{
	if (!ActiveMontage) return false;
	if (const auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const auto* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			return AnimInstance->Montage_IsPlaying(ActiveMontage);
		}
	}
	return false;
}

FAttackPayload UPlayerComboComponent::MakeCurrentAttackPayload() const
{
	FAttackPayload Payload;
	Payload.Instigator = GetOwner();

	if (const auto* RowPtr = DamageLookup.Find(CurrentStateTag))
	{
		Payload.HealthDamage = (*RowPtr)->Damage;
		Payload.StructureDamage = (*RowPtr)->Damage;
		Payload.HitReaction = (*RowPtr)->ResultTag;
	}

	return Payload;
}

void UPlayerComboComponent::OpenTransitionWindow(FGameplayTag ActionTag)
{
	OpenTransitionWindows.Add(ActionTag);

	if (BufferedActionTag.IsSet())
	{
		const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, BufferedActionTag.GetValue()};
		if (const auto* EntryPtr = TransitionLookup.Find(Key))
		{
			if ((*EntryPtr)->ActionTag == ActionTag)
			{
				ExecuteTransition(**EntryPtr);
			}
		}
	}
}

void UPlayerComboComponent::CloseTransitionWindow(FGameplayTag ActionTag)
{
	OpenTransitionWindows.Remove(ActionTag);
}
