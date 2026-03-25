// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInputComponent.generated.h"


/**
 * Holds references to Input Actions and binds them to the owning Character.
 * Responsible for Move (camera-relative) and Look input handling.
 */
UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInputComponent();

protected:
	virtual void BeginPlay() override;

public:
	/// Bind all input actions to the given EnhancedInputComponent
	void SetupInputBindings(class UEnhancedInputComponent* EnhancedInputComponent);

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

	/// Light Attack input action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputLightAttack;

	/// Heavy Attack input action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputHeavyAttack;

	/// Block input action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputBlock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement")
	double WalkSpeed = 150.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement")
	double RunSpeed = 300.;

private:
	void OnInputMove(const struct FInputActionValue& Value);
	void OnInputLook(const struct FInputActionValue& Value);
	void OnInputRunStarted();
	void OnInputRunStopped();
	void OnInputLightAttack();
	void OnInputHeavyAttack();
	void OnInputBlockStarted();
	void OnInputBlockStopped();

private:
	/// Set move speed
	void SetOwnerWalkSpeed(double NewSpeed) const;
};
