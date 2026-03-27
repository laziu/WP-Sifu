// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackComponent.h"

#include "EnhancedInputComponent.h"
#include "GameplayTags.generated.h"
#include "PlayerComboTransitionRow.h"
#include "PlayerAttackDefinitionRow.h"
#include "UserExtension.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"


UPlayerAttackComponent::UPlayerAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Ext::SetObject(InputRun, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
	Ext::SetObject(InputLightAttack,
	               TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_LightAttack.IA_LightAttack'"));
	Ext::SetObject(InputHeavyAttack,
	               TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_HeavyAttack.IA_HeavyAttack'"));

	Ext::SetObject(ComboTransitionTable,
	               TEXT("/Script/Engine.DataTable'/Game/Data/DT_PlayerComboTransition.DT_PlayerComboTransition'"));
	Ext::SetObject(AttackDefinitionTable,
	               TEXT("/Script/Engine.DataTable'/Game/Data/DT_PlayerAttackDefinition.DT_PlayerAttackDefinition'"));
}

void UPlayerAttackComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputRun, ETriggerEvent::Started, this, &UPlayerAttackComponent::OnInputRunStarted);
	EIC->BindAction(InputRun, ETriggerEvent::Completed, this, &UPlayerAttackComponent::OnInputRunStopped);
	EIC->BindAction(InputRun, ETriggerEvent::Canceled, this, &UPlayerAttackComponent::OnInputRunStopped);
	EIC->BindAction(InputLightAttack, ETriggerEvent::Started, this, &UPlayerAttackComponent::OnInputLightAttack);
	EIC->BindAction(InputHeavyAttack, ETriggerEvent::Started, this, &UPlayerAttackComponent::OnInputHeavyAttack);
}

void UPlayerAttackComponent::OnInputRunStarted()
{
	SetState(GameplayTag::Combat_State_Neutral_Run);
}

void UPlayerAttackComponent::OnInputRunStopped()
{
	SetState(GameplayTag::Combat_State_Neutral);
}

void UPlayerAttackComponent::OnInputLightAttack()
{
	InputAction(GameplayTag::Combat_Input_Light);
}

void UPlayerAttackComponent::OnInputHeavyAttack()
{
	InputAction(GameplayTag::Combat_Input_Heavy);
}

void UPlayerAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStateTag = GameplayTag::Combat_State_Neutral;

	// Backup the default table configured in the editor
	DefaultAttackDefinitionTable = AttackDefinitionTable;

	BuildTransitionLookup();
	BuildAttackDefinitionLookup();
	PreloadMontages();
}

void UPlayerAttackComponent::BuildTransitionLookup()
{
	TransitionLookup.Empty();
	if (!ComboTransitionTable) return;

	const TMap<FName, uint8*>& RowMap = ComboTransitionTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const auto* Row = reinterpret_cast<const FPlayerComboTransitionRow*>(Pair.Value);
		TransitionLookup.Add({Row->CurrentState, Row->Input}, Row);
	}
}

void UPlayerAttackComponent::BuildAttackDefinitionLookup()
{
	AttackDefinitionLookup.Empty();
	if (!AttackDefinitionTable) return;

	const TMap<FName, uint8*>& RowMap = AttackDefinitionTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const auto* Row = reinterpret_cast<const FPlayerAttackDefinitionRow*>(Pair.Value);
		if (Row->State.IsValid())
		{
			AttackDefinitionLookup.Add(Row->State, Row);
		}
	}
}

void UPlayerAttackComponent::PreloadMontages()
{
	MontageCache.Empty();
	for (const auto& Pair : AttackDefinitionLookup)
	{
		const TSoftObjectPtr<UAnimMontage>& SoftMontage = Pair.Value->Montage;
		if (!SoftMontage.IsNull())
		{
			MontageCache.Add(Pair.Key, SoftMontage.LoadSynchronous());
		}
	}
}

void UPlayerAttackComponent::InputAction(FGameplayTag InputTag)
{
	const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, InputTag};
	const auto* EntryPtr = TransitionLookup.Find(Key);
	if (!EntryPtr)
	{
		return;
	}

	// TODO: Appropriate state check with Neutral, Run, Parry
	if (CurrentStateTag == GameplayTag::Combat_State_Neutral ||
		CurrentStateTag == GameplayTag::Combat_State_Neutral_Run ||
		CurrentStateTag == GameplayTag::Combat_State_Parry ||
		OpenTransitionWindows.Contains(InputTag))
	{
		ExecuteTransition(**EntryPtr);
	}
	else
	{
		// queue input (max 1)
		BufferedInputTag = InputTag;
	}
}

void UPlayerAttackComponent::ExecuteTransition(const FPlayerComboTransitionRow& Entry)
{
	CurrentStateTag = Entry.NextState;
	OpenTransitionWindows.Empty();
	BufferedInputTag.Reset();

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
	EndDelegate.BindUObject(this, &UPlayerAttackComponent::OnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveMontage);
}

void UPlayerAttackComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Guard against stale delegates firing after a new montage has already started
	if (Montage == ActiveMontage)
	{
		ActiveMontage = nullptr;
		ResetToNeutral();
	}
}

void UPlayerAttackComponent::ResetToNeutral()
{
	CurrentStateTag = GameplayTag::Combat_State_Neutral;
	OpenTransitionWindows.Empty();
	BufferedInputTag.Reset();
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

void UPlayerAttackComponent::SetState(FGameplayTag NewStateTag)
{
	if (IsAttacking()) return;

	CurrentStateTag = NewStateTag;

	if (NewStateTag == GameplayTag::Combat_State_Parry)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ParryResetTimerHandle, this,
			&UPlayerAttackComponent::ResetToNeutral,
			ParryStateDuration, false);
	}
}

bool UPlayerAttackComponent::IsAttacking() const
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

FAttackPayload UPlayerAttackComponent::MakeCurrentAttackPayload() const
{
	FAttackPayload Payload;
	Payload.Instigator = GetOwner();

	if (const auto* RowPtr = AttackDefinitionLookup.Find(CurrentStateTag))
	{
		Payload.HealthDamage = (*RowPtr)->Damage;
		Payload.StructureDamage = (*RowPtr)->Damage;
		Payload.HitReaction = (*RowPtr)->Reaction;
	}

	return Payload;
}

void UPlayerAttackComponent::OpenTransitionWindow(FGameplayTag InputTag)
{
	OpenTransitionWindows.Add(InputTag);

	if (BufferedInputTag.IsSet())
	{
		const auto Key = TPair<FGameplayTag, FGameplayTag>{CurrentStateTag, BufferedInputTag.GetValue()};
		if (const auto* EntryPtr = TransitionLookup.Find(Key))
		{
			if ((*EntryPtr)->Input == InputTag)
			{
				ExecuteTransition(**EntryPtr);
			}
		}
	}
}

void UPlayerAttackComponent::CloseTransitionWindow(FGameplayTag InputTag)
{
	OpenTransitionWindows.Remove(InputTag);
}

void UPlayerAttackComponent::SetAttackTable(UDataTable* NewTable)
{
	AttackDefinitionTable = NewTable;
	BuildAttackDefinitionLookup();
	PreloadMontages();
}
