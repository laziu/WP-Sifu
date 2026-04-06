// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBindable.h"
#include "PlayerMoveComponent.generated.h"


/**
 * Holds references to Input Actions and binds them to the owning Character.
 * Responsible for Move (camera-relative) and Look input handling.
 */
UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerMoveComponent : public UActorComponent, public IInputBindable
{
	GENERATED_BODY()

public:
	UPlayerMoveComponent();

protected:
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetupInputBindings(class UEnhancedInputComponent* EIC) override;

public:
	/// Whether the character is currently in combat stance.
	/// Driven externally by CameraFocusComponent / PlayerAttackComponent.
	UFUNCTION(BlueprintCallable, Category="Character|Movement")
	void SetCombatStance(bool bNewCombatStance);

	UFUNCTION(BlueprintPure, Category="Character|Movement")
	bool IsInCombatStance() const { return bInCombatStance; }

	UFUNCTION(BlueprintPure, Category="Character|Movement")
	bool IsRunning() const { return bIsRunning; }

protected:
	/// Move input action (WASD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputMove;

	/// Look input action (Mouse Delta)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputLook;

	/// Run input action (Shift)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character|Movement")
	double NormalSpeed = 300.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character|Movement")
	double CombatSpeed = 150.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character|Movement")
	double RunSpeed = 500.;

	/// Interpolation speed for rotating toward focus target during combat stance.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character|Movement")
	double CombatRotationInterpSpeed = 10.;

private:
	void OnInputMove(const struct FInputActionValue& Value);
	void OnInputLook(const struct FInputActionValue& Value);
	void OnInputRunStarted();
	void OnInputRunStopped();

	void UpdateMovementSpeed();
	void UpdateCombatRotation(float DeltaTime);

	bool bIsRunning = false;
	bool bInCombatStance = false;
};
