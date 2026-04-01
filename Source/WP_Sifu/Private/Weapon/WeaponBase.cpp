// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponBase.h"

#include "AttackCollisionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"


AWeaponBase::AWeaponBase()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	AttackCollision = CreateDefaultSubobject<UAttackCollisionComponent>(TEXT("AttackCollision"));
	AttackCollision->SetupAttachment(Root);
}

void AWeaponBase::OnEquipped(ACharacter* OwnerChar)
{
	if (!OwnerChar) return;

	SetOwner(OwnerChar);
	AttachToComponent(
		OwnerChar->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName);
}

void AWeaponBase::OnUnequipped()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
}
