// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MainCharCameraManager.generated.h"

/**
 * Sets Pitch limits and provides a hook for future combat camera transitions / camera shakes.
 */
UCLASS()
class WP_SIFU_API AMainCharCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AMainCharCameraManager();
};
