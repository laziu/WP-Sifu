// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IInputBindable.h"
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
	virtual void BeginPlay() override;

public:
	virtual void SetupInputBindings(class UEnhancedInputComponent* EnhancedInputComponent) override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement")
	double WalkSpeed = 150.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement")
	double RunSpeed = 300.;

private:
	void OnInputMove(const struct FInputActionValue& Value);
	void OnInputLook(const struct FInputActionValue& Value);
	void OnInputRunStarted();
	void OnInputRunStopped();

private:
	/// Set move speed
	void SetOwnerWalkSpeed(double NewSpeed) const;
};
