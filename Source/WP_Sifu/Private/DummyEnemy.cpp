// Fill out your copyright notice in the Description page of Project Settings.


#include "DummyEnemy.h"

#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.h"
#include "DummyCombatInteractionComponent.h"
#include "UserExtension.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"


ADummyEnemy::ADummyEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	// Capsule root for WeaponTrace collision
	EXT_CREATE_DEFAULT_SUBOBJECT(CapsuleComp, TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(34.f, 88.f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CapsuleComp;

	// Visual mesh (assign in Blueprint or editor)
	EXT_CREATE_DEFAULT_SUBOBJECT(MeshComp, TEXT("Mesh"));
	MeshComp->SetupAttachment(CapsuleComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Ability system
	EXT_CREATE_DEFAULT_SUBOBJECT(AbilitySystemComp, TEXT("AbilitySystem"));
	EXT_CREATE_DEFAULT_SUBOBJECT(HealthAttribs, TEXT("HealthAttributes"));
	EXT_CREATE_DEFAULT_SUBOBJECT(CombatInteractionComp, TEXT("CombatInteraction"));

	// Initialize attributes
	HealthAttribs->InitHealth(MaxHealth);
	HealthAttribs->InitMaxHealth(MaxHealth);
	HealthAttribs->InitStructure(0.f);
	HealthAttribs->InitMaxStructure(MaxStructure);
}

void ADummyEnemy::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComp->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ADummyEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

UCombatInteractionComponentBase* ADummyEnemy::GetCombatInteractionComponent() const
{
	return CombatInteractionComp;
}
