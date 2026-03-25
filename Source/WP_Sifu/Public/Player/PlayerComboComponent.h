// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Attackable.h"
#include "PlayerComboComponent.generated.h"


UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerComboComponent();

	// Action input (ActionTag: AttackAction.Light, AttackAction.Heavy, etc.)
	UFUNCTION(BlueprintCallable, Category=Combo)
	void InputAction(FGameplayTag ActionTag);

	// Reset to neutral (with stopping montage)
	UFUNCTION(BlueprintCallable, Category=Combo)
	void ResetToNeutral();

	// Force special state (Run, Parr, etc.)
	UFUNCTION(BlueprintCallable, Category=Combo)
	void SetCombatState(FGameplayTag NewStateTag);

	UFUNCTION(BlueprintPure, Category=Combo)
	FGameplayTag GetCurrentStateTag() const { return CurrentStateTag; }

	UFUNCTION(BlueprintPure, Category=Combo)
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category=Combo)
	FAttackPayload MakeCurrentAttackPayload() const;

	// --- AnimNotify Function ---

	UFUNCTION(BlueprintCallable, Category=Combo)
	void OpenTransitionWindow(FName TransitionId);

	UFUNCTION(BlueprintCallable, Category=Combo)
	void CloseTransitionWindow(FName TransitionId);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TObjectPtr<UDataTable> ComboTransitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TObjectPtr<UDataTable> AttackDamageTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TObjectPtr<UAnimMontage> ComboMontage;

private:
	FGameplayTag CurrentStateTag;
	TSet<FName> OpenTransitionWindows;

	// (CurrentStateTag, ActionTag) → {TransitionId, Row*}
	struct FTransitionEntry
	{
		FName TransitionId; // RowName
		const struct FPlayerComboTransitionRow* Row;
	};

	TMap<TPair<FGameplayTag, FGameplayTag>, FTransitionEntry> TransitionLookup;

	// AttackDamage lookup: StateTag → Row*
	TMap<FGameplayTag, const struct FPlayerAttackDamageRow*> DamageLookup;

	TOptional<FGameplayTag> BufferedActionTag;

	// Parry -> Neutral Timer
	FTimerHandle ParryResetTimerHandle;
	static constexpr float ParryStateDuration = 1.0f;

	// Cache
	FGameplayTag NeutralTag;
	FGameplayTag RunTag;
	FGameplayTag ParryTag;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void BuildTransitionLookup();
	void BuildDamageLookup();
	void ExecuteTransition(const FTransitionEntry& Entry);

	// Export montage section name from StateTag
	// e.g. "CombatState.L1" → "L1"
	static FName StateTagToSectionName(const FGameplayTag& Tag);
};
