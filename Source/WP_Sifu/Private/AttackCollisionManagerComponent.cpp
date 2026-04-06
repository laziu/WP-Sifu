// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionManagerComponent.h"

#include "AttackCollisionComponent.h"
#include "CombatInteractionComponentBase.h"
#include "GameplayTags.generated.h"
#include "Weapon/WeaponBase.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"


UAttackCollisionManagerComponent::UAttackCollisionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// ─────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────

void UAttackCollisionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	CombatComp = Owner->FindComponentByClass<UCombatInteractionComponentBase>();

	if (!CombatComp)
	{
		LOGW(TEXT("AttackCollisionManager: Owner에 CombatInteractionComponentBase가 없습니다."));
	}
}


// ─────────────────────────────────────────────
//  Registration
// ─────────────────────────────────────────────

void UAttackCollisionManagerComponent::RegisterPersistentCollision(UAttackCollisionComponent* Comp)
{
	if (!Comp) return;

	const FGameplayTag Tag = Comp->GetAttackTag();
	if (!Tag.IsValid())
	{
		LOGW(TEXT("RegisterPersistentCollision: AttackTag가 유효하지 않습니다."));
		return;
	}

	ActiveCollisionMap.Add(Tag, Comp);

	FDelegateHandle Handle = Comp->OnAttackHit.AddUObject(
		this, &UAttackCollisionManagerComponent::HandleHit);
	HitDelegateHandles.Add(Tag, Handle);
}


UAttackCollisionComponent* UAttackCollisionManagerComponent::FindAttackCollision(
	const FGameplayTag& AttackTag) const
{
	if (const auto* Found = ActiveCollisionMap.Find(AttackTag))
	{
		return *Found;
	}
	return nullptr;
}


// ─────────────────────────────────────────────
//  Weapon Equip / Unequip
// ─────────────────────────────────────────────

void UAttackCollisionManagerComponent::EquipWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return;
	if (EquippedWeapon) UnequipWeapon();

	// Attach the weapon to the character
	auto* Character = Cast<ACharacter>(GetOwner());
	Weapon->OnEquipped(Character);

	// Register the weapon collision component
	UAttackCollisionComponent* WeaponCollision = Weapon->AttackCollision;
	if (WeaponCollision)
	{
		const FGameplayTag Tag = WeaponCollision->GetAttackTag();
		ActiveCollisionMap.Add(Tag, WeaponCollision);

		FDelegateHandle Handle = WeaponCollision->OnAttackHit.AddUObject(
			this, &UAttackCollisionManagerComponent::HandleHit);
		HitDelegateHandles.Add(Tag, Handle);
	}

	EquippedWeapon = Weapon;
}

void UAttackCollisionManagerComponent::UnequipWeapon()
{
	if (!EquippedWeapon) return;

	// Unregister the weapon collision component
	UAttackCollisionComponent* WeaponCollision = EquippedWeapon->AttackCollision;
	if (WeaponCollision)
	{
		const FGameplayTag Tag = WeaponCollision->GetAttackTag();

		if (const auto* HandlePtr = HitDelegateHandles.Find(Tag))
		{
			WeaponCollision->OnAttackHit.Remove(*HandlePtr);
			HitDelegateHandles.Remove(Tag);
		}
		ActiveCollisionMap.Remove(Tag);
	}

	EquippedWeapon->OnUnequipped();
	EquippedWeapon = nullptr;
}


// ─────────────────────────────────────────────
//  Hit Handling
// ─────────────────────────────────────────────

void UAttackCollisionManagerComponent::HandleHit(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor || !CombatComp) return;

	FAttackPayload Payload = CombatComp->MakeCurrentAttackPayload();
	Payload.ImpactLocation = HitResult.ImpactPoint;
	Payload.HealthDamage = 3; // default damage
	Payload.StructureDamage = 3;
	Payload.HitReaction = GameplayTag::Hit_Reaction_None;

	CombatComp->SendAttack(HitActor, Payload);
}
