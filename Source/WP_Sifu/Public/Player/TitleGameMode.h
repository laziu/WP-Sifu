// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

/**
 * GameMode for Lvl_Title.
 * Creates the title widget and handles the Start / Quit button actions.
 */
UCLASS()
class WP_SIFU_API ATitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATitleGameMode();

	/** Called by the Start button in WBP_Title. */
	UFUNCTION(BlueprintCallable, Category=Title)
	void RequestStartGame();

	/** Called by the Quit button in WBP_Title. */
	UFUNCTION(BlueprintCallable, Category=Title)
	void RequestQuitGame();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class UTitleWidget> TitleWidgetClass;

private:
	/** Fires after the 0.5s fade-out completes; shows loading screen and opens the main stage. */
	void OnFadeOutTimerComplete();

	FTimerHandle FadeOutTimerHandle;

	UPROPERTY()
	TObjectPtr<class UTitleWidget> TitleWidgetInstance;
};
