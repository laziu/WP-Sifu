// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraFocusComponent.generated.h"


/**
 * Handles auto camera focusing while combat.
 * Continuously scans for the nearest attackable target in range and keeps the camera/controller
 * rotation softly focused on that target.
 * When the player lands a hit on an enemy, focus is transferred to that enemy via SetFocusTarget().
 * Designed to work alongside UThirdPersonCameraComponent.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UCameraFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraFocusComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	/// Force focus onto a specific target (e.g., when the player lands a hit).
	UFUNCTION(BlueprintCallable, Category=Combat)
	void SetFocusTarget(AActor* NewTarget);

	/// @c true if currently focused on a target.
	UFUNCTION(BlueprintPure, Category=Combat)
	inline bool IsLockedOn() const { return TargetActor != nullptr; }

	/// Returns the currently focused target if exists.
	UFUNCTION(BlueprintPure, Category=Combat)
	inline AActor* GetTargetActor() const { return TargetActor; }

	/// The direction the character should face.
	/// If focused on a target, it's the direction to that target; otherwise, it's the camera forward.
	UFUNCTION(BlueprintPure, Category=Camera)
	FVector GetFacingDirection() const;

protected:
	/// Maximum distance to search for auto-focus targets.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusRadius = 1500.;

	/// Maximum angle (half-cone, degrees) from forward vector to consider a target.
	/// Default 90° covers the entire front hemisphere.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusAngle = 90.;

	/// Interpolation speed for rotating toward the target.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double InterpSpeed = 10.;

	/// Release focus if target exceeds this distance.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double BreakDistance = 2000.;

private:
	/// Find the best auto-focus target based on distance and angle.
	AActor* FindBestTarget() const;

	/// Update controller rotation toward the focused target.
	void UpdateRotationToTarget(float DeltaTime);

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
};
