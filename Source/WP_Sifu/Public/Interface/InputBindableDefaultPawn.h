// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "InputBindableDefaultPawn.generated.h"

UCLASS()
class WP_SIFU_API AInputBindableDefaultPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
