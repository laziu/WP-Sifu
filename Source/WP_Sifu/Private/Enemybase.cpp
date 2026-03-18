// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemybase.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AEnemybase::AEnemybase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	collisionComp = CreateDefaultSubobject<UBoxComponent>("collisionComp");
	collisionComp->SetupAttachment(RootComponent);
	collisionComp->SetBoxExtent(FVector(32.f,32.f,88.f));

}

// Called when the game starts or when spawned
void AEnemybase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemybase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemybase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

