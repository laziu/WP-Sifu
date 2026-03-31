// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	OneHanded UMETA(DisplayName="One Handed"),
	TwoHanded UMETA(DisplayName="Two Handed"),
};


/**
 * Equippable weapon that can be picked up or dropped in the level.
 * Collision detection is handled by the internal UAttackCollisionComponent.
 */
UCLASS(BlueprintType, Blueprintable)
class WP_SIFU_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

public: // --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon)
	TObjectPtr<class USceneComponent> Root;

	/// Weapon visual mesh (always visible)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon)
	TObjectPtr<class UStaticMeshComponent> WeaponMesh;

	/// Collision detection component. AttackTag, CollisionMeshAsset, etc. are configured in BP.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon)
	TObjectPtr<class UAttackCollisionComponent> AttackCollision;

public: // --- Configuration ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
	EWeaponType WeaponType = EWeaponType::OneHanded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
	FName AttachSocketName = TEXT("HandR");

	/// Attack definition table used by PlayerAttackComponent when this weapon is equipped.
	/// Uses the FPlayerAttackDefinitionRow struct.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
	TObjectPtr<class UDataTable> AttackDefinitionTable;

public: // --- Equip / Unequip ---
	UFUNCTION(BlueprintCallable, Category=Weapon)
	void OnEquipped(class ACharacter* OwnerChar);

	UFUNCTION(BlueprintCallable, Category=Weapon)
	void OnUnequipped();
};
