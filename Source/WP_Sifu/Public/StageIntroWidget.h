// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageIntroWidget.generated.h"

/**
 * C++ base for the stage intro (cinematic) overlay widget.
 * Inherit in Blueprint to implement black bars and best-record display.
 */
UCLASS(Abstract, Blueprintable)
class WP_SIFU_API UStageIntroWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set before calling PlayIntroAnim. INT32_MAX means no record exists. */
	UPROPERTY(BlueprintReadOnly, Category=Intro)
	int32 BestDeathCount = INT32_MAX;

	/** Play the record fade-in + black-bar appear animation. Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category=Intro)
	void PlayIntroAnim();

	/** Play the record fade-out + black-bar slide-out animation (2s). Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category=Intro)
	void PlayOutroAnim();
};
