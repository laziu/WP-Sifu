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

	// Action input (ActionTag: Combat.Command.Light, Combat.Command.Heavy, etc.)
	UFUNCTION(BlueprintCallable, Category=Combo)
	void InputAction(FGameplayTag ActionTag);

	// Reset to neutral (with stopping montage)
	UFUNCTION(BlueprintCallable, Category=Combo)
	void ResetToNeutral();

	// Force special state (Run, Parry, etc.)
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
	void OpenTransitionWindow(FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category=Combo)
	void CloseTransitionWindow(FGameplayTag ActionTag);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TObjectPtr<UDataTable> ComboTransitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TObjectPtr<UDataTable> AttackDamageTable;

private:
	FGameplayTag CurrentStateTag;
	TSet<FGameplayTag> OpenTransitionWindows;

	TMap<TPair<FGameplayTag, FGameplayTag>, const struct FPlayerComboTransitionRow*> TransitionLookup;

	// AttackDamage lookup: attack/state tag -> row
	TMap<FGameplayTag, const struct FPlayerAttackDamageRow*> DamageLookup;

	// Preloaded montages: StateTag → Montage (populated at BeginPlay)
	TMap<FGameplayTag, TObjectPtr<class UAnimMontage>> MontageCache;

	// Currently playing montage (nullptr when not attacking)
	TObjectPtr<class UAnimMontage> ActiveMontage;

	TOptional<FGameplayTag> BufferedActionTag;

	// Parry -> Neutral Timer
	FTimerHandle ParryResetTimerHandle;
	static constexpr float ParryStateDuration = 1.0f;

	void OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted);
	void BuildTransitionLookup();
	void BuildDamageLookup();
	void PreloadMontages();
	void ExecuteTransition(const struct FPlayerComboTransitionRow& Entry);
};
