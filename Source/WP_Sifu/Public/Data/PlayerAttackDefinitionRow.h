// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"
#include "PlayerAttackDefinitionRow.generated.h"

USTRUCT(BlueprintType)
struct FPlayerAttackDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이 공격 또는 전투 상태에 대응하는 GameplayTag
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag 