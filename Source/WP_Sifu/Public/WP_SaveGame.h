// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WP_SaveGame.generated.h"

/**
 * Persists the player's best (lowest) death count across sessions.
 */
UCLASS()
class WP_SIFU_API UWP_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SaveSlotName;
	static const int32 SaveUserIndex = 0;

	/** Lowest death count ever achieved. INT32_MAX means never completed. */
	UPROPERTY(SaveGame)
	int32 BestDeathCount = INT32_MAX;
};
