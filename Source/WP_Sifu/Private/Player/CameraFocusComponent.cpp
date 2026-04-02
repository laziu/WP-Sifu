// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraFocusComponent.h"

#include "Attackable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"


UCameraFocusComponent::UCameraFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCameraFocusComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCameraFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Keep focusing if target is still valid and within range
	if (IsValid(TargetActor) &&
		FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation()) <= BreakDistance)
	{
		UpdateRotationToTarget(DeltaTime);
		return;
	}

	// Target gone or out of range — auto-scan for the nearest enemy
	TargetActor = FindBestTarget();
	if (TargetActor)
	{
		UpdateRotationToTarget(DeltaTime);
	}
}

void UCameraFocusComponent::SetFocusTarget(AActor* NewTarget)
{
	TargetActor = NewTarget;
}

FVector UCameraFocusComponent::GetFacingDirection() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return FVector::ForwardVector;

	// To focusing target if exists
	if (IsValid(TargetActor))
	{
		const FVector ToTarget = TargetActor->GetActorLocation() - Owner->GetActorLocation();
		const FVector ToTargetXY = FVector(ToTarget.X, ToTarget.Y, 0.);
		if (!ToTargetXY.IsNearlyZero())
		{
			return ToTargetXY.GetSafeNormal();
		}
	}

	// Camera forward
	if (const auto Pawn = Cast<APawn>(Owner))
	{
		if (const auto PC = Pawn->GetController())
		{
			const FRotator Rot = PC->GetControlRotation();
			const FRotator RotXY(0., Rot.Yaw, 0.);
			return FRotationMatrix(RotXY).GetUnitAxis(EAxis::X);
		}
	}

	return Owner->GetActorForwardVector();
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
