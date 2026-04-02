// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTags.generated.h"
#include "HealthAttributeSet.h"
#include "PlayerAttackComponent.h"
#include "WP_Sifu.h"
#include "GameFramework/Character.h"


UPlayerCombatInteractionComponent::UPlayerCombatInteractionComponent()
{
	bWantsInitializeComponent = true;

	Ext::SetObject(InputBlock, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Block.IA_Block'"));
	Ext::SetObject(InputMove, TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	Ext::SetObject(
		BlockStartMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_GuardStart.AM_MainChar_GuardStart'"));
	Ext::SetObject(
		DodgeUpMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_DodgeJump.AM_MainChar_DodgeJump'"));
	Ext::SetObject(
		DodgeDownMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_DodgeLeft.AM_MainChar_DodgeLeft'"));
	Ext::SetObject(
		HitMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_Hit.AM_MainChar_Hit'"));
	Ext::SetObject(
		BlockHitMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_HitBlock.AM_MainChar_HitBlock'"));
}

void UPlayerCombatInteractionComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputBlock, ETriggerEvent::Started, this, &UPlayerCombatInteractionComponent::StartBlock);
	EIC->BindAction(InputBlock, ETriggerEvent::Completed, this, &UPlayerCombatInteractionComponent::StopBlock);
	EIC->BindAction(InputMove, ETriggerEvent::Started, this, &UPlayerCombatInteractionComponent::TryBlockDodge);
}

void UPlayerCombatInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const auto ASI = CastChecked<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComp = ASI->GetAbilitySystemComponent();
}

EAttackResponse UPlayerCombatInteractionComponent::ApplyDamage(const FAttackPayload& Payload)
{
	switch (DefenceState)
	{
	case EDefenceState::None:
		{
			// Late input buffer: defer damage application for a short window
			PendingHitPayload = Payload;
			GetWorld()->GetTimerManager().SetTimer(
				PendingHitTimer, this,
				&UPlayerCombatInteractionComponent::ApplyPendingHitDamage,
				LateInputWindow, false);

			return EAttackResponse::Hit; // Tentative; may be overridden by late parry/dodge
		}

	case EDefenceState::Blocking:
		{
			// no health damage, full structure damage
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive, Payload.StructureDamage);

			SetHitReaction(EHitReactionType::BlockHit, Payload.ImpactLocation);

			return EAttackResponse::Block;
		}

	case EDefenceState::Dodging:
		{
			// no health damage, reduce structure
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive, Payload.StructureDamage * DeflectStructureRate);

			return EAttackResponse::Dodge;
		}

	case EDefenceState::Parrying:
		{
			// no health damage, reduce structure
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive, Payload.StructureDamage * DeflectStructureRate);

			// 적에게 Structure 데미지 반사
			if (Payload.Instigator.IsValid())
			{
				FAttackPayload ReflectPayload;
				ReflectPayload.StructureDamage = Payload.StructureDamage;
				ReflectPayload.HealthDamage = 0.f;
				ReflectPayload.Instigator = GetOwner();
				ReflectPayload.bUnblockable = true;
				SendAttack(Payload.Instigator.Get(), ReflectPayload);
			}

			// Parry 성공 → 콤보 컴포넌트에 알림
			if (auto* AttackComp = GetOwner()->FindComponentByClass<UPlayerAttackComponent>())
			{
				AttackComp->SetState(GameplayTag::Combat_State_Parry);
			}

			return EAttackResponse::Parry;
		}

	case EDefenceState::Invincible:
	default:
		return EAttackResponse::Ignore;
	}
}

bool UPlayerCombatInteractionComponent::IsAttackFromBehind(const FVector& ImpactLocation) const
{
	// false when ImpactLocation is not set (e.g., for unblockable attacks where location is irrelevant)
	if (ImpactLocation.IsZero())
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	const FVector ToImpact = (ImpactLocation - Owner->GetActorLocation()).GetSafeNormal();
	const FVector Forward = Owner->GetActorForwardVector();

	// Dot product < 0 means the attack comes from behind (angle > 90 degrees)
	return FVector::DotProduct(Forward, ToImpact) < 0.f;
}

void UPlayerCombatInteractionComponent::StartBlock()
{
	bBlockKeyHeld = true;

	// Late input: pending hit → 즉시 패링 처리
	if (PendingHitPayload.IsSet())
	{
		GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
		SetDefenceState(EDefenceState::Parrying);
		ApplyDamage(PendingHitPayload.GetValue());
		PendingHitPayload.Reset();
		return;
	}

	SetDefenceState(EDefenceState::Parrying);

	// BlockStart 몽타주 재생 (AnimNotifyState_ParryWindow 포함)
	PlayDefenceMontage(BlockStartMontage);
}

void UPlayerCombatInteractionComponent::StopBlock()
{
	bBlockKeyHeld = false;

	// 회피 판정 구간 완주 후 OnDodgeActiveEnd 에서 복귀
	if (DefenceState == EDefenceState::Dodging)
		return;

	SetDefenceState(EDefenceState::None);
}

// ── AnimNotify callbacks ─────────────────────────────────────

void UPlayerCombatInteractionComponent::OnParryWindowBegin()
{
	// 선택적: StartBlock 에서 이미 Parrying 세팅했으므로 보통 불필요
}

void UPlayerCombatInteractionComponent::OnParryWindowEnd()
{
	if (DefenceState != EDefenceState::Parrying)
		return;

	SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
}

void UPlayerCombatInteractionComponent::OnDodgeActiveBegin()
{
	// 선택적: ExecuteBlockDodge 에서 이미 Dodging 세팅
}

void UPlayerCombatInteractionComponent::OnDodgeActiveEnd()
{
	if (DefenceState != EDefenceState::Dodging)
		return;

	SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
}

void UPlayerCombatInteractionComponent::OnDodgeCooldownEnd()
{
	bCanDodge = true;
}

// ── Dodge ────────────────────────────────────────────────────

void UPlayerCombatInteractionComponent::TryBlockDodge(const FInputActionValue& Value)
{
	const bool bInBlocking = (DefenceState == EDefenceState::Blocking);
	const bool bLateDodge = PendingHitPayload.IsSet();

	if ((!bInBlocking && !bLateDodge) || !bCanDodge)
		return;

	const FGameplayTag DodgeTag = (Value.Get<FVector2D>().X > 0.5f)
		                              ? GameplayTag::Combat_State_Dodge_Up
		                              : GameplayTag::Combat_State_Dodge_Down;

	if (bLateDodge)
	{
		GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
		SetDefenceState(EDefenceState::Dodging);
		ApplyDamage(PendingHitPayload.GetValue());
		PendingHitPayload.Reset();
	}

	ExecuteBlockDodge(DodgeTag);
}

void UPlayerCombatInteractionComponent::ExecuteBlockDodge(FGameplayTag DodgeStateTag)
{
	bCanDodge = false;
	SetDefenceState(EDefenceState::Dodging);

	UAnimMontage* Montage = (DodgeStateTag == GameplayTag::Combat_State_Dodge_Up)
		                        ? DodgeUpMontage.Get()
		                        : DodgeDownMontage.Get();

	if (!PlayDefenceMontage(Montage))
	{
		bCanDodge = true;
		SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
	}
}

// ── Hit Reaction ─────────────────────────────────────────────

FVector2D UPlayerCombatInteractionComponent::ComputeHitDirection(const FVector& ImpactLocation) const
{
	const AActor* Owner = GetOwner();
	if (ImpactLocation.IsZero() || !Owner)
		return FVector2D::ZeroVector;

	const FVector ToImpact = (ImpactLocation - Owner->GetActorLocation()).GetSafeNormal();
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Right = Owner->GetActorRightVector();

	// X = Right (positive = hit from right), Y = Forward (positive = hit from front)
	return FVector2D(
		FVector::DotProduct(Right, ToImpact),
		FVector::DotProduct(Forward, ToImpact)
	);
}

void UPlayerCombatInteractionComponent::SetHitReaction(EHitReactionType Type, const FVector& ImpactLocation)
{
	HitReactionType = Type;
	HitDirection = ComputeHitDirection(ImpactLocation);

	UAnimMontage* Montage = (Type == EHitReactionType::BlockHit)
		                        ? BlockHitMontage.Get()
		                        : HitMontage.Get();

	PlayDefenceMontage(Montage);
}

void UPlayerCombatInteractionComponent::OnHitReactionEnd()
{
	HitReactionType = EHitReactionType::None;
	HitDirection = FVector2D::ZeroVector;
}

// ── Montage Helper ──────────────────────────────────────────

bool UPlayerCombatInteractionComponent::PlayDefenceMontage(UAnimMontage* Montage)
{
	if (!Montage) return false;

	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return false;

	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return false;

	return AnimInstance->Montage_Play(Montage) > 0.f;
}

// ── Late Input Buffer ────────────────────────────────────────

void UPlayerCombatInteractionComponent::ApplyPendingHitDamage()
{
	if (!PendingHitPayload.IsSet())
		return;

	// 타이머 만료 → 실제 피해 적용 (None 상태에서 재처리)
	const FAttackPayload Payload = PendingHitPayload.GetValue();
	PendingHitPayload.Reset();

	// 직접 피해 적용 (Health)
	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive, -Payload.HealthDamage);

	// Structure 피해
	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetStructureAttribute(),
		EGameplayModOp::Additive, Payload.StructureDamage);

	SetHitReaction(EHitReactionType::Hit, Payload.ImpactLocation);
}
