// Fill out your copyright notice in the Description page of Project Settings.

#include "MainStageGameMode.h"

#include "EngineUtils.h"
#include "Enemybase.h"
#include "EnemyDeathHandlerComponent.h"
#include "MainCharacter.h"
#include "MainCharPlayerController.h"
#include "PlayerDeathHandlerComponent.h"
#include "StageClearWidget.h"
#include "StageIntroWidget.h"
#include "WP_GameInstance.h"
#include "WP_Sifu.h"
#include "UserExtension.h"
#include "Camera/CameraActor.h"
#include "CineCameraActor.h"
#include "Kismet/GameplayStatics.h"


AMainStageGameMode::AMainStageGameMode()
{
	// Use BP_MainCharacter so AI Perception (StimuliSourceComponent) is active on the pawn.
	Ext::SetClass(DefaultPawnClass, TEXT("/Game/Blueprints/Player/BP_MainCharacter"));
	PlayerControllerClass = AMainCharPlayerController::StaticClass();
}

void AMainStageGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Count alive enemies and bind death delegates.
	for (TActorIterator<AEnemybase> It(GetWorld()); It; ++It)
	{
		if (It->DeathHandler)
		{
			AliveEnemyCount++;
			It->DeathHandler->OnDeathStarted.AddDynamic(this, &AMainStageGameMode::OnEnemyDied);
		}
	}
	LOGW(TEXT("Alive enemies at start: %d"), AliveEnemyCount);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	// Disable player input for the duration of the intro.
	if (Pawn)
	{
		Pawn->DisableInput(PC);
	}

	// Find the intro camera by tag if not assigned in the editor.
	if (!IntroCameraActor)
	{
		for (TActorIterator<ACineCameraActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(TEXT("IntroCamera")))
			{
				IntroCameraActor = *It;
				break;
			}
		}
	}

	if (IntroCameraActor && PC)
	{
		PC->SetViewTarget(IntroCameraActor);
	}

	StartIntroSequence();

	// Hide the loading screen now that the level is ready.
	if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
	{
		GI->HideLoadingScreen();
	}
}

// ---------------------------------------------------------------------------
// Intro sequence
// ---------------------------------------------------------------------------

void AMainStageGameMode::StartIntroSequence()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (StageIntroWidgetClass)
	{
		StageIntroWidgetInstance = CreateWidget<UStageIntroWidget>(PC, StageIntroWidgetClass);
		if (StageIntroWidgetInstance)
		{
			int32 Best = INT32_MAX;
			if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
			{
				Best = GI->GetBestDeathCount();
			}
			StageIntroWidgetInstance->BestDeathCount = Best;
			StageIntroWidgetInstance->AddToViewport();
			StageIntroWidgetInstance->PlayIntroAnim();
		}
	}

	GetWorldTimerManager().SetTimer(
		IntroDurationTimer,
		this, &AMainStageGameMode::BeginCameraTransition,
		IntroDurationSeconds, false);
}

void AMainStageGameMode::BeginCameraTransition()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!PC || !Pawn) return;

	// Blend to the player camera.
	PC->SetViewTargetWithBlend(Pawn, CameraBlendSeconds, VTBlend_EaseInOut, CameraBlendExponent);

	// Simultaneously animate out the intro overlay.
	if (StageIntroWidgetInstance)
	{
		StageIntroWidgetInstance->PlayOutroAnim();
	}

	GetWorldTimerManager().SetTimer(
		CameraBlendTimer,
		this, &AMainStageGameMode::OnIntroComplete,
		CameraBlendSeconds, false);
}

void AMainStageGameMode::OnIntroComplete()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	// Hand control back to the player.
	if (Pawn)
	{
		Pawn->EnableInput(PC);
	}
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	if (StageIntroWidgetInstance)
	{
		StageIntroWidgetInstance->RemoveFromParent();
		StageIntroWidgetInstance = nullptr;
	}
}

// ---------------------------------------------------------------------------
// Enemy tracking
// ---------------------------------------------------------------------------

void AMainStageGameMode::OnEnemyDied()
{
	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
	LOGW(TEXT("Enemy died. Remaining: %d"), AliveEnemyCount);
	if (AliveEnemyCount == 0)
	{
		OnStageClear();
	}
}

// ---------------------------------------------------------------------------
// Stage clear
// ---------------------------------------------------------------------------

void AMainStageGameMode::OnStageClear()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	// Block all player input.
	if (Pawn)
	{
		Pawn->DisableInput(PC);
	}

	if (!StageClearWidgetClass || !PC) return;

	// Read the death count from the player character.
	int32 DeathCount = 0;
	if (AMainCharacter* MainChar = Cast<AMainCharacter>(Pawn))
	{
		DeathCount = MainChar->PlayerDeathHandler->DeathCount;
	}

	UStageClearWidget* ClearWidget = CreateWidget<UStageClearWidget>(PC, StageClearWidgetClass);
	if (!ClearWidget) return;

	ClearWidget->DeathCount = DeathCount;
	ClearWidget->AddToViewport();
	ClearWidget->PlayFadeIn();

	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());
}

void AMainStageGameMode::RestartStage()
{
	TransitionToLevel(TEXT("Lvl_MainStage"));
}

void AMainStageGameMode::ReturnToTitle()
{
	TransitionToLevel(TEXT("Lvl_Title"));
}

void AMainStageGameMode::TransitionToLevel(FName LevelName)
{
	// Restore world speed before the level unloads.
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// Save the best record.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (AMainCharacter* MainChar = Cast<AMainCharacter>(Pawn))
	{
		if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
		{
			GI->UpdateBestRecord(MainChar->PlayerDeathHandler->DeathCount);
		}
	}

	// Show the loading screen immediately (no fade-in: instant blackout).
	if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
	{
		GI->ShowLoadingScreen(false);
	}

	UGameplayStatics::OpenLevel(this, LevelName);
}
