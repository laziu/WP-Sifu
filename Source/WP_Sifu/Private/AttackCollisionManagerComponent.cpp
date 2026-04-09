// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionManagerComponent.h"

#include "AttackCollisionComponent.h"
#include "CombatInteractionComponentBase.h"
#include "GameplayTags.generated.h"
#include "Weapon/WeaponBase.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


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

	EAttackResponse Response = CombatComp->SendAttack(HitActor, Payload);

	if (Response == EAttackResponse::Hit
		|| Response == EAttackResponse::Block
		|| Response == EAttackResponse::Parry)
	{
		ApplyHitStop();
	}
}


void UAttackCollisionManagerComponent::ApplyHitStop()
{
	if (!bEnableHitStop) return;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(HitStopTimerHandle);

	UGameplayStatics::SetGlobalTimeDilation(this, HitStopTimeDilation);

	FTimerDelegate Delegate;
	Delegate.BindWeakLambda(this, [this]()
	{
		UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	});

	// Timer runs in dilated game-time, so compensate duration
	const float AdjustedDuration = HitStopDuration * HitStopTimeDilation;
	World->GetTimerManager().SetTimer(
		HitStopTimerHandle, Delegate, AdjustedDuration, false);
}
