// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainStageGameMode.generated.h"

/**
 * GameMode for Lvl_MainStage.
 * Manages the intro cinematic sequence, enemy death tracking, and stage-clear handling.
 */
UCLASS()
class WP_SIFU_API AMainStageGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainStageGameMode();

	/** Called by the Restart button in WBP_StageClear. */
	UFUNCTION(BlueprintCallable, Category=Stage)
	void RestartStage();

	/** Called by the Title button in WBP_StageClear. */
	UFUNCTION(BlueprintCallable, Category=Stage)
	void ReturnToTitle();

protected:
	virtual void BeginPlay() override;

	/** Widget class for the intro overlay (black bars + best record). */
	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class UStageIntroWidget> StageIntroWidgetClass;

	/** Widget class for the stage-clear result screen. */
	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class UStageClearWidget> StageClearWidgetClass;

	/**
	 * Cinematic camera placed in the level for the intro shot.
	 * If left null, auto-discovered by the "IntroCamera" actor tag.
	 */
	UPROPERTY(EditInstanceOnly, Category=Intro)
	TObjectPtr<class ACineCameraActor> IntroCameraActor;

	/** EaseInOut exponent for the player camera blend. */
	UPROPERTY(EditDefaultsOnly, Category=Intro)
	float CameraBlendExponent = 2.f;

	/** Seconds the intro shot is held before the camera transitions to the player. */
	UPROPERTY(EditDefaultsOnly, Category=Intro)
	float IntroDurationSeconds = 4.f;

	/** Duration (seconds) of the camera blend from intro cam to player cam. */
	UPROPERTY(EditDefaultsOnly, Category=Intro)
	float CameraBlendSeconds = 4.f;

private:
	// --- Intro sequence ---
	void StartIntroSequence();
	void BeginCameraTransition();
	void OnIntroComplete();

	// --- Enemy tracking ---
	UFUNCTION()
	void OnEnemyDied();

	// --- Stage end ---
	void OnStageClear();

	/** Saves the best record, shows the loading screen, and opens LevelName. */
	void TransitionToLevel(FName LevelName);

	// --- State ---
	int32 AliveEnemyCount = 0;

	FTimerHandle IntroDurationTimer;
	FTimerHandle CameraBlendTimer;

	UPROPERTY()
	TObjectPtr<class UStageIntroWidget> StageIntroWidgetInstance;
};
