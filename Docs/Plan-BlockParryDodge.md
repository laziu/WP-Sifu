# 구현 계획: 블록 / 패링 / 블록-중-회피

**대상 파일**:

- `Source/WP_Sifu/Public/Player/PlayerCombatInteractionComponent.h`
- `Source/WP_Sifu/Private/Player/PlayerCombatInteractionComponent.cpp`
- `Source/WP_Sifu/Public/Combat/AnimNotifyState_ParryWindow.h` (신규)
- `Source/WP_Sifu/Private/Combat/AnimNotifyState_ParryWindow.cpp` (신규)
- `Source/WP_Sifu/Public/Combat/AnimNotifyState_DodgeActive.h` (신규)
- `Source/WP_Sifu/Private/Combat/AnimNotifyState_DodgeActive.cpp` (신규)
- `Source/WP_Sifu/Public/Combat/AnimNotify_DodgeCooldownEnd.h` (신규)
- `Source/WP_Sifu/Private/Combat/AnimNotify_DodgeCooldownEnd.cpp` (신규)

---

## 1. 현재 동작 요약

| 상황 | 현재 동작 |
|------|-----------|
| 블록 키 누름 | 즉시 `EDefenceState::Parrying` 진입, `ParryWindowDuration`(0.15s) 타이머 시작 |
| 타이머 만료 & 키 유지 | `EDefenceState::Blocking` 전환 |
| 타이머 만료 & 키 없음 | `EDefenceState::None` 전환 |
| 블록 키 뗌 | 타이머 클리어, `EDefenceState::None` |

**미구현**: 블록 중 방향키 회피 / AnimNotify 기반 판정 / 늦은 입력 허용.

---

## 2. 확정된 목표 동작

### 2-A. 패링 / 블록 타이밍 분리

- 블록 키 누름 → 블록 진입 몽타주 재생 → `AnimNotifyState_ParryWindow` 구간(~0.15s) 동안 패링 판정 활성.
- 패링 판정 구간 내 피격 → 패링 성공.
- 판정 구간 종료 후 키 유지 → `Blocking` / 키 없음 → `None`.

### 2-B. 블록 중 방향 회피

`EDefenceState::Blocking` 상태에서 `IA_Move` **Started** 이벤트 + 쿨다운 통과 시:

| IA_Move Y 값 | 동작 | 설명 | GameplayTag |
| --- | --- | --- | --- |
| Y > 0.5 (윗키) | 아랫회피 | 점프 — 낮은 공격 회피 | `Combat.State.Dodge.Up` |
| Y ≤ 0.5 (아래키·좌우키 포함) | 윗회피 | 숙이기 — 높은 공격 회피 | `Combat.State.Dodge.Down` |

> Y ≈ 0 (좌우키만 입력)인 경우 윗회피(숙이기)로 간주.

- **회피 판정 시간(0.4s)**: `AnimNotifyState_DodgeActive` 구간으로 정의.
  **블록 키를 뗀 경우에도 판정 구간은 항상 완주.**
  키 뗌 → `bBlockKeyHeld = false` 세팅만 수행. `NotifyEnd` 도달 후 `bBlockKeyHeld` 기준으로 `Blocking` / `None` 결정.
- **회피 쿨다운(0.2s)**: `AnimNotify_DodgeCooldownEnd` 발동 시 `bCanDodge = true`.
- **연속 횟수 제한 없음** (쿨다운만 적용).

### 2-C. 늦은 입력 허용 (Late Input Buffer)

타격보다 방어 입력이 조금 늦어도 유효하게 처리:

- 방어 상태 없이 피격 → `PendingHit` 기록, `LateInputWindow`(기본 0.1s) 타이머 시작.
- 타이머 만료 전 **블록 입력** → `Parrying` 진입 + `PendingHit` 즉시 패링 판정.
- 타이머 만료 전 **회피 입력** → 해당 방향 회피 실행 + `PendingHit` 즉시 회피 판정.
- 타이머 만료 전 입력 없음 → `PendingHit` 피해 정상 적용.

---

## 3. 필요한 변경 사항

### 3-1. AnimNotify C++ 클래스 신규 작성

세 클래스 모두 `MeshComp->GetOwner()`에서 `UPlayerCombatInteractionComponent`를 찾아 콜백 호출.

> **공통 헬퍼** (선택): 중복 탐색 로직을 `static UPlayerCombatInteractionComponent* GetCIC(USkeletalMeshComponent*)` 인라인 함수로 분리.

#### `UAnimNotifyState_ParryWindow`

```cpp
UCLASS()
class UAnimNotifyState_ParryWindow : public UAnimNotifyState
{
    GENERATED_BODY()
    virtual void NotifyBegin(USkeletalMeshComponent*, UAnimSequenceBase*,
        float TotalDuration, const FAnimNotifyEventReference&) override;
    virtual void NotifyEnd(USkeletalMeshComponent*, UAnimSequenceBase*,
        const FAnimNotifyEventReference&) override;
};
```

- `NotifyBegin` → `CIC->OnParryWindowBegin()` (선택적, 현재 `StartBlock`에서 이미 상태 세팅)
- `NotifyEnd` → `CIC->OnParryWindowEnd()` ← **패링 판정 종료, Blocking/None 전환**

#### `UAnimNotifyState_DodgeActive`

- `NotifyBegin` → `CIC->OnDodgeActiveBegin()` (선택적)
- `NotifyEnd` → `CIC->OnDodgeActiveEnd()` ← **회피 판정 종료, bBlockKeyHeld 기준 상태 복귀**

#### `UAnimNotify_DodgeCooldownEnd`

```cpp
UCLASS()
class UAnimNotify_DodgeCooldownEnd : public UAnimNotify
{
    GENERATED_BODY()
    virtual void Notify(USkeletalMeshComponent*, UAnimSequenceBase*,
        const FAnimNotifyEventReference&) override;
};
```

- `Notify` → `CIC->OnDodgeCooldownEnd()` ← **`bCanDodge = true`**

### 3-2. `PlayerCombatInteractionComponent.h` 변경

```cpp
// ── 제거 ─────────────────────────────────────────────────────
// FTimerHandle ParryWindowTimer;   → AnimNotifyState_ParryWindow 으로 대체
// FTimerHandle DodgeCooldownTimer; → AnimNotify_DodgeCooldownEnd 으로 대체
// float DodgeActiveWindow;         → 몽타주 NotifyState 길이로 정의

// ── 추가 ─────────────────────────────────────────────────────

// AnimNotify 콜백 (public — AnimNotify 클래스에서 호출)
void OnParryWindowBegin();
void OnParryWindowEnd();
void OnDodgeActiveBegin();
void OnDodgeActiveEnd();
void OnDodgeCooldownEnd();

// 회피 쿨다운 (몽타주 DodgeCooldownEnd Notify 위치 조정 시 참고용)
UPROPERTY(EditDefaultsOnly, Category=Dodge, meta=(ClampMin="0"))
float DodgeCooldown = 0.2f;

bool bCanDodge = true;

// 늦은 입력 버퍼
UPROPERTY(EditDefaultsOnly, Category=Input, meta=(ClampMin="0"))
float LateInputWindow = 0.1f;

struct FPendingHit
{
    // 구현 시 실제 프로젝트 피격 정보 구조체로 교체
    float TimeReceived;
};
TOptional<FPendingHit> PendingHit;
FTimerHandle PendingHitTimer;

float LastBlockInputTime = -1.f;

// private 함수
void TryBlockDodge(const FInputActionValue& Value);
void ExecuteBlockDodge(FGameplayTag DodgeStateTag);
void ApplyPendingHitDamage();
void ApplyLateParry(const FPendingHit& Hit);
void ApplyLateDodge(const FPendingHit& Hit, FGameplayTag DodgeTag);
```

### 3-3. `PlayerCombatInteractionComponent.cpp` 변경

**`SetupInputBindings`**: 기존 Triggered 이동 바인딩 유지, Started 이벤트 추가:

```cpp
EIC->BindAction(InputMove, ETriggerEvent::Started, this, &ThisClass::TryBlockDodge);
```

**`StartBlock()`**:

```cpp
void UPlayerCombatInteractionComponent::StartBlock()
{
    bBlockKeyHeld = true;
    LastBlockInputTime = GetWorld()->GetTimeSeconds();

    // 늦은 입력: 대기 중인 피격이 있으면 즉시 패링으로 처리
    if (PendingHit.IsSet())
    {
        GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
        ApplyLateParry(PendingHit.GetValue());
        PendingHit.Reset();
    }

    SetDefenceState(EDefenceState::Parrying);
    // 블록 진입 몽타주 재생 → NotifyState_ParryWindow 가 패링 판정 구간 구동
}
```

**`StopBlock()`**:

```cpp
void UPlayerCombatInteractionComponent::StopBlock()
{
    bBlockKeyHeld = false;

    if (DefenceState == EDefenceState::Dodging)
        return; // 판정 구간 완주 후 OnDodgeActiveEnd 에서 복귀

    SetDefenceState(EDefenceState::None);
    // 패링 중 키 뗌: 몽타주 중단 시 NotifyState 도 종료되어 OnParryWindowEnd 호출됨
}
```

**AnimNotify 콜백**:

```cpp
void UPlayerCombatInteractionComponent::OnParryWindowEnd()
{
    if (DefenceState != EDefenceState::Parrying) return;
    SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
}

void UPlayerCombatInteractionComponent::OnDodgeActiveEnd()
{
    if (DefenceState != EDefenceState::Dodging) return;
    SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
}

void UPlayerCombatInteractionComponent::OnDodgeCooldownEnd()
{
    bCanDodge = true;
}
```

**`TryBlockDodge()`** — 늦은 회피 포함:

```cpp
void UPlayerCombatInteractionComponent::TryBlockDodge(const FInputActionValue& Value)
{
    const bool bInBlocking = (DefenceState == EDefenceState::Blocking);
    const bool bLateDodge  = PendingHit.IsSet();

    if ((!bInBlocking && !bLateDodge) || !bCanDodge)
        return;

    const FGameplayTag DodgeTag = (Value.Get<FVector2D>().Y > 0.5f)
        ? GameplayTag::Combat_State_Dodge_Up
        : GameplayTag::Combat_State_Dodge_Down;

    if (bLateDodge)
    {
        GetWorld()->GetTimerManager().ClearTimer(PendingHitTimer);
        ApplyLateDodge(PendingHit.GetValue(), DodgeTag);
        PendingHit.Reset();
    }

    ExecuteBlockDodge(DodgeTag);
}
```

**`ExecuteBlockDodge()`** — OnCombatMontageEnded 구독 제거 (AnimNotify 로 대체):

```cpp
void UPlayerCombatInteractionComponent::ExecuteBlockDodge(FGameplayTag DodgeStateTag)
{
    bCanDodge = false;
    SetDefenceState(EDefenceState::Dodging);

    auto* AttackComp = GetOwner()->FindComponentByClass<UPlayerAttackComponent>();
    if (!AttackComp || !AttackComp->SetState(DodgeStateTag))
    {
        // 몽타주 재생 실패 → AnimNotify 미발동이므로 직접 복귀
        bCanDodge = true;
        SetDefenceState(bBlockKeyHeld ? EDefenceState::Blocking : EDefenceState::None);
    }
    // 성공 시: OnDodgeActiveEnd / OnDodgeCooldownEnd 가 AnimNotify 에서 호출됨
}
```

**`HandleIncomingHit()`** — 피격 라우팅:

```cpp
void UPlayerCombatInteractionComponent::HandleIncomingHit(const FHitInfo& Hit)
{
    if (DefenceState == EDefenceState::Parrying) { ApplyParry(Hit); return; }
    if (DefenceState == EDefenceState::Blocking) { ApplyBlock(Hit); return; }
    if (DefenceState == EDefenceState::Dodging)  { ApplyDodge(Hit); return; }

    // 방어 상태 없음 → 늦은 입력 대기
    PendingHit = FPendingHit{ GetWorld()->GetTimeSeconds() };
    GetWorld()->GetTimerManager().SetTimer(
        PendingHitTimer,
        this, &ThisClass::ApplyPendingHitDamage,
        LateInputWindow, false
    );
}

void UPlayerCombatInteractionComponent::ApplyPendingHitDamage()
{
    if (!PendingHit.IsSet()) return;
    // TODO: 실제 피해 적용
    PendingHit.Reset();
}
```

> `FHitInfo` 및 `ApplyParry / ApplyBlock / ApplyDodge` 는 기존 피격 처리 구조에 맞게 연결할 것.
> 적의 공격 처리가 현재 `TakeDamage` 또는 다른 경로로 구현되어 있다면, 그 경로에서 `HandleIncomingHit` 를 호출하도록 라우팅 필요.

### 3-4. `PlayerAttackComponent` 변경 사항

- `OnCombatMontageEnded` 델리게이트 **불필요** — AnimNotify 로 대체.
- `SetState()` 반환 타입을 `bool`(성공 여부)로 변경 (이미 아니라면):

```cpp
bool SetState(FGameplayTag StateTag); // 몽타주 재생 성공 시 true
```

---

## 4. 에디터 수동 작업 목록

### 4-1. 몽타주 제작 및 AnimNotify 배치 (전부 신규)

| 몽타주 | GameplayTag / 용도 | AnimNotify 배치 |
| --- | --- | --- |
| 블록 진입 몽타주 | 블록 키 누름 시 재생 | `NotifyState_ParryWindow` (0 ~ 0.15s) |
| 아랫회피 (점프) | `Combat.State.Dodge.Up` | `NotifyState_DodgeActive` (0 ~ 0.4s), `Notify_DodgeCooldownEnd` (0.6s) |
| 윗회피 (숙이기) | `Combat.State.Dodge.Down` | `NotifyState_DodgeActive` (0 ~ 0.4s), `Notify_DodgeCooldownEnd` (0.6s) |
| 패링 성공 반응 | 패링 성공 시 재생 | 별도 이벤트로 처리 (선택적) |

> `Notify_DodgeCooldownEnd` 위치(0.6s) = 판정 종료(0.4s) + 쿨다운(0.2s).
> 실제 쿨다운을 조정할 경우 이 위치만 옮기면 됨.

### 4-2. Animation Blueprint

- `EDefenceState::Blocking` 진입/이탈 시 가드 포즈 BlendIn/Out.
- 블록 진입 몽타주 슬롯 지정 (`UpperBody` 또는 `FullBody`).
- 회피 몽타주 슬롯과 이동 블렌딩 충돌 여부 확인.

### 4-3. AttackDefinitionTable

`Combat.State.Dodge.Up`, `Combat.State.Dodge.Down` 행에 위 몽타주 등록.

> 블록 진입 몽타주는 `AttackComponent::SetState` 경유 vs `CombatInteractionComponent`에서 직접 재생 중 방식 결정 필요.

### 4-4. Blueprint / Character

- `LateInputWindow`, `DodgeCooldown` 값 조정 (기본값: 0.1s / 0.2s).
- `FPendingHit` 내 실제 피격 정보 타입은 구현 시 프로젝트 데미지 구조체에 맞게 교체.
