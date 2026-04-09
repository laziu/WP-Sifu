// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidget.generated.h"

/**
 * C++ base for the title screen widget.
 * Inherit in Blueprint to implement the title layout and fade-out animation.
 */
UCLASS(Abstract, Blueprintable)
class WP_SIFU_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Play the title fade-out animation (0.5s). Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category=Title)
	void PlayFadeOut();
};
