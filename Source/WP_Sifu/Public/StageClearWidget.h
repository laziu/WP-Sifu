// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageClearWidget.generated.h"

/**
 * C++ base for the stage-clear result screen widget.
 * Inherit in Blueprint to implement the translucent overlay with death count and buttons.
 */
UCLASS(Abstract, Blueprintable)
class WP_SIFU_API UStageClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Death count for this run. Set before calling PlayFadeIn. */
	UPROPERTY(BlueprintReadOnly, Category=Clear)
	int32 DeathCount = 0;

	/** Play the fade-in animation. Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category=Clear)
	void PlayFadeIn();
};
