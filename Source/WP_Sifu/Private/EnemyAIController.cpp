// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Components/StateTreeComponent.h"

AEnemyAIController::AEnemyAIController()
{
	// State Tree 컴포넌트 생성 — 에디터에서 ST_Enemy 에셋 연결
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic("UnPossess");
	}

	Super::OnUnPossess();
}