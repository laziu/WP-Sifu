// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatInteractionComponent.h"

#include "CameraFocusComponent.h"
#include "DeathHandlerComponentBase.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UserExtension.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTags.generated.h"
#include "HealthAttributeSet.h"
#include "PlayerAttackComponent.h"
#include "WP_Sifu.h"
#include "Camera/CameraShakeBase.h"
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
	Ext::SetObject(
		StructureBrokenMontage,
		TEXT(
			"/Script/Engine.AnimMontage'/Game/ART_FROM_SIFU/Player/SK_M_MainChar_01/Anims/Montages/AM_MainChar_Broken.AM_MainChar_Broken'"));
}

void UPlayerCombatInteractionComponent::SetupInputBindings(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(InputBlock, ETriggerEvent::Started, this, &UPlayerCombatInteractionComponent::StartBlock);
	EIC->BindAction(InputBlock, ETriggerEvent::Completed, this, &UPlayerCombatInteractionComponent::StopBlock);
	EIC->BindAction(InputMove, ETriggerEvent::Started, this, &UPlayerCombatInteractionComponent::TryBlockDodge);
}

FAttackPayload UPlayerCombatInteractionComponent::MakeCurrentAttackPayload() const
{
	if (const auto* AttackComp = GetOwner()->FindComponentByClass<UPlayerAttackComponent>())
	{
		return AttackComp->MakeCurrentAttackPayload();
	}
	return Super::MakeCurrentAttackPayload();
}

void UPlayerCombatInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const auto ASI = CastChecked<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComp = ASI->GetAbilitySystemComponent();
}

void UPlayerCombatInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		DeathComp->OnDeathStarted.AddDynamic(this, &UPlayerCombatInteractionComponent::ResetCombatState);
	}

	if (const auto* HealthAttr = AbilitySystemComp->GetSet<UHealthAttributeSet>())
	{
		const_cast<UHealthAttributeSet*>(HealthAttr)->OnStructureChanged.AddDynamic(
			this, &UPlayerCombatInteractionComponent::HandleStructureChanged);
	}

	// Attacker-side camera shake: fires when our attack lands on an enemy
	OnAttackSent.AddDynamic(this, &UPlayerCombatInteractionComponent::HandleAttackSent);
}

void UPlayerCombatInteractionComponent::ResetCombatState()
{
	SetDefenceState(EDefenceState::None);
	bBlockKeyHeld = false;
	bHitRecoveryMovementLock = false;
	bStructureBroken = false;
	HitReactionType = EHitReactionType::None;
	HitDirection = FVector2D::ZeroVector;
	PendingHitPayload.Reset();
	GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(HitRecoveryTimer);
	GetWorld()->GetTimerManager().ClearTimer(DodgeCooldownTimer);
	GetWorld()->GetTimerManager().ClearTimer(BlockCooldownTimer);
	bCanDodge = true;
	bCanBlock = true;
}

EAttackResponse UPlayerCombatInteractionComponent::ApplyDamage(const FAttackPayload& Payload)
{
	// Structure broken → 즉시 Idle 복귀 + 피격 리액션
	if (bStructureBroken)
	{
		bStructureBroken = false;
		AbilitySystemComp->SetNumericAttributeBase(UHealthAttributeSet::GetStructureAttribute(), 0.f);
		SetDefenceState(EDefenceState::None);

		AbilitySystemComp->ApplyModToAttribute(
			UHealthAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive, -Payload.HealthDamage);
		AbilitySystemComp->ApplyModToAttribute(
			UHealthAttributeSet::GetStructureAttribute(),
			EGameplayModOp::Additive, Payload.StructureDamage);

		if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead()) return EAttackResponse::Hit;
		}

		SetHitReaction(EHitReactionType::Hit, Payload.ImpactLocation);

		if (Payload.Instigator.IsValid())
		{
			if (auto* FocusComp = GetOwner()->FindComponentByClass<UCameraFocusComponent>())
			{
				FocusComp->SetFocusTarget(Payload.Instigator.Get());
			}
		}

		return EAttackResponse::Hit;
	}

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

			if (!bStructureBroken)
			{
				SetHitReaction(EHitReactionType::BlockHit, Payload.ImpactLocation);
			}

		// Transfer camera focus to the attacker
		if (Payload.Instigator.IsValid())
		{
			if (auto* FocusComp = GetOwner()->FindComponentByClass<UCameraFocusComponent>())
			{
				FocusComp->SetFocusTarget(Payload.Instigator.Get());
			}
		}

		PlayCameraShake(DefendBlockShake);
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

		PlayCameraShake(DefendParryShake);
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

	// 쿨다운 중에는 몽타주 없이 Blocking 상태로 전환 (Parry Window 방지)
	if (!bCanBlock)
	{
		SetDefenceState(EDefenceState::Blocking);
		return;
	}

	SetDefenceState(EDefenceState::Parrying);

	// BlockStart 몽타주 재생 (AnimNotifyState_ParryWindow 포함)
	if (PlayDefenceMontage(BlockStartMontage))
	{
		bCanBlock = false;
		GetWorld()->GetTimerManager().SetTimer(
			BlockCooldownTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				bCanBlock = true;
			}),
			BlockCooldown, false);
	}
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
	// 타이머 기반으로 전환됨 — AnimNotify 호환용으로 유지
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
		GetWorld()->GetTimerManager().ClearTimer(DodgeCooldownTimer);
		SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
		return;
	}

	// AnimNotify 대신 타이머로 쿨다운 관리 (몽타주 중단 시에도 확실히 복구)
	GetWorld()->GetTimerManager().SetTimer(
		DodgeCooldownTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			bCanDodge = true;
		}),
		DodgeCooldown, false);
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
	if (Type == EHitReactionType::Hit)
	{
		bHitRecoveryMovementLock = true;
		GetWorld()->GetTimerManager().SetTimer(
			HitRecoveryTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				bHitRecoveryMovementLock = false;
			}),
			HitRecoveryLockDuration, false);
	}

	UAnimMontage* Montage = (Type == EHitReactionType::BlockHit)
		                        ? BlockHitMontage.Get()
		                        : HitMontage.Get();

	PlayDefenceMontage(Montage);
}

void UPlayerCombatInteractionComponent::OnHitReactionEnd()
{
	if (bStructureBroken)
	{
		bStructureBroken = false;
		AbilitySystemComp->SetNumericAttributeBase(UHealthAttributeSet::GetStructureAttribute(), 0.f);
	}

	HitReactionType = EHitReactionType::None;
	HitDirection = FVector2D::ZeroVector;
}

bool UPlayerCombatInteractionComponent::IsMovementBlocked() const
{
	return DefenceState == EDefenceState::Blocking || bHitRecoveryMovementLock || bStructureBroken;
}

// ── Structure Broken ─────────────────────────────────────────

void UPlayerCombatInteractionComponent::HandleStructureChanged(UAttributeSet* AttributeSet, float OldValue, float NewValue)
{
	const auto* HealthAttr = Cast<UHealthAttributeSet>(AttributeSet);
	if (!HealthAttr || bStructureBroken)
		return;

	if (NewValue >= HealthAttr->GetMaxStructure() && OldValue < HealthAttr->GetMaxStructure())
	{
		OnStructureBroken();
	}
}

void UPlayerCombatInteractionComponent::OnStructureBroken()
{
	bStructureBroken = true;

	// 진행 중인 피격 관련 타이머 취소
	GetWorld()->GetTimerManager().ClearTimer(HitRecoveryTimer);
	bHitRecoveryMovementLock = false;
	GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
	PendingHitPayload.Reset();

	HitReactionType = EHitReactionType::Broken;
	HitDirection = FVector2D::ZeroVector;

	PlayDefenceMontage(StructureBrokenMontage);
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

	// Skip hit reaction if the damage killed the character or triggered structure break
	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		if (DeathComp->IsDead()) return;
	}

	if (bStructureBroken) return;

	SetHitReaction(EHitReactionType::Hit, Payload.ImpactLocation);

	// Transfer camera focus to the attacker
	if (Payload.Instigator.IsValid())
	{
		if (auto* FocusComp = GetOwner()->FindComponentByClass<UCameraFocusComponent>())
		{
			FocusComp->SetFocusTarget(Payload.Instigator.Get());
		}
	}
}

// ── Camera Shake ─────────────────────────────────────────────

void UPlayerCombatInteractionComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass) const
{
	if (!ShakeClass) return;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ClientStartCameraShake(ShakeClass, ShakeScale);
	}
}

void UPlayerCombatInteractionComponent::HandleAttackSent(AActor* Target, const FAttackPayload& Payload, EAttackResponse Response)
{
	// Skip reflected/structure-only attacks (e.g., parry reflect)
	if (Payload.HealthDamage <= 0.f) return;

	switch (Response)
	{
	case EAttackResponse::Hit:
		PlayCameraShake(AttackHitShake);
		break;
	case EAttackResponse::Block:
		PlayCameraShake(AttackBlockedShake);
		break;
	default:
		break;
	}
}

