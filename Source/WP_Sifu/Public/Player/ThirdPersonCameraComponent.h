// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThirdPersonCameraComponent.generated.h"


/**
 * Manages the 3rd-person camera rig.
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UThirdPersonCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UThirdPersonCameraComponent();

protected:
	virtual void OnRegister() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<class UCameraComponent> Camera;

	/// Distance from the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Camera)
	double TargetArmLength = 350.;

	/// Socket offset for over-the-shoulder view
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Camera)
	FVector SocketOffset = FVector(0., 60., 60.);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Camera)
	double CameraLagSpeed = 10.;
};
