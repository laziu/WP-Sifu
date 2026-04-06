// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Attackable.h"
#include "GameFramework/Character.h"
#include "Enemybase.generated.h"

UCLASS()
class WP_SIFU_API AEnemybase :
	public ACharacter,
	public IAbilitySystemInterface,
	public IAttackable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemybase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<class UBoxComponent> collisionComp;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual class UCombatInteractionComponentBase* GetCombatInteractionComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Abilities)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<class UHealthAttributeSet> HealthAttributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<class UDummyCombatInteractionComponent> CombatInteractionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	float MaxHealth = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	float MaxStructure = 50.f;
};
