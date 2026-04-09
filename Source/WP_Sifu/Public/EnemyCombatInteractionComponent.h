#pragma once

#include "CoreMinimal.h"
#include "CombatInteractionComponentBase.h"
#include "EnemyCombatInteractionComponent.generated.h"

UENUM(BlueprintType)
enum class EEnemyReactionType : uint8
{
	None UMETA(DisplayName="None"),
	Hit UMETA(DisplayName="Hit"),
	Block UMETA(DisplayName="Block"),
	Stun UMETA(DisplayName="Stun"),
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UEnemyCombatInteractionComponent : public UCombatInteractionComponentBase
{
	GENERATED_BODY()

public:
	UEnemyCombatInteractionComponent();

	UFUNCTION(BlueprintPure, Category=Combat)
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category=Combat)
	EEnemyReactionType GetReactionType() const { return ReactionType; }

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual EAttackResponse ApplyDamage(const FAttackPayload& Payload) override;

private:
	UFUNCTION()
	void HandleStructureChanged(class UAttributeSet* AttributeSet, float OldValue, float NewValue);

	void ApplyHitDamage(const FAttackPayload& Payload);
	bool ShouldBlockAttack(const FAttackPayload& Payload) const;
	void EnterStun();
	void ExitStun();
	void BreakStun();
	bool PlayReactionMontage(class UAnimMontage* Montage, const TArray<FName>& Sections);
	void SetReaction(EEnemyReactionType NewReaction) { ReactionType = NewReaction; }

private:
	UPROPERTY(EditAnywhere, Category=Combat, meta=(ClampMin="0.0", ClampMax="1.0"))
	float BlockChance = 0.35f;

	UPROPERTY(EditAnywhere, Category=Combat, meta=(ClampMin="0.1"))
	float StunDuration = 3.5f;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> BlockMontage;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<class UAnimMontage> StunMontage;

	UPROPERTY(EditAnywhere, Category=Animation)
	TArray<FName> HitSections;

	UPROPERTY(EditAnywhere, Category=Animation)
	TArray<FName> BlockSections;

	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY()
	TObjectPtr<class UHealthAttributeSet> HealthAttributes;

	UPROPERTY(VisibleAnywhere, Category=Combat)
	bool bIsStunned = false;

	UPROPERTY(VisibleAnywhere, Category=Combat)
	EEnemyReactionType ReactionType = EEnemyReactionType::None;

	FTimerHandle StunTimerHandle;
};
