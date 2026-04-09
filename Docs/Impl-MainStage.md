# 메인 스테이지 구현 문서

`Plan-MainStage.md`를 바탕으로 한 상세 구현 계획.

---

## 1. 확정된 설계 사항

| 항목 | 결정 |
|---|---|
| "최고 기록" 정의 | 게임 세션 전체의 역대 최저 DeathCount (베스트 기록) |
| "로드 중" 화면 귀속 | GameInstance 글로벌 위젯 — 레벨 무관하게 표시 |
| 타이틀 위젯 | 신규 생성 필요 (`WBP_Title`) |
| 시네마틱 카메라 | `Lvl_MainStage`에 `CineCameraActor` 수동 배치 |
| 다시 시작 | `Lvl_MainStage` 언로드 후 재로드 (첫 로드 흐름과 동일) |

---

## 2. 기존 코드와의 연관성

| 기존 요소 | 활용 방식 |
|---|---|
| `UDeathHandlerComponentBase::OnDeathFinished` | `AMainStageGameMode`가 각 Enemy에 bind해 적 사망 추적 |
| `UPlayerDeathHandlerComponent::DeathCount` | 스테이지 완료 시 `GameInstance`로 전달 |
| `AMainCharPlayerController` | 인트로 중 `DisableInput` / 완료 후 `EnableInput` 호출 |
| `UThirdPersonCameraComponent::Camera` | 인트로 카메라 → 플레이어 카메라 블렌드의 타겟 |

---

## 3. 신규 C++ 클래스

### 3-1. `UWP_SaveGame` (`Source/WP_Sifu/Public/WP_SaveGame.h`)

베스트 기록을 게임 세션 간 영속 저장하기 위한 SaveGame 클래스.

```cpp
UCLASS()
class UWP_SaveGame : public USaveGame
{
    UPROPERTY() int32 BestDeathCount = INT_MAX;
};
```

- 슬롯명: `"WP_SifuSave"`, 인덱스: `0`

---

### 3-2. `UWP_GameInstance` (`Source/WP_Sifu/Public/WP_GameInstance.h`)

레벨 전환에도 유지되는 글로벌 상태 및 로딩 화면 관리.

**주요 역할**
- 역대 최저 DeathCount 저장/로드
- 로딩 화면 위젯을 레벨 전환 후에도 유지

**프로퍼티**
```cpp
// BP_WP_GameInstance에서 설정
UPROPERTY(EditDefaultsOnly, Category=UI) TSubclassOf<UUserWidget> LoadingWidgetClass;

// 런타임
int32 BestDeathCount = INT_MAX;
bool bLoadingScreenPending = false;
TObjectPtr<UUserWidget> LoadingWidgetInstance;
```

**주요 메서드**
```cpp
// 로딩 화면을 바로 혹은 다음 월드 전환 시 표시
void ShowLoadingScreen();

// 로딩 화면 FadeOut + 제거
void HideLoadingScreen();

// 역대 최저 기록 갱신 (더 낮을 때만 업데이트) + SaveGame 즉시 저장
void UpdateBestRecord(int32 NewDeathCount);

int32 GetBestDeathCount() const;

// 월드 전환 시 WBP_Loading 재부착
virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
```

**구현 포인트**
- `Init()` 시점에 `UGameplayStatics::LoadGameFromSlot("WP_SifuSave", 0)`으로 `BestDeathCount`를 복원  
- `UpdateBestRecord()`는 갱신 즉시 `UGameplayStatics::SaveGameToSlot()` 호출  
- `ShowLoadingScreen()` 호출 시점에 World가 이미 있으면 즉시 위젯 생성  
- `bLoadingScreenPending = true`로 설정해두면 `OnWorldChanged()`에서 새 World 생성 직후 자동으로 위젯 재생성  
- `HideLoadingScreen()`은 위젯의 FadeOut 애니메이션 완료 뒤 `RemoveFromParent()` 호출

---

### 3-3. `ATitleGameMode` (`Source/WP_Sifu/Public/Player/TitleGameMode.h`)

`Lvl_Title`용 GameMode. `WBP_Title`의 버튼 이벤트를 처리.

**주요 메서드**
```cpp
UFUNCTION(BlueprintCallable) void RequestStartGame(); // 시작 버튼
UFUNCTION(BlueprintCallable) void RequestQuitGame();  // 종료 버튼
```

**`RequestStartGame()` 흐름**
1. `WBP_Title`의 FadeOut 애니메이션 재생 (0.5초)  
2. 완료 후 `GameInstance->ShowLoadingScreen()` 호출  
3. `UGameplayStatics::OpenLevel(this, "Lvl_MainStage")` 호출

**`BeginPlay()` 흐름**
1. `WBP_Title` 생성 + `AddToViewport`  
2. `InputMode: UI Only` 설정 (마우스 커서 표시)

**프로퍼티**
```cpp
UPROPERTY(EditDefaultsOnly, Category=UI) TSubclassOf<UUserWidget> TitleWidgetClass;
```

---

### 3-4. `AMainStageGameMode` (`Source/WP_Sifu/Public/Player/MainStageGameMode.h`)

`Lvl_MainStage`용 GameMode. 인트로 시퀀스, 적 추적, 스테이지 클리어를 관장.

**프로퍼티**
```cpp
UPROPERTY(EditDefaultsOnly, Category=UI) TSubclassOf<UUserWidget> StageIntroWidgetClass;
UPROPERTY(EditDefaultsOnly, Category=UI) TSubclassOf<UUserWidget> StageClearWidgetClass;

/** 레벨에 배치된 인트로용 CineCameraActor. 태그 "IntroCamera"로 auto-find */
UPROPERTY(EditInstanceOnly, Category=Intro) TObjectPtr<ACineCameraActor> IntroCameraActor;

/** 카메라 전환에 사용할 EaseExponent (VTBlend_EaseInOut) */
UPROPERTY(EditDefaultsOnly, Category=Intro) float CameraBlendExponent = 2.f;

// 런타임
int32 AliveEnemyCount = 0;
TObjectPtr<UUserWidget> StageIntroWidgetInstance;
```

**주요 메서드**
```cpp
// 적 사망 시 EnemyDeathHandler의 OnDeathFinished에서 호출
void OnEnemyDied();

// 인트로 시퀀스 시작 (BeginPlay에서 호출)
void StartIntroSequence();

// 5초 후 카메라 블렌드 + 위젯 애니메이션 시작
void BeginCameraTransition();

// 2초 블렌드 완료 후 플레이어 조작 활성화
void OnIntroComplete();

// 모든 적 사망 시 호출
void OnStageClear();

UFUNCTION(BlueprintCallable) void RestartStage();
UFUNCTION(BlueprintCallable) void ReturnToTitle();
```

**`BeginPlay()` 흐름**
1. 레벨 내 모든 `AEnemyBase` 를 찾아 `AliveEnemyCount` 집계  
2. 각 Enemy의 `EnemyDeathHandler::OnDeathFinished` 에 `OnEnemyDied` 바인드  
3. 플레이어 컨트롤러의 `DisableInput` 호출 (InputMode: Game+UI, 마우스 숨김)  
4. `IntroCameraActor`가 null이면 태그 `"IntroCamera"`로 월드에서 검색  
5. `SetViewTarget(IntroCameraActor)` → 인트로 카메라로 전환  
6. `StartIntroSequence()` 호출  
7. `GameInstance->HideLoadingScreen()` 호출 (WBP_Loading FadeOut + WBP_StageIntro FadeIn은 위젯 자체 처리)

**`StartIntroSequence()` 흐름**
1. `WBP_StageIntro` 생성 + AddToViewport  
2. 위젯에 `BestDeathCount` 전달 후 `PlayIntroAnim()` 호출 (위젯 내부에서 최고 기록 표시 FadeIn)  
3. 5초 타이머 → `BeginCameraTransition()` 호출

**`BeginCameraTransition()` 흐름**
1. `PlayerController->SetViewTargetWithBlend(PlayerPawn, 2.f, VTBlend_EaseInOut, CameraBlendExponent)` 호출  
2. 동시에 `StageIntroWidget->PlayOutroAnim()` 호출 (기록 텍스트 FadeOut + 블랙바 사라짐)  
3. 2초 타이머 → `OnIntroComplete()` 호출

**`OnIntroComplete()` 흐름**
1. PlayerController `EnableInput` 호출 (InputMode: Game Only)  
2. `WBP_StageIntro→RemoveFromParent()`

**`OnStageClear()` 흐름**
1. `UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f)`  
2. PlayerController `DisableInput` 호출 (플레이어 조작 전면 차단)  
3. `WBP_StageClear` 생성 + AddToViewport  
4. 위젯에 `DeathCount`(PlayerDeathHandler) 전달 후 FadeIn 애니메이션  
5. InputMode: UI Only 전환 (마우스 커서 표시, 버튼 클릭은 UI 이벤트로만 처리)

**`RestartStage()` / `ReturnToTitle()` 공통 흐름**
1. `UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f)` 즉시 복구  
2. `GameInstance->ShowLoadingScreen()` 호출  
3. 화면 즉시 암전 (WBP_StageClear 위에 검정 오버레이 또는 LoadingScreen이 불투명하게 덮음)  
4. `OpenLevel("Lvl_MainStage")` 또는 `OpenLevel("Lvl_Title")`

---

## 4. 신규 Widget Blueprint

모두 C++ 베이스 없이 **순수 Blueprint `UserWidget`** 으로 생성.

### `WBP_Title`
| 요소 | 설명 |
|---|---|
| 배경 이미지/패널 | 타이틀 화면 레이아웃 |
| Text "WP Sifu" | 타이틀 텍스트 |
| Button `Btn_Start` | 클릭 → `TitleGameMode::RequestStartGame()` 호출 |
| Button `Btn_Quit` | 클릭 → `TitleGameMode::RequestQuitGame()` 호출 |
| **Animation `Anim_FadeOut`** | 위젯 전체 0.5초 FadeOut |

버튼 클릭 이벤트에서 `Get Game Mode → Cast to TitleGameMode → 함수 호출`.

---

### `WBP_Loading`
| 요소 | 설명 |
|---|---|
| SizeBox 전체화면 | 검정 불투명 배경 (ZOrder=10) |
| Text 혹은 이미지 | "로드 중..." |
| **Animation `Anim_FadeIn`** | 위젯 FadeIn (0.3초) |
| **Animation `Anim_FadeOut`** | 위젯 FadeOut (0.3초) → 완료 시 RemoveFromParent |

`GameInstance`가 생성·관리. `HideLoadingScreen()`에서 `PlayAnimation(Anim_FadeOut)` 호출.

---

### `WBP_StageIntro`
| 요소 | 설명 |
|---|---|
| Panel 전체화면 | 투명 배경 (HitTest Invisible) |
| Text `Txt_BestRecord` | "최고 기록: {BestDeathCount}회" (없으면 "기록 없음") |
| Image `BlackBar_Top` | 화면 상단 검정 바 (Height ≈ 80px) |
| Image `BlackBar_Bottom` | 화면 하단 검정 바 |
| **Animation `Anim_RecordFadeIn`** | 인트로카메라 시작 시 재생 |
| **Animation `Anim_OutroSlide`** | 기록 텍스트 FadeOut + 바 슬라이드 아웃 (동시, 2초) |

`MainStageGameMode`에서 생성 후:
- `BestDeathCount` 바인딩 (BP에서 Text의 Binding 또는 SetBestRecord 함수 사용)
- `PlayAnimation(Anim_RecordFadeIn)` 호출
- 전환 시작 시 `PlayAnimation(Anim_OutroSlide)` 호출

---

### `WBP_StageClear`
| 요소 | 설명 |
|---|---|
| SizeBox 전체화면 | 반투명 검정 배경 (Opacity 0.7) |
| Text `Txt_DeathCount` | "이번 시도 사망 횟수: {DeathCount}회" |
| Button `Btn_Restart` | "다시 시작" → `MainStageGameMode::RestartStage()` |
| Button `Btn_Title` | "타이틀로" → `MainStageGameMode::ReturnToTitle()` |
| **Animation `Anim_FadeIn`** | 0.5초 FadeIn |

---

## 5. 레벨 및 에디터 설정 (수동 작업)

### 5-1. GameInstance 등록

`Config/DefaultEngine.ini`에서:
```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/WP_Sifu.WP_GameInstance
```
또는 Project Settings → Maps & Modes → Game Instance Class: `BP_WP_GameInstance`

### 5-2. `Lvl_Title` 설정
- World Settings → GameMode Override: `BP_TitleGameMode`
- `BP_TitleGameMode` Details에서 `TitleWidgetClass = WBP_Title` 설정

### 5-3. `Lvl_MainStage` 설정
- World Settings → GameMode Override: `BP_MainStageGameMode`
- `BP_MainStageGameMode` Details에서:
  - `StageIntroWidgetClass = WBP_StageIntro`
  - `StageClearWidgetClass = WBP_StageClear`
  - `IntroCameraActor` = 레벨에 배치할 CineCameraActor 참조 (또는 태그 `"IntroCamera"`로 자동탐색)
- 레벨에 `CineCameraActor` 배치 후 **Tags** 배열에 `"IntroCamera"` 추가 (auto-find 시)

### 5-4. `WBP_Loading`
- `BP_WP_GameInstance` Details에서 `LoadingWidgetClass = WBP_Loading` 설정

---

## 6. 전체 흐름 다이어그램

```
[Lvl_Title 시작]
  └─ TitleGameMode::BeginPlay()
       └─ WBP_Title 생성 + AddToViewport

[시작 버튼 클릭]
  └─ TitleGameMode::RequestStartGame()
       ├─ WBP_Title Anim_FadeOut (0.5초)
       ├─ GameInstance::ShowLoadingScreen()
       │    └─ WBP_Loading 생성 + AddToViewport + FadeIn
       └─ OpenLevel("Lvl_MainStage")

[Lvl_MainStage 로드 완료]
  └─ MainStageGameMode::BeginPlay()
       ├─ GameInstance::OnWorldChanged() → WBP_Loading 재부착
       ├─ Enemy 탐색 + OnDeathFinished 바인드
       ├─ PlayerController DisableInput
       ├─ ViewTarget → IntroCameraActor
       └─ StartIntroSequence()
            ├─ WBP_StageIntro 생성 (+ BestDeathCount 세팅)
            ├─ GameInstance::HideLoadingScreen() (FadeOut)
            ├─ WBP_StageIntro::Anim_RecordFadeIn 재생
            └─ [5초 타이머] → BeginCameraTransition()
                  ├─ SetViewTargetWithBlend(Player, 2s, EaseInOut)
                  ├─ WBP_StageIntro::Anim_OutroSlide 재생
                  └─ [2초 타이머] → OnIntroComplete()
                        ├─ PlayerController EnableInput
                        └─ WBP_StageIntro 제거

[전투 중 Enemy 사망]
  └─ MainStageGameMode::OnEnemyDied()
       └─ AliveEnemyCount == 0 → OnStageClear()
            ├─ SetGlobalTimeDilation(0.5)
            ├─ WBP_StageClear 생성 + AddToViewport + FadeIn
            └─ InputMode: UI

[다시 시작 or 타이틀 버튼]
  └─ MainStageGameMode::RestartStage() / ReturnToTitle()
       ├─ SetGlobalTimeDilation(1.0)
       ├─ GameInstance::UpdateBestRecord(DeathCount) [RestartStage 경우에도 기록]
       ├─ GameInstance::ShowLoadingScreen() (즉시 불투명)
       └─ OpenLevel("Lvl_MainStage" or "Lvl_Title")
```

---

## 7. 구현 순서

| 단계 | 작업 |
|---|---|
| 1 | `UWP_SaveGame`, `UWP_GameInstance` C++ 작성 + 빌드 |
| 2 | `ATitleGameMode`, `AMainStageGameMode` C++ 작성 + 빌드 |
| 3 | `BP_WP_GameInstance`, `BP_TitleGameMode`, `BP_MainStageGameMode` BP 생성 + 설정 |
| 4 | `WBP_Loading` 위젯 생성 + 애니메이션 구성 |
| 5 | `WBP_Title` 위젯 생성 + 버튼 이벤트 연결 |
| 6 | `WBP_StageIntro` 위젯 생성 + 블랙바 + 애니메이션 구성 |
| 7 | `WBP_StageClear` 위젯 생성 + 버튼 이벤트 연결 |
| 8 | `Lvl_Title` / `Lvl_MainStage` World Settings 설정 |
| 9 | `Lvl_MainStage` 에 CineCameraActor 배치 |
| 10 | 전체 흐름 플레이 테스트 |

---

## 8. 확정된 사항 (추가)

- `SetGlobalTimeDilation`은 월드 액터 움직임에만 적용되며 UI 인터랙션에는 영향 없음. `WBP_StageClear`의 버튼 클릭은 정상 동작.
