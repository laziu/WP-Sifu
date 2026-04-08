#include "EnemyCombatInteractionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DeathHandlerComponentBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthAttributeSet.h"
#include "TimerManager.h"

UEnemyCombatInteractionComponent::UEnemyCombatInteractionComponent()
{
	bWantsInitializeComponent = true;

	HitSections = {TEXT("Hit_A"), TEXT("Hit_B"), TEXT("Hit_C")};
	BlockSections = {TEXT("Block_A"), TEXT("Block_B"), TEXT("Block_C")};
}

void UEnemyCombatInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const auto* ASI = CastChecked<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComp = ASI->GetAbilitySystemComponent();
	HealthAttributes = AbilitySystemComp
		                   ? const_cast<UHealthAttributeSet*>(AbilitySystemComp->GetSet<UHealthAttributeSet>())
		                   : nullptr;
}

void UEnemyCombatInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HealthAttributes)
	{
		HealthAttributes->OnStructureChanged.AddDynamic(this, &UEnemyCombatInteractionComponent::HandleStructureChanged);
	}
}

EAttackResponse UEnemyCombatInteractionComponent::ApplyDamage(const FAttackPayload& Payload)
{
	if (!AbilitySystemComp || !HealthAttributes)
	{
		return EAttackResponse::Ignore;
	}

	if (bIsStunned)
	{
		BreakStun();
		ApplyHitDamage(Payload);

		if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead())
			{
				return EAttackResponse::Hit;
			}
		}

		SetReaction(EEnemyReactionType::Hit);
		PlayReactionMontage(HitMontage, HitSections);
		return EAttackResponse::Hit;
	}

	if (ShouldBlockAttack(Payload))
	{
		AbilitySystemComp->ApplyModToAttribute(
			UHealthAttributeSet::GetStructureAttribute(),
			EGameplayModOp::Additive,
			Payload.StructureDamage);

		if (!bIsStunned)
		{
			SetReaction(EEnemyReactionType::Block);
			PlayReactionMontage(BlockMontage, BlockSections);
		}

		return EAttackResponse::Block;
	}

	ApplyHitDamage(Payload);

	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		if (DeathComp->IsDead())
		{
			return EAttackResponse::Hit;
		}
	}

	if (!bIsStunned)
	{
		SetReaction(EEnemyReactionType::Hit);
		PlayReactionMontage(HitMontage, HitSections);
	}

	return EAttackResponse::Hit;
}

void UEnemyCombatInteractionComponent::HandleStructureChanged(UAttributeSet* AttributeSet, float OldValue, float NewValue)
{
	if (!HealthAttributes || bIsStunned)
	{
		return;
	}

	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		if (DeathComp->IsDead())
		{
			return;
		}
	}

	if (NewValue >= HealthAttributes->GetMaxStructure() && OldValue < HealthAttributes->GetMaxStructure())
	{
		EnterStun();
	}
}

void UEnemyCombatInteractionComponent::ApplyHitDamage(const FAttackPayload& Payload)
{
	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-Payload.HealthDamage);

	AbilitySystemComp->ApplyModToAttribute(
		UHealthAttributeSet::GetStructureAttribute(),
		EGameplayModOp::Additive,
		Payload.StructureDamage);
}

bool UEnemyCombatInteractionComponent::ShouldBlockAttack(const FAttackPayload& Payload) const
{
	if (bIsStunned || Payload.bUnblockable)
	{
		return false;
	}

	if (auto* DeathComp = GetOwner()->FindComponentByClass<UDeathHandlerComponentBase>())
	{
		if (DeathComp->IsDead())
		{
			return false;
		}
	}

	return FMath::FRand() <= BlockChance;
}

void UEnemyCombatInteractionComponent::EnterStun()
{
	if (bIsStunned)
	{
		return;
	}

	bIsStunned = true;
	SetReaction(EEnemyReactionType::Stun);

	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->DisableMovement();

		if (auto* Controller = Cast<AAIController>(Character->GetController()))
		{
			Controller->StopMovement();
		}
	}

	PlayReactionMontage(StunMontage, {});

	GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&UEnemyCombatInteractionComponent::ExitStun,
		StunDuration,
		false);
}

void UEnemyCombatInteractionComponent::ExitStun()
{
	bIsStunned = false;
	SetReaction(EEnemyReactionType::None);

	if (AbilitySystemComp && HealthAttributes)
	{
		const float CurrentStructure = HealthAttributes->GetStructure();
		if (CurrentStructure > 0.f)
		{
			AbilitySystemComp->ApplyModToAttribute(
				UHealthAttributeSet::GetStructureAttribute(),
				EGameplayModOp::Additive,
				-CurrentStructure);
		}
	}

	if (auto* Character = Cast<ACharacter>(GetOwner()))
	{
		if (auto* DeathComp = Character->FindComponentByClass<UDeathHandlerComponentBase>())
		{
			if (DeathComp->IsDead())
			{
				return;
			}
		}

		if (auto* Movement = Character->GetCharacterMovement())
		{
			Movement->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}
}

void UEnemyCombatInteractionComponent::BreakStun()
{
	if (!bIsStunned)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
	ExitStun();
}

bool UEnemyCombatInteractionComponent::PlayReactionMontage(UAnimMontage* Montage, const TArray<FName>& Sections)
{
	if (!Montage)
	{
		return false;
	}

	auto* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return false;
	}

	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	if (AnimInstance->Montage_Play(Montage) <= 0.f)
	{
		return false;
	}

	if (!Sections.IsEmpty())
	{
		const int32 Index = FMath::RandRange(0, Sections.Num() - 1);
		AnimInstance->Montage_JumpToSection(Sections[Index], Montage);
	}

	return true;
}
