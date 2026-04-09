// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WP_GameInstance.generated.h"

/**
 * Global game instance: persists best-death-count and manages the loading screen widget
 * across level transitions.
 */
UCLASS()
class WP_SIFU_API UWP_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

	/**
	 * Show the loading screen.
	 * @param bWithFadeIn  true: fade in from transparent (title → stage).
	 *                     false: appear instantly opaque (stage-clear → restart).
	 */
	UFUNCTION(BlueprintCallable, Category=Loading)
	void ShowLoadingScreen(bool bWithFadeIn = true);

	/** Fade out and remove the loading screen. */
	UFUNCTION(BlueprintCallable, Category=Loading)
	void HideLoadingScreen();

	/**
	 * Update the best (lowest) death count and immediately save.
	 * Ignored when NewDeathCount >= current best.
	 */
	UFUNCTION(BlueprintCallable, Category=Record)
	void UpdateBestRecord(int32 NewDeathCount);

	/** Returns INT32_MAX if no record exists. */
	UFUNCTION(BlueprintCallable, Category=Record)
	int32 GetBestDeathCount() const { return BestDeathCount; }

	/** Blueprint-settable class for the loading screen widget. */
	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class ULoadingScreenWidget> LoadingWidgetClass;

private:
	void CreateAndShowLoadingWidget(bool bPlayFadeIn);

	int32 BestDeathCount = INT32_MAX;

	/** True while a level is loading so OnWorldChanged re-creates the widget. */
	bool bLoadingScreenPending = false;

	UPROPERTY()
	TObjectPtr<class ULoadingScreenWidget> LoadingWidgetInstance;
};
