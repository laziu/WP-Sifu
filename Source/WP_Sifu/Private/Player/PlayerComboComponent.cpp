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
	Ext::SetObject(ComboMontage,
	               TEXT(
		               "/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/AM_MainChar_Attack.AM_MainChar_Attack'"));
}

void UPlayerComboComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStateTag = GameplayTag::CombatState_Neutral;

	BuildTransitionLookup();
	BuildDamageLookup();
}

void UPlayerComboComponent::BuildTransitionLookup()
{
	TransitionLookup.Empty();
	if (!ComboTransitionTable) return;

	const TMap<FName, uint8*>& RowMap = ComboTransitionTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const auto* Row = reinterpret_cast<const FPlayerComboTransitionRow*>(Pair.Value);
		TransitionLookup.Add(
			{Row->CurrentStateTag, Row->ActionTag},
			{Pair.Key, Row});
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

void UPlayerComboComponent::InputAction(FGameplayTag ActionTag)
{
	const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, ActionTag};
	const auto* EntryPtr = TransitionLookup.Find(Key);
	if (!EntryPtr)
	{
		return;
	}

	// TODO: Appropriate state check with Neutral, Run, Parry 
	if (CurrentStateTag == GameplayTag::CombatState_Neutral ||
		CurrentStateTag == GameplayTag::CombatState_Run ||
		CurrentStateTag == GameplayTag::CombatState_Parry ||
		OpenTransitionWindows.Contains(EntryPtr->TransitionId))
	{
		ExecuteTransition(*EntryPtr);
	}
	else
	{
		// queue input (max 1)
		BufferedActionTag = ActionTag;
	}
}

void UPlayerComboComponent::ExecuteTransition(const FTransitionEntry& Entry)
{
	CurrentStateTag = Entry.Row->NextStateTag;
	OpenTransitionWindows.Empty();
	BufferedActionTag.Reset();

	// Clear parry reset timer after transition (consume it)
	GetWorld()->GetTimerManager().ClearTimer(ParryResetTimerHandle);

	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !ComboMontage) return;

	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	const FName SectionName = StateTagToSectionName(CurrentStateTag);

	if (!AnimInstance->Montage_IsPlaying(ComboMontage))
	{
		AnimInstance->Montage_Play(ComboMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UPlayerComboComponent::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboMontage);
	}

	AnimInstance->Montage_JumpToSection(SectionName, ComboMontage);
}

void UPlayerComboComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ResetToNeutral();
}

void UPlayerComboComponent::ResetToNeutral()
{
	CurrentStateTag = GameplayTag::CombatState_Neutral;
	OpenTransitionWindows.Empty();
	BufferedActionTag.Reset();
	GetWorld()->GetTimerManager().ClearTimer(ParryResetTimerHandle);

	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (auto* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			if (AnimInstance->Montage_IsPlaying(ComboMontage))
			{
				AnimInstance->Montage_Stop(0.25f, ComboMontage);
			}
		}
	}
}

void UPlayerComboComponent::SetCombatState(FGameplayTag NewStateTag)
{
	if (IsAttacking()) return;

	CurrentStateTag = NewStateTag;

	if (NewStateTag == GameplayTag::CombatState_Parry)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ParryResetTimerHandle, this,
			&UPlayerComboComponent::ResetToNeutral,
			ParryStateDuration, false);
	}
}

bool UPlayerComboComponent::IsAttacking() const
{
	if (const auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const auto* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			return AnimInstance->Montage_IsPlaying(ComboMontage);
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

void UPlayerComboComponent::OpenTransitionWindow(FName TransitionId)
{
	OpenTransitionWindows.Add(TransitionId);

	if (BufferedActionTag.IsSet())
	{
		const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, BufferedActionTag.GetValue()};
		if (const auto* EntryPtr = TransitionLookup.Find(Key))
		{
			if (EntryPtr->TransitionId == TransitionId)
			{
				ExecuteTransition(*EntryPtr);
			}
		}
	}
}

void UPlayerComboComponent::CloseTransitionWindow(FName TransitionId)
{
	OpenTransitionWindows.Remove(TransitionId);
}

FName UPlayerComboComponent::StateTagToSectionName(const FGameplayTag& Tag)
{
	// "CombatState.L1" → "L1", "CombatState.Parry.L" → "Parry.L"
	const FString TagStr = Tag.ToString();
	const FString Prefix = TEXT("CombatState.");
	if (TagStr.StartsWith(Prefix))
	{
		return FName(*TagStr.Mid(Prefix.Len()));
	}
	return FName(*TagStr);
}
