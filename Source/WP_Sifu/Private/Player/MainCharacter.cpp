// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "AbilitySystemComponent.h"
#include "PlayerMoveComponent.h"
#include "ThirdPersonCameraComponent.h"
#include "CameraFocusComponent.h"
#include "HealthAttributeSet.h"
#include "PlayerAttackComponent.h"
#include "PlayerCombatInteractionComponent.h"
#include "AttackCollisionComponent.h"
#include "AttackCollisionManagerComponent.h"
#include "UserExtension.h"
#include "EnhancedInputComponent.h"
#include "GameplayTags.generated.h"
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
	struct _FCompConfig
	{
		const TCHAR* Name;
		FGameplayTag Tag;
		FTransform Transform;
		const TCHAR* AssetPath;
	};
	for (const auto& Config : std::initializer_list<_FCompConfig>{
		     {
			     TEXT("HandL"), GameplayTag::Attack_Source_Hand_L,
			     FTransform(FRotator(0, 0, -20), FVector(1.0, 0.0, -0.5), FVector(1.0, 1.0, 0.5)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
		     {
			     TEXT("HandR"), GameplayTag::Attack_Source_Hand_R,
			     FTransform(FRotator(0, 0, -20), FVector(-1.0, 1.5, -4.5), FVector(1.0, 1.0, 0.5)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
		     {
			     TEXT("FootL"), GameplayTag::Attack_Source_Foot_L,
			     FTransform(FRotator(0, 0, -90), FVector(-3.5, 3.0, -1.5), FVector(1.0, 1.0, 1.0)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_FootCollision.SM_FootCollision'")
		     },
		     {
			     TEXT("FootR"), GameplayTag::Attack_Source_Foot_R,
			     FTransform(FRotator(0, 0, 90), FVector(3.5, -3.0, 1.5), FVector(1.0, 1.0, 1.0)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_FootCollision.SM_FootCollision'")
		     },
		     {
			     TEXT("ElbowL"), GameplayTag::Attack_Source_Elbow_L,
			     FTransform(FRotator(0, 0, 0), FVector(0.0, 0.0, 0.0), FVector(0.25, 0.25, 0.25)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
		     {
			     TEXT("ElbowR"), GameplayTag::Attack_Source_Elbow_R,
			     FTransform(FRotator(0, 0, 180), FVector(0.0, 0.0, 0.0), FVector(0.25, 0.25, 0.25)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
		     {
			     TEXT("KneeL"), GameplayTag::Attack_Source_Knee_L,
			     FTransform(FRotator(0, 0, 0), FVector(0.0, 0.0, 0.0), FVector(0.4, 0.4, 0.4)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
		     {
			     TEXT("KneeR"), GameplayTag::Attack_Source_Knee_R,
			     FTransform(FRotator(0, 0, 180), FVector(0.0, 0.0, 0.0), FVector(0.4, 0.4, 0.4)),
			     TEXT("/Script/Engine.StaticMesh'/Game/Meshes/SM_HandCollision.SM_HandCollision'")
		     },
	     })
	{
		auto Comp = CreateDefaultSubobject<UAttackCollisionComponent>(
			*FString::Printf(TEXT("AttackCollision_%s"), Config.Name));

		Comp->AttackTag = Config.Tag;
		Comp->SetupAttachment(GetMesh(), Config.Name);

		if (auto StaticMesh = Ext::OpenObject<UStaticMesh>(Config.AssetPath))
		{
			Comp->CollisionMeshAsset = StaticMesh;
			Comp->SetRelativeTransform(Config.Transform);
		}

		LimbCollisions.Add(Comp);
	}

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
	for (auto& Comp : LimbCollisions)
	{
		AttackCollisionManagerComp->RegisterPersistentCollision(Comp);
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
