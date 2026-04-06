// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionComponent.h"

#include "WP_Sifu.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"


UAttackCollisionComponent::UAttackCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}


// ─────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────

void UAttackCollisionComponent::OnRegister()
{
	Super::OnRegister();

	if (!CollisionMeshComp)
	{
		CollisionMeshComp = NewObject<UStaticMeshComponent>(
			GetOwner(), MakeUniqueObjectName(
				GetOwner(), UStaticMeshComponent::StaticClass(),
				*FString::Printf(TEXT("CollisionMesh_%s"), *GetName())));

		CollisionMeshComp->SetupAttachment(this);
		CollisionMeshComp->RegisterComponent();
	}

	ApplyCollisionMesh();
	ApplyEditorVisibility();
}

void UAttackCollisionComponent::ApplyCollisionMesh()
{
	if (!CollisionMeshComp) return;

	CollisionMeshComp->SetStaticMesh(CollisionMeshAsset);

	// Use QueryOnly so the BodyInstance is preserved (required for ComponentSweepMulti).
	// Set all responses to Ignore so no automatic collision or overlap events are generated.
	CollisionMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionMeshComp->SetGenerateOverlapEvents(false);
	CollisionMeshComp->SetCollisionObjectType(TraceChannel);
	CollisionMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void UAttackCollisionComponent::ApplyEditorVisibility()
{
	if (!CollisionMeshComp) return;

	CollisionMeshComp->SetHiddenInGame(true);
	CollisionMeshComp->SetVisibility(bShowCollisionMeshInEditor);
}

#if WITH_EDITOR
void UAttackCollisionComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetMemberPropertyName();

	if (PropName == GET_MEMBER_NAME_CHECKED(UAttackCollisionComponent, CollisionMeshAsset)
		|| PropName == GET_MEMBER_NAME_CHECKED(UAttackCollisionComponent, TraceChannel))
	{
		ApplyCollisionMesh();
	}

	if (PropName == GET_MEMBER_NAME_CHECKED(UAttackCollisionComponent, bShowCollisionMeshInEditor))
	{
		ApplyEditorVisibility();
	}
}
#endif


// ─────────────────────────────────────────────
//  Trace Activation
// ─────────────────────────────────────────────

void UAttackCollisionComponent::ActivateTrace()
{
	bTraceActive = true;
	HitActors.Empty();
	PreviousLocation = GetComponentLocation();
	SetComponentTickEnabled(true);
}

void UAttackCollisionComponent::DeactivateTrace()
{
	bTraceActive = false;
	HitActors.Empty();
	SetComponentTickEnabled(false);
}


// ─────────────────────────────────────────────
//  Tick → Sweep
// ─────────────────────────────────────────────

void UAttackCollisionComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bTraceActive)
	{
		PerformSweep();
		PreviousLocation = GetComponentLocation();
	}
}

void UAttackCollisionComponent::PerformSweep()
{
	if (!CollisionMeshComp || !CollisionMeshComp->GetStaticMesh()) return;

	const FVector CurrentLocation = GetComponentLocation();
	const FQuat CurrentRotation = GetComponentQuat();

	FComponentQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	TArray<FHitResult> Hits;
	GetWorld()->ComponentSweepMulti(
		Hits,
		CollisionMeshComp,
		PreviousLocation,
		CurrentLocation,
		CurrentRotation,
		Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;

		TWeakObjectPtr<AActor> WeakHit(HitActor);
		if (HitActors.Contains(WeakHit)) continue;

		HitActors.Add(WeakHit);
		OnAttackHit.Broadcast(HitActor, Hit);
	}
}
