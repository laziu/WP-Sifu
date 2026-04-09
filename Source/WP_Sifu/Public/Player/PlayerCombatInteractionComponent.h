// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatInteractionComponentBase.h"
#include "GameplayTagContainer.h"
#include "InputBindable.h"
#include "PlayerCombatInteractionComponent.generated.h"


UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
	None UMETA(DisplayName="None"),
	Hit UMETA(DisplayName="Hit"),
	BlockHit UMETA(DisplayName="BlockHit"),
	Broken UMETA(DisplayName="Broken"),
};


UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerCombatInteractionComponent : public UCombatInteractionComponentBase, public IInputBindable
{
	GENERATED_BODY()

public:
	UPlayerCombatInteractionComponent();

	virtual void SetupInputBindings(class UEnhancedInputComponent* EIC) override;
	virtual FAttackPayload MakeCurrentAttackPayload() const override;

public: /// --- Block input handlers ---
	UFUNCTION(BlueprintCallable, Category=Combat)
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category=Combat)
	void StopBlock();

public: /// --- AnimNotify callbacks (called by AnimNotify classes) ---
	void OnParryWindowBegin();
	void OnParryWindowEnd();
	void OnDodgeActiveBegin();
	void OnDodgeActiveEnd();
	void OnDodgeCooldownEnd();
	void OnHitReactionEnd();

public: /// --- Movement blocking ---
	UFUNCTION(BlueprintPure, Category=Combat)
	bool IsMovementBlocked() const;

public: /// --- ABP-readable state ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Animation)
	EHitReactionType HitReactionType = EHitReactionType::None;

	/// Hit direction in character-local space (X=Right, Y=Forward) for blend space
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Animation)
	FVector2D HitDirection = FVector2D::ZeroVector;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputBlock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputMove;

private:
	/// Check hit direction based on ImpactLocation and player's forward vector
	bool IsAttackFromBehind(const FVector& ImpactLocation) const;

	/// Dodging/Parrying 시 Structure 데미지에 곱해지는 비율 (임시)
	static constexpr float DeflectStructureRate = -2.f;

	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;

private: // --- Parry/Block ---
	static constexpr float BlockCooldown = 0.25f;

	bool bBlockKeyHeld = false;
	bool bCanBlock = true;
	FTimerHandle BlockCooldownTimer;
	bool bHitRecoveryMovementLock = false;
	FTimerHandle HitRecoveryTimer;
	static constexpr float HitRecoveryLockDuration = 0.2f;

	FVector2D ComputeHitDirection(const FVector& ImpactLocation) const;
	void SetHitReaction(EHitReactionType Type, const FVector& ImpactLocation);

private: // --- Montages ---
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> BlockStartMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> DodgeUpMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> DodgeDownMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> BlockHitMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> StructureBrokenMontage;

	bool PlayDefenceMontage(class UAnimMontage* Montage);

private: // --- Dodge ---
	static constexpr float DodgeCooldown = 0.15f;

	bool bCanDodge = true;
	FTimerHandle DodgeCooldownTimer;

	void TryBlockDodge(const struct FInputActionValue& Value);
	void ExecuteBlockDodge(FGameplayTag DodgeStateTag);

private: // --- Late Input Buffer ---
	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(ClampMin="0"))
	float LateInputWindow = 0.1f;

	TOptional<FAttackPayload> PendingHitPayload;
	FTimerHandle PendingHitTimer;

	void ApplyPendingHitDamage();

private: // --- Structure Broken ---
	bool bStructureBroken = false;

	UFUNCTION()
	void HandleStructureChanged(UAttributeSet* AttributeSet, float OldValue, float NewValue);

	void OnStructureBroken();

private:
	UFUNCTION()
	void ResetCombatState();
};
