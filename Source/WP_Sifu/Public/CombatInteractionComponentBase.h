// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attackable.h"
#include "CombatInteractionComponentBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FAttackRecvEvent,
	const FAttackPayload&, Payload, EAttackResponse, Response);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FAttackSendEvent,
	AActor*, Target, const FAttackPayload&, Payload, EAttackResponse, Response);


// 
UENUM(BlueprintType)
enum class EDefenceState : uint8
{
	None UMETA(DisplayName="None"),
	Blocking UMETA(DisplayName="Blocking"),
	Dodging UMETA(DisplayName="Dodging"),
	Parrying UMETA(DisplayName="Parrying"),
	Invincible UMETA(DisplayName="Invincible"),
};

/**
 * Player/Enemy common combat component.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent), Blueprintable)
class WP_SIFU_API UCombatInteractionComponentBase : public UActorComponent
{
	GENERATED_BODY()

public: // --- Interactions ---
	/// Receive attack; called from IAttackable::ReceiveAttack
	UFUNCTION(BlueprintCallable, Category=Combat, meta=(ReturnDisplayName = "Response"))
	EAttackResponse ProcessReceivedAttack(const FAttackPayload& Payload);

	/// Send attack to target Actor (calls ReceiveAttack if target implements IAttackable)
	UFUNCTION(BlueprintCallable, Category=Combat, meta=(ReturnDisplayName = "Response"))
	EAttackResponse SendAttack(AActor* Target, const FAttackPayload& Payload) const;

public: // --- Events ---
	UPROPERTY(BlueprintAssignable)
	FAttackRecvEvent OnAttackReceived;

	UPROPERTY(BlueprintAssignable)
	FAttackSendEvent OnAttackSent;

protected: // --- Internal logic ---
	/// Apply damage to attributes
	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) { return K2_ApplyDamage(Payload); }

	UFUNCTION(BlueprintImplementableEvent, Category=Combat, meta=(DisplayName="Apply Damage"))
	EAttackResponse K2_ApplyDamage(const FAttackPayload& Payload);

public: // --- Combat state ---
	UFUNCTION(BlueprintCallable, Category=Combat)
	void SetDefenceState(EDefenceState NewState) { DefenceState = NewState; }

	UFUNCTION(BlueprintCallable, Category=Combat)
	EDefenceState GetDefenceState() const { return DefenceState; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	EDefenceState DefenceState = EDefenceState::None;
};
