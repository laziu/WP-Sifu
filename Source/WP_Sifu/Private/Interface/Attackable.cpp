// Fill out your copyright notice in the Description page of Project Settings.


#include "Attackable.h"

#include "CombatComponentBase.h"

// Add default functionality here for any IAttackable functions that are not pure virtual.

EAttackResponse IAttackable::ReceiveAttack(const FAttackPayload& AttackPayload)
{
	return GetCombatComponent()->ProcessReceivedAttack(AttackPayload);
}
