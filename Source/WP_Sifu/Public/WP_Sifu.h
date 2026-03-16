// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(Sifu, Log, All);

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define LOG_CALLINFO() UE_LOG(Sifu, Warning, TEXT("%s"), *CALLINFO)
#define LOG_TEXT(cat, fmt, ...) UE_LOG(Sifu, cat, TEXT("%s: %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))
#define LOGE(fmt, ...) LOG_TEXT(Error, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG_TEXT(Warning, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG_TEXT(Display, fmt, ##__VA_ARGS__)
