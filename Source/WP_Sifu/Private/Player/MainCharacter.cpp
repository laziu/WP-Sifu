// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "AbilitySystemComponent.h"
#include "PlayerMoveComponent.h"
#include "IInputBindable.h"
#include "ThirdPersonCameraComponent.h"
#include "CameraFocusComponent.h"
#include "HealthAttributeSet.h"
#include "PlayerAttackComponent.h"
#include "PlayerCombatInteractionComponent.h"
#include "AttackCollisionComponent.h"
#include "AttackCollisionManagerComponent.h"
#include "UserExtension.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set mesh
	if (auto TempMesh = Ext::OpenObject<USkeletalMesh>(TEXT(
		"/Script/Engine.SkeletalMesh'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/SkeletalMeshes/SK_M_MainChar_01.SK_M_MainChar_01'")))
	{
		GetMesh()->SetSkeletalMesh(TempMesh);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0., 0., -88.), FRotator(0., -90., 0.));

		if (auto TempAnim = Ext::OpenClass<UAnimInstance>(
			TEXT("/Script/Engine.AnimBlueprint'/Game/Blueprints/Anims/ABP_MainCharacter.ABP_MainCharacter_C'")))
		{
			GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			GetMesh()->SetAnimInstanceClass(TempAnim);
		}
	}

	// Third-person camera & input components
	EXT_CREATE_DEFAULT_SUBOBJECT(PlayerMoveComp, TEXT("PlayerMove"));
	EXT_CREATE_DEFAULT_SUBOBJECT(ThirdPersonCameraComp, TEXT("ThirdPersonCamera"));
	EXT_CREATE_DEFAULT_SUBOBJECT(LockOnComp, TEXT("LockOn"));

	// Set ability system and combat-related components.
	EXT_CREATE_DEFAULT_SUBOBJECT(AbilitySystemComp, TEXT("AbilitySystemComponent"));
	EXT_CREATE_DEFAULT_SUBOBJECT(HealthAttribs, TEXT("HealthAttributes"));
	EXT_CREATE_DEFAULT_SUBOBJECT(CombatInteractionComp, TEXT("CombatInteractionComponent"));
	EXT_CREATE_DEFAULT_SUBOBJECT(AttackComp, TEXT("AttackComponent"));
	EXT_CREATE_DEFAULT_SUBOBJECT(AttackCollisionManagerComp, TEXT("AttackCollisionManager"));

	// Unarmed hand/foot collision components (attached to skeletal mesh sockets)
	EXT_CREATE_DEFAULT_SUBOBJECT(HandCollisionLeft, TEXT("HandCollisionLeft"));
	HandCollisionLeft->SetupAttachment(GetMesh(), TEXT("HandL"));
	HandCollisionLeft->AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Source.Hand.L"));

	EXT_CREATE_DEFAULT_SUBOBJECT(HandCollisionRight, TEXT("HandCollisionRight"));
	HandCollisionRight->SetupAttachment(GetMesh(), TEXT("HandR"));
	HandCollisionRight->AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Source.Hand.R"));

	EXT_CREATE_DEFAULT_SUBOBJECT(FootCollisionLeft, TEXT("FootCollisionLeft"));
	FootCollisionLeft->SetupAttachment(GetMesh(), TEXT("FootL"));
	FootCollisionLeft->AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Source.Foot.L"));

	EXT_CREATE_DEFAULT_SUBOBJECT(FootCollisionRight, TEXT("FootCollisionRight"));
	FootCollisionRight->SetupAttachment(GetMesh(), TEXT("FootR"));
	FootCollisionRight->AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Source.Foot.R"));

	HealthAttribs->InitHealth(MaxHealth);
	HealthAttribs->InitMaxHealth(MaxHealth);
	HealthAttribs->InitStructure(0.f);
	HealthAttribs->InitMaxStructure(MaxStructure);
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Setup owner for ability system component
	AbilitySystemComp->InitAbilityActorInfo(this, this);

	// Register the unarmed collision components
	AttackCollisionManagerComp->RegisterPersistentCollision(HandCollisionLeft);
	AttackCollisionManagerComp->RegisterPersistentCollision(HandCollisionRight);
	AttackCollisionManagerComp->RegisterPersistentCollision(FootCollisionLeft);
	AttackCollisionManagerComp->RegisterPersistentCollision(FootCollisionRight);
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		TArray<UActorComponent*> Components;
		GetComponents(Components);

		for (UActorComponent* Comp : Components)
		{
			if (IInputBindable* Bindable = Cast<IInputBindable>(Comp))
			{
				Bindable->SetupInputBindings(EnhancedInput);
			}
		}
	}
}

UAbilitySystemComponent* AMainCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

UCombatInteractionComponentBase* AMainCharacter::GetCombatInteractionComponent() const
{
	return CombatInteractionComp;
}
