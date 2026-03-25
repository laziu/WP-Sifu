// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemybase.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemybase::AEnemybase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	collisionComp = CreateDefaultSubobject<UBoxComponent>("collisionComp");
	collisionComp->SetupAttachment(RootComponent);
	collisionComp->SetBoxExtent(FVector(32.f,32.f,88.f));
	
	// 이동 방향으로 자동 회전
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

	// 컨트롤러 회전 비활성화 (이거 꺼야 OrientToMovement 제대로 작동)
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
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

