// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathHandlerComponentBase.h"
#include "EnemyDeathHandlerComponent.generated.h"

/**
 * Enemy death handler: permanently disables the enemy after death montage.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UEnemyDeathHandlerComponent : public UDeathHandlerComponentBase
{
	GENERATED_BODY()

public:
	UEnemyDeathHandlerComponent();

protected:
	virtual void OnDeathBegin() override;
	virtual void OnDeathComplete() override;
};
