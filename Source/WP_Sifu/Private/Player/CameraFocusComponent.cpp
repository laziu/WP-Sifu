// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraFocusComponent.h"

#include "Attackable.h"
#include "DeathHandlerComponentBase.h"
#include "ThirdPersonCameraComponent.h"
#include "PlayerAttackComponent.h"
#include "PlayerMoveComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"


UCameraFocusComponent::UCameraFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
}

void UCameraFocusComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCameraFocusComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputMove, ETriggerEvent::Triggered, this, &UCameraFocusComponent::OnInputMove);
	EIC->BindAction(InputMove, ETriggerEvent::Completed, this, &UCameraFocusComponent::OnInputMoveStopped);
	EIC->BindAction(InputMove, ETriggerEvent::Canceled, this, &UCameraFocusComponent::OnInputMoveStopped);
}

void UCameraFocusComponent::OnInputMove(const FInputActionValue& Value)
{
	MoveInputY = Value.Get<FVector2D>().Y;
}

void UCameraFocusComponent::OnInputMoveStopped()
{
	MoveInputY = 0.f;
}

void UCameraFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Debug: draw red sphere at focused target
	if (IsValid(TargetActor))
	{
		DrawDebugSphere(GetWorld(), TargetActor->GetActorLocation(), 40.f, 12, FColor::Red, false, -1.f, 0, 2.f);
	}

	// Keep target if still valid, alive, and within range; otherwise auto-scan
	bool bTargetValid = IsValid(TargetActor) &&
		FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation()) <= FocusRadius;
	if (bTargetValid)
	{
		if (auto* DeathComp = TargetActor->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead()) bTargetValid = false;
		}
	}
	if (!bTargetValid)
	{
		TargetActor = FindBestTarget();
	}

	// Collect nearby enemies (used by combat stance, zoom, and socket offset)
	TArray<AActor*> NearbyEnemies = FindNearbyEnemies();
	FilterOutliers(NearbyEnemies);

	// Update combat stance
	UpdateCombatStance();

	// Update zoom based on nearby enemies
	UpdateZoom(NearbyEnemies, DeltaTime);

	// Update dynamic socket offset
	UpdateSocketOffset(DeltaTime);
}

// ── Focus Target ─────────────────────────────────────────────

void UCameraFocusComponent::SetFocusTarget(AActor* NewTarget)
{
	if (NewTarget && (NewTarget == GetOwner() || !NewTarget->Implements<UAttackable>()))
	{
		return;
	}
	if (NewTarget)
	{
		if (auto* DeathComp = NewTarget->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead()) return;
		}
	}
	TargetActor = NewTarget;
}

FVector UCameraFocusComponent::GetFacingDirection() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return FVector::ForwardVector;

	if (IsValid(TargetActor))
	{
		const FVector ToTarget = TargetActor->GetActorLocation() - Owner->GetActorLocation();
		const FVector ToTargetXY = FVector(ToTarget.X, ToTarget.Y, 0.);
		if (!ToTargetXY.IsNearlyZero())
		{
			return ToTargetXY.GetSafeNormal();
		}
	}

	if (const auto Pawn = Cast<APawn>(Owner))
	{
		if (const auto PC = Pawn->GetController())
		{
			const FRotator Rot = PC->GetControlRotation();
			return FRotationMatrix(FRotator(0., Rot.Yaw, 0.)).GetUnitAxis(EAxis::X);
		}
	}

	return Owner->GetActorForwardVector();
}

AActor* UCameraFocusComponent::FindBestTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	const FVector OwnerLocation = Owner->GetActorLocation();

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

	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), OwnerLocation, FocusRadius, {}, AActor::StaticClass(), {Owner}, OverlapActors);

	AActor* BestCandidate = nullptr;
	double BestScore = TNumericLimits<double>::Max();
	const double CosAngleThreshold = FMath::Cos(FMath::DegreesToRadians(FocusAngle));

	for (AActor* Candidate : OverlapActors)
	{
		if (!Candidate->Implements<UAttackable>()) continue;
		if (auto* DeathComp = Candidate->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead()) continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - OwnerLocation;
		const FVector Direction = ToCandidate.GetSafeNormal();
		const double Distance = ToCandidate.Size();
		const double CosAngle = FVector::DotProduct(ForwardDir, Direction);
		if (Distance < 1e-4 || Distance > FocusRadius) continue;
		if (CosAngle < CosAngleThreshold) continue;

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
	const float TargetYaw = Direction.Rotation().Yaw;

	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetYaw);
	const float NewYaw = CurrentRot.Yaw + FMath::FInterpTo(0.f, DeltaYaw, DeltaTime, InterpSpeed);

	const FRotator DesiredRot(CurrentRot.Pitch, NewYaw, 0.);
	PC->SetControlRotation(DesiredRot);
}

// ── Nearby Enemies ───────────────────────────────────────────

TArray<AActor*> UCameraFocusComponent::FindNearbyEnemies() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return {};

	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Owner->GetActorLocation(), FocusRadius,
		{}, AActor::StaticClass(), {Owner}, OverlapActors);

	TArray<AActor*> Enemies;
	for (AActor* Actor : OverlapActors)
	{
		if (Actor->Implements<UAttackable>())
		{
			if (auto* DeathComp = Actor->FindComponentByClass<UDeathHandlerComponentBase>())
			{
				if (DeathComp->IsDead()) continue;
			}
			Enemies.Add(Actor);
		}
	}
	return Enemies;
}

void UCameraFocusComponent::FilterOutliers(TArray<AActor*>& Enemies) const
{
	if (Enemies.Num() <= 1) return;

	const AActor* Owner = GetOwner();
	const FVector OwnerLoc = Owner->GetActorLocation();

	// Compute mean distance
	double MeanDist = 0.;
	TArray<double> Distances;
	Distances.Reserve(Enemies.Num());
	for (const AActor* E : Enemies)
	{
		const double D = FVector::Dist(OwnerLoc, E->GetActorLocation());
		Distances.Add(D);
		MeanDist += D;
	}
	MeanDist /= Distances.Num();

	// Compute standard deviation
	double Variance = 0.;
	for (double D : Distances)
	{
		Variance += FMath::Square(D - MeanDist);
	}
	const double StdDev = FMath::Sqrt(Variance / Distances.Num());

	// Remove enemies beyond 2 standard deviations from mean
	const double Threshold = MeanDist + 2. * StdDev;
	for (int32 i = Enemies.Num() - 1; i >= 0; --i)
	{
		if (Distances[i] > Threshold)
		{
			Enemies.RemoveAt(i);
		}
	}
}

// ── Combat Stance ────────────────────────────────────────────

void UCameraFocusComponent::UpdateCombatStance()
{
	const AActor* Owner = GetOwner();
	if (!Owner) return;

	// Check if attack component is in a non-neutral state
	bool bShouldBeCombat = false;
	if (const auto* AttackComp = Owner->FindComponentByClass<UPlayerAttackComponent>())
	{
		bShouldBeCombat = AttackComp->IsAttacking();
	}

	// Or if there are enemies nearby
	if (!bShouldBeCombat && IsLockedOn())
	{
		bShouldBeCombat = true;
	}

	// Check if running — running overrides combat stance
	if (const auto* MoveComp = Owner->FindComponentByClass<UPlayerMoveComponent>())
	{
		if (MoveComp->IsRunning())
		{
			bShouldBeCombat = false;
		}
	}

	if (bShouldBeCombat)
	{
		// Cancel any pending cooldown
		GetWorld()->GetTimerManager().ClearTimer(CombatCooldownTimer);
		SetCombatStanceInternal(true);
	}
	else if (bInCombatStance && !CombatCooldownTimer.IsValid())
	{
		// Start cooldown timer to leave combat stance
		GetWorld()->GetTimerManager().SetTimer(
			CombatCooldownTimer, [this]()
			{
				SetCombatStanceInternal(false);
			},
			CombatStanceCooldown, false);
	}
}

void UCameraFocusComponent::SetCombatStanceInternal(bool bNewValue)
{
	if (bInCombatStance == bNewValue) return;
	bInCombatStance = bNewValue;

	// Notify combat stance to the move component
	if (auto* MoveComp = GetOwner()->FindComponentByClass<UPlayerMoveComponent>())
	{
		MoveComp->SetCombatStance(bInCombatStance);
	}

	OnCombatStanceChanged.Broadcast(bInCombatStance);
}

// ── Zoom ─────────────────────────────────────────────────────

void UCameraFocusComponent::UpdateZoom(const TArray<AActor*>& NearbyEnemies, float DeltaTime)
{
	auto* CamComp = GetOwner()->FindComponentByClass<UThirdPersonCameraComponent>();
	if (!CamComp || !CamComp->SpringArm) return;

	double DesiredArmLength = BaseArmLength;

	if (bInCombatStance && NearbyEnemies.Num() > 0)
	{
		const FVector OwnerLoc = GetOwner()->GetActorLocation();

		// Compute spread radius (max distance from owner)
		double MaxDist = 0.;
		for (const AActor* E : NearbyEnemies)
		{
			MaxDist = FMath::Max(MaxDist, FVector::Dist(OwnerLoc, E->GetActorLocation()));
		}

		DesiredArmLength = BaseArmLength
			+ NearbyEnemies.Num() * PerEnemyZoomOut
			+ MaxDist * SpreadZoomFactor;
	}

	DesiredArmLength = FMath::Clamp(DesiredArmLength, BaseArmLength, BaseArmLength * 2.0);

	// --- Smooth damp (critically-damped spring) ---
	// Naturally produces ease-in / ease-out.  ZoomMaxSpeed caps peak velocity.
	const double SmoothTime = FMath::Max(ZoomSmoothTime, 1e-4);
	const double Omega = 2.0 / SmoothTime;

	// Clamp the displacement so velocity never exceeds ZoomMaxSpeed
	const double MaxChange = ZoomMaxSpeed * SmoothTime;
	double Change = FMath::Clamp(
		CamComp->SpringArm->TargetArmLength - DesiredArmLength, -MaxChange, MaxChange);
	const double AdjustedTarget = CamComp->SpringArm->TargetArmLength - Change;

	const double X = Omega * DeltaTime;
	const double Exp = 1.0 / (1.0 + X + 0.48 * X * X + 0.235 * X * X * X);
	const double Temp = (ZoomVelocity + Omega * Change) * DeltaTime;
	ZoomVelocity = (ZoomVelocity - Omega * Temp) * Exp;

	double NewLength = AdjustedTarget + (Change + Temp) * Exp;

	// Overshoot guard
	if ((AdjustedTarget - CamComp->SpringArm->TargetArmLength) * (NewLength - AdjustedTarget) > 0.0)
	{
		NewLength = AdjustedTarget;
		ZoomVelocity = 0.0;
	}

	CamComp->SpringArm->TargetArmLength = NewLength;
}

// ── Socket Offset ────────────────────────────────────────────

void UCameraFocusComponent::UpdateSocketOffset(float DeltaTime)
{
	auto* CamComp = GetOwner()->FindComponentByClass<UThirdPersonCameraComponent>();
	if (!CamComp || !CamComp->SpringArm) return;

	USpringArmComponent* SpringArm = CamComp->SpringArm;
	const double CurrentY = SpringArm->SocketOffset.Y;
	double NewY;

	// Ease-in: smoothly ramp the strafe signal so drift starts gently.
	SmoothedStrafeInput = FMath::FInterpTo(SmoothedStrafeInput, MoveInputY, DeltaTime, DriftEaseSpeed);
	const double StrafeInput = SmoothedStrafeInput;

	if (FMath::Abs(StrafeInput) > KINDA_SMALL_NUMBER)
	{
		NewY = FMath::Clamp(
			CurrentY - StrafeInput * CombatOffsetDriftSpeed * DeltaTime,
			-MaxSocketOffsetY, MaxSocketOffsetY);

		OffsetVelocity = 0.; // Reset so smooth-damp starts clean on return
	}
	else
	{
		// No strafe input: return to shoulder at ±MaxSocketOffsetY using a critically-damped spring.
		// Produces ease-in/out motion and caps speed at OffsetMaxSpeed.
		const double TargetY = (CurrentY >= 0.) ? MaxSocketOffsetY : -MaxSocketOffsetY;

		const double SmoothTime = FMath::Max(OffsetSmoothTime, 1e-4);
		const double Omega = 2.0 / SmoothTime;
		const double MaxChange = OffsetMaxSpeed * SmoothTime;
		double Change = FMath::Clamp(CurrentY - TargetY, -MaxChange, MaxChange);
		const double AdjTarget = CurrentY - Change;
		const double X = Omega * DeltaTime;
		const double Exp = 1.0 / (1.0 + X + 0.48 * X * X + 0.235 * X * X * X);
		const double Temp = (OffsetVelocity + Omega * Change) * DeltaTime;
		OffsetVelocity = (OffsetVelocity - Omega * Temp) * Exp;
		NewY = AdjTarget + (Change + Temp) * Exp;

		// Overshoot guard
		if ((AdjTarget - CurrentY) * (NewY - AdjTarget) > 0.)
		{
			NewY = AdjTarget;
			OffsetVelocity = 0.;
		}
	}

	SpringArm->SocketOffset.Y = NewY;
}
