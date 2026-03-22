// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "AbilitySystemComponent.h"
#include "PlayerInputComponent.h"
#include "ThirdPersonCameraComponent.h"
#include "CameraFocusComponent.h"
#include "HealthAttributeSet.h"
#include "PlayerCombatComponent.h"
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

		if (auto TempAnim = Ext::OpenObject<UAnimInstance>(
			TEXT("/Script/Engine.AnimBlueprint'/Game/Blueprints/Anims/ABP_MainCharacter.ABP_MainCharacter_C'")))
		{
			GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			GetMesh()->SetAnimInstanceClass(TempAnim->GetClass());
		}
	}

	// Third-person camera & input components
	EXT_CREATE_DEFAULT_SUBOBJECT(PlayerInputComp, TEXT("PlayerInput"));
	EXT_CREATE_DEFAULT_SUBOBJECT(ThirdPersonCameraComp, TEXT("ThirdPersonCamera"));
	EXT_CREATE_DEFAULT_SUBOBJECT(LockOnComp, TEXT("LockOn"));

	// Character rotation: follow the camera/controller yaw instead of movement direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0., 540., 0.);
	GetCharacterMovement()->JumpZVelocity = 600.;
	GetCharacterMovement()->AirControl = 0.2;

	// Set ability system
	EXT_CREATE_DEFAULT_SUBOBJECT(AbilitySystemComp, TEXT("AbilitySystemComponent"));
	EXT_CREATE_DEFAULT_SUBOBJECT(HealthAttribs, TEXT("HealthAttributes"));
	EXT_CREATE_DEFAULT_SUBOBJECT(CombatComp, TEXT("CombatComponent"));

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
		if (PlayerInputComp)
		{
			PlayerInputComp->SetupInputBindings(EnhancedInput);
		}
	}
}

UAbilitySystemComponent* AMainCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

UCombatComponentBase* AMainCharacter::GetCombatComponent() const
{
	return CombatComp;
}
