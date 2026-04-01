// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "AttackCollisionComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttackHit, AActor* /*HitActor*/, const FHitResult& /*HitResult*/);


/**
 * Reusable component that performs collision detection for attack sources.
 * It can be attached directly to the character's hand/foot sockets or used
 * as a subcomponent of AWeaponBase.
 *
 * When a StaticMesh with Simple Collision is assigned to CollisionMeshAsset,
 * ComponentSweepMulti is performed using that collision shape.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UAttackCollisionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAttackCollisionComponent();

public: // --- Configuration ---
	/// Tag that identifies this attack source (Attack.Source.Hand.R, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Collision)
	FGameplayTag AttackTag;

	/// StaticMesh with Simple Collision configured. Sweeps use this mesh's collision shape.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Collision)
	TObjectPtr<UStaticMesh> CollisionMeshAsset;

	/// Trace channel used for sweeps.
	/// Set this after adding the WeaponTrace channel in Project Settings.
	/// Default: ECC_GameTraceChannel1 (when WeaponTrace is the first custom channel) 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Collision)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

	/// Whether to show the collision mesh in the editor. Always hidden at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Debug)
	bool bShowCollisionMeshInEditor = true;

public: // --- Public Methods ---
	/// Starts the sweep, clears hit tracking, and initializes PreviousLocation.
	UFUNCTION(BlueprintCallable, Category=Collision)
	void ActivateTrace();

	/// Stops the sweep.
	UFUNCTION(BlueprintCallable, Category=Collision)
	void DeactivateTrace();

	UFUNCTION(BlueprintPure, Category=Collision)
	bool IsTraceActive() const { return bTraceActive; }

	FGameplayTag GetAttackTag() const { return AttackTag; }

public: // --- Events ---
	/// Broadcast when an attack hit occurs. Subscribed to by AttackCollisionManagerComponent.
	FOnAttackHit OnAttackHit;

protected:
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/// Internal subcomponent. Uses the CollisionMeshAsset shape and stays hidden at runtime.
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CollisionMeshComp;

	bool bTraceActive = false;
	FVector PreviousLocation = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	void PerformSweep();
	void ApplyCollisionMesh();
	void ApplyEditorVisibility();
};
