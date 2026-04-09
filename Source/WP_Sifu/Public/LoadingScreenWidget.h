// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

/**
 * C++ base for the loading screen widget.
 * Inherit in Blueprint to implement visual animations.
 */
UCLASS(Abstract, Blueprintable)
class WP_SIFU_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Play fade-in animation. Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category=Loading)
	void PlayFadeIn();

	/** Play fade-out animation. Override in Blueprint. Call NotifyFadeOutComplete() when done. */
	UFUNCTION(BlueprintImplementableEvent, Category=Loading)
	void PlayFadeOut();

	/** Call this from Blueprint at the end of the fade-out animation to remove the widget. */
	UFUNCTION(BlueprintCallable, Category=Loading)
	void NotifyFadeOutComplete();
};
