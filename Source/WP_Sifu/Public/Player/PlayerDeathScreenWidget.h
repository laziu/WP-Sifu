// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerDeathScreenWidget.generated.h"

/**
 * Base C++ class for the player death screen widget.
 * Create a UMG Blueprint inheriting from this class for visuals.
 * All data and flow is driven from C++ via Show() / the OnFinished delegate.
 */
UCLASS(Abstract, Blueprintable)
class WP_SIFU_API UPlayerDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Called from C++ to initialize and start the death screen sequence.
	 * Override PlayDeathSequence in Blueprint to implement the visual animation.
	 */
	void Show(class UPlayerDeathHandlerComponent* InOwnerComponent, int32 InDeathCount);

protected:
	/** Override in Blueprint to play the aging/death animation sequence.
	 *  Call FinishDeathSequence() when the animation is done. */
	UFUNCTION(BlueprintImplementableEvent, Category=Death)
	void PlayDeathSequence();

	/** Call this from Blueprint when the death screen animation is finished. */
	UFUNCTION(BlueprintCallable, Category=Death)
	void FinishDeathSequence();

	UPROPERTY(BlueprintReadOnly, Category=Death)
	int32 DeathCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Death)
	TObjectPtr<class UPlayerDeathHandlerComponent> OwnerDeathHandler;
};
