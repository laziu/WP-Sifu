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

	/// Bind all input actions to the given EnhancedInputComponent
	void SetupInputBindings(class UEnhancedInputComponent* EnhancedInputComponent);

protected:
	/// Move input action (WASD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputMove;

	/// Look input action (Mouse Delta)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	TObjectPtr<class UInputAction> InputLook;

private:
	void OnInputMove(const struct FInputActionValue& Value);
	void OnInputLook(const struct FInputActionValue& Value);
};
