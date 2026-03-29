// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PlayerComboTransitionRow.generated.h"

USTRUCT(BlueprintType)
struct FPlayerComboTransitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CurrentStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NextStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ActionTag;
};
