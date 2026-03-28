// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Attackable.generated.h"


/**
 * Attack position for matched dodge checks. Use None if not relevant.
 */
UENUM(BlueprintType)
enum class EAttackPosition : uint8
{
	None UMETA(DisplayName="None"),
	High UMETA(DisplayName="High"),
	Low UMETA(DisplayName="Low"),
};

UENUM(BlueprintType)
enum class EAttackResponse : uint8
{
	Ignore UMETA(DisplayName="Ignore"),
	Hit UMETA(DisplayName="Hit"),
	Block UMETA(DisplayName="Block"),
	Dodge UMETA(DisplayName="Dodge"),
	Parry UMETA(DisplayName="Parry"),
};


/**
 * Attack information encapsulation.
 * Will be created by the attacker and passed to IAttackable::ReceiveAttack.
 */
USTRUCT(BlueprintType)
struct FAttackPayload
{
	GENERATED_BODY()

	// --- Damage values ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Damage)
	float HealthDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Damage)
	float StructureDamage = 0.f;

	// --- Other properties ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attack)
	EAttackPosition AttackPosition = EAttackPosition::None;

	// Hit location; used by the receiver for front/back checks.
	// Use ZeroVector for attacks where location is irrelevant (e.g., bUnblockable=true).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attack)
	FVector ImpactLocation = FVector::ZeroVector;

	// Instigator of the attack, for potential use in animation, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attack)
	TWeakObjectPtr<AActor> Instigator = nullptr;

	// Whether this is a special attack (takedown, focus, etc. - always hits, cannot be blocked)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attack)
	bool bUnblockable = false;

	// HitReaction tag (Hit.Action.Stun, Hit.Action.Down, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attack)
	FGameplayTag HitReaction;
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAttackable : public UInterface
{
	GENERATED_BODY()
};


/**
 * Shared hit-receive interface for Player and Enemy.
 * Actual logic is delegated to UCombatComponent.
 */
class WP_SIFU_API IAttackable
{
	GENERATED_BODY()

public:
	// Called when receiving an attack
	virtual EAttackResponse ReceiveAttack(const FAttackPayload& AttackPayload);

	// Accessor for this Actor's CombatComponent
	virtual class UCombatComponentBase* GetCombatComponent() const = 0;
};
