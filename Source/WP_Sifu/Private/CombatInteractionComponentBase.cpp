// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatInteractionComponentBase.h"
#include "DeathHandlerComponentBase.h"
#include "WP_Sifu.h"
#include "Kismet/GameplayStatics.h"


FAttackPayload UCombatInteractionComponentBase::MakeCurrentAttackPayload() const
{
	FAttackPayload Payload;
	Payload.Instigator = GetOwner();
	Payload.HealthDamage = DefaultDamage;
	Payload.StructureDamage = DefaultDamage;
	return Payload;
}


EAttackResponse UCombatInteractionComponentBase::ProcessReceivedAttack(const FAttackPayload& Payload)
{
	// Ignore damage if already dead
	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		if (DeathComp->IsDead()) return EAttackResponse::Ignore;
	}

	if (DefenceState == EDefenceState::Invincible)
	{
		return EAttackResponse::Ignore;
	}

	EAttackResponse Response = ApplyDamage(Payload);

	OnAttackReceived.Broadcast(Payload, Response);
	return Response;
}


EAttackResponse UCombatInteractionComponentBase::SendAttack(AActor* Target, const FAttackPayload& Payload) const
{
	if (!Target)
	{
		LOGE(TEXT("SendAttack called with null Target"));
		return EAttackResponse::Ignore;
	}

	EAttackResponse Response = EAttackResponse::Block;

	// Send attack to target; compatible with normal actors
	if (auto Attackable = Cast<IAttackable>(Target))
	{
		Response = Attackable->ReceiveAttack(Payload);
	}
	else
	{
		auto Owner = GetOwner();
		auto Instigator = Owner ? Owner->GetInstigatorController() : nullptr;
		UGameplayStatics::ApplyDamage(Target, Payload.HealthDamage, Instigator, Owner, nullptr);
	}

	OnAttackSent.Broadcast(Target, Payload, Response);
	return Response;
}
