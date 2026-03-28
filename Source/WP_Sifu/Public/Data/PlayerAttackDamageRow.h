// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"
#include "PlayerAttackDamageRow.generated.h"

USTRUCT(BlueprintType)
struct FPlayerAttackDamageRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이 공격 또는 전투 상태에 대응하는 GameplayTag
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag StateTag;

	// 기본 데미지 (HealthDamage = StructureDamage = Damage)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double Damage = 0.;

	// 히트 리액션 GameplayTag (Hit.Action.Stun, Hit.Action.Down 등)
	// 빈 값 = 일반 히트
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ResultTag;

	// 이 공격 상태에서 재생할 개별 AnimMontage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;
};
