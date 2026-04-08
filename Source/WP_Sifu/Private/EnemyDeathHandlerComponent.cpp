// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyDeathHandlerComponent.h"

#include "UserExtension.h"
#include "Components/CapsuleComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"


UEnemyDeathHandlerComponent::UEnemyDeathHandlerComponent()
{
	Ext::SetObject(DeathMontage,
		TEXT("/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Enemy/SK_Servant_M_Hideout00_CasualDisciple_01/SkeletalMeshes/AM_Death.AM_Death'"));
}


void UEnemyDeathHandlerComponent::OnDeathBegin()
{
	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		// Stop AI behavior immediately
		if (auto* AIC = Cast<AAIController>(Character->GetController()))
		{
			if (auto* STC = AIC->FindComponentByClass<UStateTreeComponent>())
			{
				STC->StopLogic(TEXT("Dead"));
			}
		}

		// Stop all movement
		Character->GetCharacterMovement()->DisableMovement();
	}
}


void UEnemyDeathHandlerComponent::OnDeathComplete()
{
	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		OnDeathFinished.Broadcast();
		return;
	}

	auto* SourceMesh = Character->GetMesh();
	if (SourceMesh && SourceMesh->GetSkeletalMeshAsset())
	{
		// Spawn a minimal corpse actor to hold the frozen mesh
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto* Corpse = GetWorld()->SpawnActor<AActor>(
			AActor::StaticClass(), FTransform::Identity, SpawnParams);

		if (Corpse)
		{
			auto* CorpseMesh = NewObject<USkeletalMeshComponent>(Corpse, TEXT("CorpseMesh"));
			CorpseMesh->SetSkeletalMeshAsset(SourceMesh->GetSkeletalMeshAsset());
			CorpseMesh->SetWorldTransform(SourceMesh->GetComponentTransform());

			// Copy materials
			for (int32 i = 0; i < SourceMesh->GetNumMaterials(); ++i)
			{
				CorpseMesh->SetMaterial(i, SourceMesh->GetMaterial(i));
			}

			Corpse->SetRootComponent(CorpseMesh);
			CorpseMesh->RegisterComponent();

			// Copy the death pose from the original mesh
			CorpseMesh->SetLeaderPoseComponent(SourceMesh);
			// Force one evaluation so the pose is copied
			CorpseMesh->TickPose(0.f, false);
			// Disconnect from leader and freeze
			CorpseMesh->SetLeaderPoseComponent(nullptr);
			CorpseMesh->bPauseAnims = true;
			CorpseMesh->SetComponentTickEnabled(false);
		}
	}

	OnDeathFinished.Broadcast();

	// Destroy the original enemy actor
	Character->Destroy();
}
