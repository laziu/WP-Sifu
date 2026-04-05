// Fill out your copyright notice in the Description page of Project Settings.

#include "ThirdPersonCameraComponent.h"

#include "UserExtension.h"
#include "WP_Sifu.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


UThirdPersonCameraComponent::UThirdPersonCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UThirdPersonCameraComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetRootComponent())
	{
		LOGE(TEXT("ThirdPersonCameraComponent requires an Actor with a RootComponent"));
		return;
	}

	SpringArm = NewObject<USpringArmComponent>(Owner, TEXT("CameraSpringArm"));
	SpringArm->SetupAttachment(Owner->GetRootComponent());
	SpringArm->TargetArmLength = TargetArmLength;
	SpringArm->TargetOffset = TargetOffset;
	SpringArm->SocketOffset = SocketOffset;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->RegisterComponent();

	Camera = NewObject<UCameraComponent>(Owner, TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->RegisterComponent();
}
