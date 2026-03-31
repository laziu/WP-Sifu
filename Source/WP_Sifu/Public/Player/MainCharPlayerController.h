// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainCharPlayerController.generated.h"


/**
 */
UCLASS()
class WP_SIFU_API AMainCharPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainCharPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputMappingContext> InputMappingDefault;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputMappingContext> InputMappingCombat;
};
