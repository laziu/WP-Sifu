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

	/// Current attack state GameplayTag (Attack.Type.L2, Attack.Type.Parry.L, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CurrentState;

	/// Next attack state GameplayTag to transition to if Input is received during the transition window.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NextState;

	/// Input tag (Combat.Input.Light, Combat.Input.Heavy, etc.) that triggers the transition if received during the transition window.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Input;
};
