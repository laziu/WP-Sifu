// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraFocusComponent.generated.h"


/**
 * Handles target lock-on while combat.
 * Finds the nearest attackable target in range and keeps the camera/controller rotation focused on that target. 
 * Designed to work alongside UThirdPersonCameraComponent.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UCameraFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraFocusComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/// Find and focus to the nearest target, or release if already exists.
	UFUNCTION(BlueprintCallable, Category=Combat)
	bool Focus();

	UFUNCTION(BlueprintCallable, Category=Combat)
	bool ReleaseFocus();

	/// @c true if currently focused on a target.
	UFUNCTION(BlueprintPure, Category=Combat)
	inline bool IsLockedOn() const { return TargetActor != nullptr; }

	/// Returns the currently locked target if exists.
	UFUNCTION(BlueprintPure, Category=Combat)
	inline AActor* GetTargetActor() const { return TargetActor; }

	/// Switch to the next target (left/right).
	UFUNCTION(BlueprintCallable, Category=Combat)
	void SwitchTarget(bool bToRight);

	/// The direction the character should face.
	/// If locked on, it's the direction to the target; otherwise, it's the character's forward vector.
	UFUNCTION(BlueprintPure, Category=Camera)
	FVector GetFacingDirection() const;

protected:
	virtual void BeginPlay() override;

	/// Maximum distance to search for focusing targets.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusRadius = 1500.;

	/// Maximum angle (half-cone, degrees) from forward vector to consider a target.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusAngle = 45.;

	/// Interpolation speed for rotating toward the target.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double InterpSpeed = 10.;

	/// Release focus if target exceeds this distance.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double BreakDistance = 2000.;

private:
	/// Find the best target based on distance and angle.
	UFUNCTION(BlueprintCallable, Category=Combat)
	AActor* FindBestTarget() const;

	/// Find the next target to the left/right of the current target, based on distance and angle.
	UFUNCTION(BlueprintCallable, Category=Combat)
	AActor* FindNextTarget(bool bToRight) const;

	/// Update controller rotation toward the locked target.
	void UpdateRotationToTarget(float DeltaTime);

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
};
