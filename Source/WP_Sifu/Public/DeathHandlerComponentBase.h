// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeathHandlerComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathFinished);

/**
 * Base component that handles death when Health reaches 0.
 * Subclass for player (revive) or enemy (permanent death) behavior.
 */
UCLASS(Abstract, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UDeathHandlerComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category=Death)
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category=Death)
	FOnDeathStarted OnDeathStarted;

	UPROPERTY(BlueprintAssignable, Category=Death)
	FOnDeathFinished OnDeathFinished;

protected:
	virtual void BeginPlay() override;

	/** Called when HealthAttributeSet::OnHealthDepleted fires */
	UFUNCTION()
	void HandleHealthDepleted();

	/** Called when the death montage finishes playing */
	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Subclasses implement post-death-montage behavior */
	virtual void OnDeathComplete() {}

	/** Called after bIsDead is set, before death montage plays. Subclass hook. */
	virtual void OnDeathBegin() {}

	UPROPERTY(EditDefaultsOnly, Category=Death)
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Death)
	bool bIsDead = false;
};
