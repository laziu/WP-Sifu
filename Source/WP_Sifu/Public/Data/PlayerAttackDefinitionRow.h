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

	/// Attack state GameplayTag (Attack.Type.L2, Attack.Type.Parry.L, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag State;

	/// Basic damage (current: HealthDamage = StructureDamage = Damage)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double Damage = 0.;

	/// Hit reaction GameplayTag (Hit.Reaction.Stun, Hit.Reaction.Down, etc.)
	/// blank = normal hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Reaction;

	/// Attack montage name to play when this attack is executed
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;
};
