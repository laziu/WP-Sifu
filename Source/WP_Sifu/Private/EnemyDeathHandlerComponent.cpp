// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyDeathHandlerComponent.h"

#include "UserExtension.h"
#include "Components/CapsuleComponent.h"
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
		// Release the pawn from its AI controller as soon as death starts.
		if (auto* AIC = Cast<AAIController>(Character->GetController()))
		{
			AIC->UnPossess();
		}

		// Stop all movement
		Character->GetCharacterMovement()->DisableMovement();

		// Disable capsule collision so the corpse doesn't block movement
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
