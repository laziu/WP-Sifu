// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interface/Attackable.h"
#include "AttackCollisionManagerComponent.generated.h"


/**
 * Manages the character's UAttackCollisionComponent instances for all attack
 * sources (unarmed + weapon) by AttackTag and routes damage handling on hit.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UAttackCollisionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackCollisionManagerComponent();

public: // --- Public Methods ---
	/// Finds the CollisionComponent mapped to AttackTag from an AnimNotifyState.
	UFUNCTION(BlueprintCallable, Category=Combat)
	class UAttackCollisionComponent* FindAttackCollision(const FGameplayTag& AttackTag) const;

	/// Registers unarmed collision components (hands/feet) by AttackTag.
	/// Called for each component from AMainCharacter::BeginPlay.
	UFUNCTION(BlueprintCallable, Category=Combat)
	void RegisterPersistentCollision(class UAttackCollisionComponent* Comp);

	UFUNCTION(BlueprintCallable, Category=Weapon)
	void EquipWeapon(class AWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category=Weapon)
	void UnequipWeapon();

	UFUNCTION(BlueprintPure, Category=Weapon)
	bool HasWeaponEquipped() const { return EquippedWeapon != nullptr; }

protected:
	virtual void BeginPlay() override;

private: // --- External References (cached in BeginPlay) ---
	UPROPERTY()
	TObjectPtr<class UCombatInteractionComponentBase> CombatComp;

private: // --- State ---
	UPROPERTY()
	TObjectPtr<class AWeaponBase> EquippedWeapon;

	/// AttackTag -> currently active UAttackCollisionComponent
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<class UAttackCollisionComponent>> ActiveCollisionMap;

	/// OnAttackHit subscription handles for registered persistent collisions
	TMap<FGameplayTag, FDelegateHandle> HitDelegateHandles;

private: // --- Hit Stop ---
	UPROPERTY(EditAnywhere, Category = "Hit Stop")
	bool bEnableHitStop = true;

	UPROPERTY(EditAnywhere, Category = "Hit Stop", meta = (EditCondition = "bEnableHitStop"))
	float HitStopTimeDilation = 0.02f;

	UPROPERTY(EditAnywhere, Category = "Hit Stop", meta = (EditCondition = "bEnableHitStop"))
	float HitStopDuration = 0.06f;

	FTimerHandle HitStopTimerHandle;

	void ApplyHitStop();

private: // --- Internal ---
	void HandleHit(AActor* HitActor, const FHitResult& HitResult);
};
