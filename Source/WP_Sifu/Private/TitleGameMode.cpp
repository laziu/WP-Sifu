// Fill out your copyright notice in the Description page of Project Settings.

#include "TitleGameMode.h"

#include "TitleWidget.h"
#include "WP_GameInstance.h"
#include "WP_Sifu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ATitleGameMode::ATitleGameMode()
{
	// No player pawn needed on the title screen.
	DefaultPawnClass = nullptr;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	if (TitleWidgetClass)
	{
		TitleWidgetInstance = CreateWidget<UTitleWidget>(PC, TitleWidgetClass);
		if (TitleWidgetInstance)
		{
			TitleWidgetInstance->AddToViewport();
		}
	}

	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());

	// Hide loading screen (applies when returning from MainStage → Title).
	if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
	{
		GI->HideLoadingScreen();
	}
}

void ATitleGameMode::RequestStartGame()
{
	if (TitleWidgetInstance)
	{
		TitleWidgetInstance->PlayFadeOut();
	}

	// Wait for the fade-out animation (0.5s), then transition.
	GetWorldTimerManager().SetTimer(
		FadeOutTimerHandle,
		this, &ATitleGameMode::OnFadeOutTimerComplete,
		1.f, false);
}

void ATitleGameMode::OnFadeOutTimerComplete()
{
	if (UWP_GameInstance* GI = GetGameInstance<UWP_GameInstance>())
	{
		GI->ShowLoadingScreenAndOpenLevel(TEXT("Lvl_MainStage"), true);
	}
}

void ATitleGameMode::RequestQuitGame()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetWorld()->GetFirstPlayerController(),
		EQuitPreference::Quit,
		false);
}
