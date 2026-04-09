// Fill out your copyright notice in the Description page of Project Settings.

#include "WP_GameInstance.h"

#include "LoadingScreenWidget.h"
#include "WP_SaveGame.h"
#include "WP_Sifu.h"
#include "Kismet/GameplayStatics.h"


void UWP_GameInstance::Init()
{
	Super::Init();

	if (UWP_SaveGame* Save = Cast<UWP_SaveGame>(
		UGameplayStatics::LoadGameFromSlot(UWP_SaveGame::SaveSlotName, UWP_SaveGame::SaveUserIndex)))
	{
		BestDeathCount = Save->BestDeathCount;
		LOGD(TEXT("Loaded best record: %d"), BestDeathCount);
	}
}

void UWP_GameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	// Invalidate the old widget reference — it was destroyed with the old world.
	LoadingWidgetInstance = nullptr;

	if (bLoadingScreenPending && IsValid(NewWorld))
	{
		// Re-attach the loading screen to the new world, instantly opaque.
		CreateAndShowLoadingWidget(false);
	}
}

void UWP_GameInstance::ShowLoadingScreen(bool bWithFadeIn)
{
	// Remove any existing widget before creating a fresh one.
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->RemoveFromParent();
		LoadingWidgetInstance = nullptr;
	}

	bLoadingScreenPending = true;
	CreateAndShowLoadingWidget(bWithFadeIn);
}

void UWP_GameInstance::ShowLoadingScreenAndOpenLevel(FName LevelName, bool bWithFadeIn)
{
	ShowLoadingScreen(bWithFadeIn);

	PendingLevelName = LevelName;

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &UWP_GameInstance::OnLoadingDelayComplete);
		World->GetTimerManager().SetTimer(LoadingScreenDelayTimer, TimerDel, MinLoadingScreenSeconds, false);
	}
}

void UWP_GameInstance::HideLoadingScreen()
{
	bLoadingScreenPending = false;

	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->PlayFadeOut();
		LoadingWidgetInstance = nullptr;
	}
}

void UWP_GameInstance::OnLoadingDelayComplete()
{
	if (!PendingLevelName.IsNone())
	{
		FName LevelToOpen = PendingLevelName;
		PendingLevelName = NAME_None;

		// Clear pending flag so OnWorldChanged won't re-create the widget in the new world.
		bLoadingScreenPending = false;

		UGameplayStatics::OpenLevel(this, LevelToOpen);
	}
}

void UWP_GameInstance::UpdateBestRecord(int32 NewDeathCount)
{
	if (NewDeathCount >= BestDeathCount)
	{
		return;
	}

	BestDeathCount = NewDeathCount;
	LOGD(TEXT("New best record: %d"), BestDeathCount);

	UWP_SaveGame* Save = Cast<UWP_SaveGame>(
		UGameplayStatics::CreateSaveGameObject(UWP_SaveGame::StaticClass()));
	Save->BestDeathCount = BestDeathCount;
	UGameplayStatics::SaveGameToSlot(Save, UWP_SaveGame::SaveSlotName, UWP_SaveGame::SaveUserIndex);
}

void UWP_GameInstance::CreateAndShowLoadingWidget(bool bPlayFadeIn)
{
	if (!LoadingWidgetClass)
	{
		LOGW(TEXT("LoadingWidgetClass is not set on the GameInstance."));
		return;
	}

	// Use GameInstance as the owning object so the widget can be created
	// even before a PlayerController exists in the new world.
	LoadingWidgetInstance = CreateWidget<ULoadingScreenWidget>(this, LoadingWidgetClass);
	if (!LoadingWidgetInstance)
	{
		return;
	}

	// ZOrder 10: always on top of game HUD.
	LoadingWidgetInstance->AddToViewport(10);

	if (bPlayFadeIn)
	{
		LoadingWidgetInstance->PlayFadeIn();
	}
}
