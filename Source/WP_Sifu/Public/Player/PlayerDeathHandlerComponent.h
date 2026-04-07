// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathHandlerComponentBase.h"
#include "PlayerDeathHandlerComponent.generated.h"

class UPlayerDeathScreenWidget;

/**
 * Player death handler: plays aging overlay, increments age, then revives.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class WP_SIFU_API UPlayerDeathHandlerComponent : public UDeathHandlerComponentBase
{
	GENERATED_BODY()

public:
	UPlayerDeathHandlerComponent();

	UPROPERTY(EditDefaultsOnly, Category=Death)
	TObjectPtr<UAnimMontage> ReviveMontage;

	UPROPERTY(EditDefaultsOnly, Category=Death)
	TSubclassOf<UPlayerDeathScreenWidget> DeathScreenWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Death)
	int32 DeathCount = 0;

	UPROPERTY(EditDefaultsOnly, Category=Death, meta=(ClampMin="0.01", ClampMax="1.0"))
	float DeathTimeDilation = 0.2f;

	/** Called from the death screen widget when its animation is complete */
	UFUNCTION(BlueprintCallable, Category=Death)
	void OnAgingComplete();

protected:
	/** Show widget + set time dilation immediately when death starts */
	virtual void OnDeathBegin() override;

private:
	UFUNCTION()
	void OnReviveMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void RestoreHealth();

	UPROPERTY()
	TObjectPtr<UPlayerDeathScreenWidget> DeathScreenInstance;
};
