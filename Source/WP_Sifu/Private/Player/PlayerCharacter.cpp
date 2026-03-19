// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.h"
#include "PlayerCombatComponent.h"
#include "UserExtension.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set ability system
	EXT_CREATE_SUBOBJECT(AbilitySystemComp);
	EXT_CREATE_SUBOBJECT(HealthAttribs);
	EXT_CREATE_SUBOBJECT(CombatComp);

	HealthAttribs->InitHealth(MaxHealth);
	HealthAttribs->InitMaxHealth(MaxHealth);
	HealthAttribs->InitStructure(0.f);
	HealthAttribs->InitMaxStructure(MaxStructure);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Setup owner for ability system component
	AbilitySystemComp->InitAbilityActorInfo(this, this);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

UCombatComponentBase* APlayerCharacter::GetCombatComponent() const
{
	return CombatComp;
}
