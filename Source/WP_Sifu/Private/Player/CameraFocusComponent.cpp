// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraFocusComponent.h"

#include "Attackable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"


UCameraFocusComponent::UCameraFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCameraFocusComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCameraFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Release focusing if target is destroyed or out of range
	if (!IsValid(TargetActor) ||
		FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation()) > BreakDistance)
	{
		TargetActor = nullptr;
		SetComponentTickEnabled(false);
		return;
	}

	UpdateRotationToTarget(DeltaTime);
}

bool UCameraFocusComponent::Focus()
{
	// Find a new target
	TargetActor = FindBestTarget();
	if (TargetActor)
	{
		SetComponentTickEnabled(true);
		return true;
	}
	return false;
}

bool UCameraFocusComponent::ReleaseFocus()
{
	if (TargetActor)
	{
		TargetActor = nullptr;
		SetComponentTickEnabled(false);
		return true;
	}
	return false;
}

void UCameraFocusComponent::SwitchTarget(bool bToRight)
{
	if (!TargetActor) return;

	if (AActor* BestCandidate = FindNextTarget(bToRight))
	{
		TargetActor = BestCandidate;
	}
}

AActor* UCameraFocusComponent::FindBestTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	const FVector OwnerLocation = Owner->GetActorLocation();

	// Use controller forward if available, otherwise actor forward
	FVector ForwardDir;
	if (const auto OwnerCharacter = Cast<ACharacter>(Owner);
		OwnerCharacter && OwnerCharacter->GetController())
	{
		const FRotator ControlRotation = OwnerCharacter->GetControlRotation();
		ForwardDir = FRotationMatrix(FRotator(0.f, ControlRotation.Yaw, 0.f)).GetUnitAxis(EAxis::X);
	}
	else
	{
		ForwardDir = Owner->GetActorForwardVector();
	}


	// Find candidates in a sphere around the owner
	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), OwnerLocation, FocusRadius, {}, AActor::StaticClass(), {Owner}, OverlapActors);

	// Find the best candidate that is closest to the center of view
	AActor* BestCandidate = nullptr;
	double BestScore = TNumericLimits<double>::Max();

	const double CosAngleThreshold = FMath::Cos(FMath::DegreesToRadians(FocusAngle));

	for (AActor* Candidate : OverlapActors)
	{
		if (!Candidate->Implements<UAttackable>()) continue;

		const FVector ToCandidate = Candidate->GetActorLocation() - OwnerLocation;
		const FVector Direction = ToCandidate.GetSafeNormal();
		const double Distance = ToCandidate.Size();
		const double CosAngle = FVector::DotProduct(ForwardDir, Direction);
		if (Distance < 1e-4) continue;
		if (CosAngle < CosAngleThreshold) continue;

		// Score by distance (closer is better); bias toward center of view
		const double Score = Distance * (1. - CosAngle * 0.5);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestCandidate = Candidate;
		}
	}

	return BestCandidate;
}

AActor* UCameraFocusComponent::FindNextTarget(bool bToRight) const
{
	if (!TargetActor)
	{
		return FindBestTarget();
	}

	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	const FVector OwnerLocation = Owner->GetActorLocation();

	// Get a direction relative to the current target direction
	const FVector ForwardDir = (TargetActor->GetActorLocation() - OwnerLocation).GetSafeNormal();
	const FVector RightDir = FVector::CrossProduct(FVector::UpVector, ForwardDir).GetSafeNormal();

	// Find candidates in a sphere around the owner
	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), OwnerLocation, FocusRadius, {}, AActor::StaticClass(), {Owner}, OverlapActors);

	// Find the best candidate that is in the desired direction (left/right) and closest to the current target
	AActor* BestCandidate = nullptr;
	double BestScore = TNumericLimits<double>::Max();

	for (AActor* Candidate : OverlapActors)
	{
		if (Candidate == TargetActor || Candidate == Owner) continue;
		if (!Candidate->Implements<UAttackable>()) continue;

		const FVector ToCandidate = Candidate->GetActorLocation() - OwnerLocation;
		const FVector Direction = ToCandidate.GetSafeNormal();
		const double Distance = ToCandidate.Size();
		const double CosAngle = FVector::DotProduct(ForwardDir, Direction);
		const double DotRight = FVector::DotProduct(RightDir, Direction);
		if (Distance < 1e-4) continue;

		if ((bToRight && DotRight > 0.05) || (!bToRight && DotRight < -0.05))
		{
			const double Score = Distance * (1. - CosAngle * 0.5);
			if (Score < BestScore)
			{
				BestScore = Score;
				BestCandidate = Candidate;
			}
		}
	}

	return BestCandidate;
}

void UCameraFocusComponent::UpdateRotationToTarget(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !TargetActor) return;

	const APawn* Pawn = Cast<APawn>(Owner);
	AController* PC = Pawn ? Pawn->GetController() : nullptr;
	if (!PC) return;

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();

	const FRotator CurrentRot = PC->GetControlRotation();
	const FRotator LookAtRot = Direction.Rotation();

	// Interpolate yaw and pitch toward the target
	const FRotator DesiredRot(
		FMath::FInterpTo(CurrentRot.Pitch, LookAtRot.Pitch, DeltaTime, InterpSpeed),
		FMath::FInterpTo(CurrentRot.Yaw, LookAtRot.Yaw, DeltaTime, InterpSpeed),
		0.);

	PC->SetControlRotation(DesiredRot);
}
