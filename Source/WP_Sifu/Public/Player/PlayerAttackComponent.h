// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Attackable.h"
#include "InputBindable.h"
#include "PlayerAttackComponent.generated.h"


UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerAttackComponent : public UActorComponent, public IInputBindable
{
	GENERATED_BODY()

public:
	UPlayerAttackComponent();

	virtual void SetupInputBindings(class UEnhancedInputComponent* EIC) override;

public:
	/// Input tag (Combat.Input.Light, Combat.Input.Heavy, etc.)
	UFUNCTION(BlueprintCallable, Category=Attack)
	void InputAction(FGameplayTag InputTag);

	/// Reset to neutral (with stopping montage)
	UFUNCTION(BlueprintCallable, Category=Attack)
	void ResetToNeutral();

	/// Force special state (Run, Parry, etc.)
	UFUNCTION(BlueprintCallable, Category=Attack)
	void SetState(FGameplayTag NewStateTag);

	UFUNCTION(BlueprintPure, Category=Attack)
	FGameplayTag GetCurrentStateTag() const { return CurrentStateTag; }

	UFUNCTION(BlueprintPure, Category=Attack)
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category=Attack)
	FAttackPayload MakeCurrentAttackPayload() const;

public: // --- Attack Table Management (swapped on weapon equip/unequip) ---
	/// Replaces the current AttackDefinitionTable and rebuilds the cache.
	UFUNCTION(BlueprintCallable, Category=Attack)
	void SetAttackTable(UDataTable* NewTable);

	/// Returns the default unarmed AttackDefinitionTable configured in the editor.
	UFUNCTION(BlueprintPure, Category=Attack)
	UDataTable* GetDefaultAttackTable() const { return DefaultAttackDefinitionTable; }

public: // --- AnimNotify Function ---
	UFUNCTION(BlueprintCallable, Category=Attack)
	void OpenTransitionWindow(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable, Category=Attack)
	void CloseTransitionWindow(FGameplayTag InputTag);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputLightAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputHeavyAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Attack)
	TObjectPtr<UDataTable> ComboTransitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Attack)
	TObjectPtr<UDataTable> AttackDefinitionTable;

	/// Default table configured in the editor, initialized in BeginPlay for restore.
	UPROPERTY()
	TObjectPtr<UDataTable> DefaultAttackDefinitionTable;

private:
	FGameplayTag CurrentStateTag;
	TSet<FGameplayTag> OpenTransitionWindows;

	TMap<TPair<FGameplayTag, FGameplayTag>, const struct FPlayerComboTransitionRow*> TransitionLookup;

	/// Attack definition lookup: state tag -> row
	TMap<FGameplayTag, const struct FPlayerAttackDefinitionRow*> AttackDefinitionLookup;

	/// Preloaded montages: State → Montage (populated at BeginPlay)
	TMap<FGameplayTag, TObjectPtr<class UAnimMontage>> MontageCache;

	/// Currently playing montage (nullptr when not attacking)
	TObjectPtr<class UAnimMontage> ActiveMontage;

	TOptional<FGameplayTag> BufferedInputTag;

	/// Parry -> Neutral Timer
	FTimerHandle ParryResetTimerHandle;
	static constexpr float ParryStateDuration = 1.0f;

	void OnInputRunStarted();
	void OnInputRunStopped();
	void OnInputLightAttack();
	void OnInputHeavyAttack();
	void OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted);
	void BuildTransitionLookup();
	void BuildAttackDefinitionLookup();
	void PreloadMontages();
	void ExecuteTransition(const struct FPlayerComboTransitionRow& Entry);
};
