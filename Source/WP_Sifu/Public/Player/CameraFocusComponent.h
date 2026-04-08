// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBindable.h"
#include "CameraFocusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatStanceChangedEvent, bool, bInCombatStance);

/**
 * Handles auto camera focusing, combat stance detection, dynamic zoom-out, and
 * world-stabilised SocketOffset adjustments.
 * Designed to work alongside UThirdPersonCameraComponent.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UCameraFocusComponent : public UActorComponent, public IInputBindable
{
	GENERATED_BODY()

public:
	UCameraFocusComponent();

	virtual void BeginPlay() override;
	virtual void SetupInputBindings(class UEnhancedInputComponent* EIC) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public: // --- Focus Target ---
	/// Force focus onto a specific target (e.g., when the player lands a hit).
	UFUNCTION(BlueprintCallable, Category=Combat, meta=(AllowAbstract="false"))
	void SetFocusTarget(UPARAM(meta=(MustImplement="Attackable")) AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category=Combat)
	inline bool IsLockedOn() const { return TargetActor != nullptr; }

	UFUNCTION(BlueprintPure, Category=Combat)
	inline AActor* GetTargetActor() const { return TargetActor; }

	/// The direction the character should face.
	UFUNCTION(BlueprintPure, Category=Camera)
	FVector GetFacingDirection() const;

public: // --- Combat Stance ---
	UFUNCTION(BlueprintPure, Category=Combat)
	bool IsInCombatStance() const { return bInCombatStance; }

	/// Fired when combat stance changes.
	UPROPERTY(BlueprintAssignable, Category=Combat)
	FCombatStanceChangedEvent OnCombatStanceChanged;

protected: // --- Input ---
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<class UInputAction> InputMove;

protected: // --- Focus Config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusRadius = 500.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double FocusAngle = 90.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Focus)
	double InterpSpeed = 10.;

protected: // --- Combat Stance Config ---
	/// Time (seconds) after enemies disappear before combat stance is released.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat Stance")
	float CombatStanceCooldown = 1.f;

protected: // --- Zoom Config ---
	/// Base arm length (no enemies).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Zoom)
	double BaseArmLength = 120.;

	/// Additional arm length per nearby enemy.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Zoom)
	double PerEnemyZoomOut = 30.;

	/// Additional arm length multiplied by enemy spread radius.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Zoom)
	double SpreadZoomFactor = 0.15;

	/// Smooth time for zoom interpolation – controls how quickly arm length reaches the
	/// target (seconds to reach target under ideal conditions; lower = snappier).
	/// Produces ease-in / ease-out motion via a critically-damped spring.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Zoom)
	double ZoomSmoothTime = 0.4;

	/// Maximum arm-length change per second (cm/s).  Caps the peak velocity so the
	/// camera never jumps too fast even when the target suddenly changes a lot.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Zoom)
	double ZoomMaxSpeed = 50.;

protected: // --- SocketOffset Config ---
	/// Maximum absolute Y offset (cm). Non-combat targets ±this value.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Socket Offset")
	double MaxSocketOffsetY = 60.;

	/// Smooth time for non-combat shoulder return (controls ease-in-out shape, seconds).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Socket Offset")
	double OffsetSmoothTime = 0.4;

	/// Max Y speed when returning to shoulder outside combat (cm/s).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Socket Offset")
	double OffsetMaxSpeed = 100.;

	/// Speed (cm/s) at which Y drifts opposite to strafe input during combat.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Socket Offset")
	double CombatOffsetDriftSpeed = 100.;

	/// How quickly the drift eases in when strafe input begins (higher = snappier).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Socket Offset")
	double DriftEaseSpeed = 4.;

private: // --- Focus ---
	AActor* FindBestTarget() const;
	void UpdateRotationToTarget(float DeltaTime);

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

private: // --- Combat Stance ---
	void UpdateCombatStance();
	void SetCombatStanceInternal(bool bNewValue);

	bool bInCombatStance = false;
	FTimerHandle CombatCooldownTimer;

private: // --- Nearby Enemies ---
	TArray<AActor*> FindNearbyEnemies() const;
	/// Removes outliers from an enemy list based on distance standard deviation.
	void FilterOutliers(TArray<AActor*>& Enemies) const;

private: // --- Zoom ---
	void UpdateZoom(const TArray<AActor*>& NearbyEnemies, float DeltaTime);

	/// Current arm-length velocity used by the smooth-damp integrator.
	double ZoomVelocity = 0.;

private: // --- SocketOffset ---
	void UpdateSocketOffset(float DeltaTime);
	void OnInputMove(const struct FInputActionValue& Value);
	void OnInputMoveStopped();

	float MoveInputY = 0.f;
	double SmoothedStrafeInput = 0.;

	/// Current velocity of the Y offset, used by the smooth-damp integrator.
	double OffsetVelocity = 0.;
};
