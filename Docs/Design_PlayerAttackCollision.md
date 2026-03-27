# 플레이어 공격 충돌 검출 시스템 설계

## 개요

공격 애니메이션 재생 중 AnimNotifyState로 공격 부위의 충돌 검출을 활성화하고,
매 틱 Sweep을 통해 피격 대상에게 데미지를 적용하는 시스템.

무기(`AWeaponBase`)와 맨몸(손·발) 충돌을 구분하여 설계한다.

---

## 아키텍처 다이어그램

```
[AnimNotifyState_AttackCollision]
  AttackTag: Attack.Source.Hand.R   (맨손 몽타주)
  AttackTag: Attack.Source.Default     (무기 몽타주)
       │
       ▼ FindAttackCollision(AttackTag)
[UAttackCollisionManagerComponent]
  ┌──────────────────────────────────────────────┐
  │  TMap<FGameplayTag, UAttackCollisionComponent*>  │
  │                                              │
  │  Attack.Source.Hand.L  → HandCollisionLeft    │ ← 항상 등록 (맨몸)
  │  Attack.Source.Hand.R  → HandCollisionRight   │ ← 항상 등록 (맨몸)
  │  Attack.Source.Foot.L  → FootCollisionLeft    │ ← 항상 등록 (맨몸)
  │  Attack.Source.Foot.R  → FootCollisionRight   │ ← 항상 등록 (맨몸)
  │  Attack.Source.Default → WeaponCollision      │ ← 무기 장착 시 추가, 해제 시 제거
  └──────────────────────────────────────────────┘
       │ ActivateTrace() / DeactivateTrace()
       ▼
[UAttackCollisionComponent]
  PreviousLocation → CurrentLocation 방향으로 ComponentSweepMulti
  (내부 UStaticMeshComponent의 Simple Collision 도형 사용)
       │
       │ HitActor 발견 → OnAttackHit.Broadcast
       ▼
[UAttackCollisionManagerComponent::HandleHit]
  AttackComp->MakeCurrentAttackPayload()
  + ImpactLocation 세팅
       ▼
[CombatComponentBase::SendAttack]
  ├─ IAttackable → ReceiveAttack()
  └─ else        → UGameplayStatics::ApplyDamage()
```

---

## AttackTag 설계

### 맨몸 태그 (항상 유효)

| 태그 | 의미 | 소켓 |
| --- | --- | --- |
| `Attack.Source.Hand.L` | 왼손 | `HandL` |
| `Attack.Source.Hand.R` | 오른손 | `HandR` |
| `Attack.Source.Foot.L` | 왼발 | `FootL` |
| `Attack.Source.Foot.R` | 오른발 | `FootR` |

### 무기 태그 (무기 장착 시에만 유효)

무기 BP 서브클래스에서 `AttackCollision->AttackTag`를 직접 설정한다.

| 예시 태그 | 용도 |
| --- | --- |
| `Attack.Source.Default` | 양손무기 등 단일 파츠 무기 |
| `Attack.Source.Blade` | 검류 별도 구분이 필요한 경우 (선택) |

맨몸 태그는 무기 장착 여부와 무관하게 **항상 맵에 등록**되어 있다.
무기 장착 시 해당 무기의 AttackTag가 **추가**될 뿐, 맨몸 태그를 대체하지 않는다.

> AnimNotifyState는 몽타주에 배치할 때 보이는 애니메이션을 보고 직접 AttackTag를 지정하면 된다.
> 맨손 공격 몽타주에는 `Attack.Source.Hand.R` 등을, 무기 공격 몽타주에는 `Attack.Source.Default` 등을 사용한다.

---

## 신규 클래스

### 1. `UAttackCollisionComponent` (USceneComponent 파생)

충돌 검출 로직을 캡슐화하는 재사용 가능한 컴포넌트.

- **캐릭터에 직접 부착** 시 맨손/발 충돌 역할
- **AWeaponBase의 서브컴포넌트** 역할

**콜리전 도형:** `CollisionMeshAsset` (UStaticMesh)을 설정하면 해당 메시의 Simple Collision을 내부 UStaticMeshComponent에 적용한다.  Sweep에는 `UWorld::ComponentSweepMulti`를 사용하므로 메시에 Simple Collision이 설정되어 있어야 한다.

```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class UAttackCollisionComponent : public USceneComponent
{
    // --- 설정 ---

    UPROPERTY(EditAnywhere, Category=Collision)
    FGameplayTag AttackTag;                    // 식별 태그 (Attack.Source.Hand.R 등)

    // Simple Collision이 설정된 StaticMesh. 이 메시의 콜리전 도형으로 Sweep한다.
    UPROPERTY(EditAnywhere, Category=Collision)
    TObjectPtr<UStaticMesh> CollisionMeshAsset;

    UPROPERTY(EditAnywhere, Category=Collision)
    TEnumAsByte<ECollisionChannel> TraceChannel;  // WeaponTrace 커스텀 채널

    // --- 공개 메서드 ---

    void ActivateTrace();             // Sweep 시작, HitSet 초기화, PrevLocation 세팅
    void DeactivateTrace();           // Sweep 중단
    bool IsTraceActive() const;

    FGameplayTag GetAttackTag() const { return AttackTag; }

    // --- 이벤트 (AttackCollisionManagerComponent가 구독) ---

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttackHit, AActor*, const FHitResult&)
    FOnAttackHit OnAttackHit;

    // --- 내부 ---
protected:
    virtual void OnRegister() override;   // CollisionMeshAsset → CollisionMeshComp 적용

    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> CollisionMeshComp;  // 내부 서브컴포넌트, Hidden in Game

    bool bActive = false;
    FVector PreviousLocation;
    TSet<TWeakObjectPtr<AActor>> HitActors;

    void PerformSweep();
    // GetWorld()->ComponentSweepMulti(Hits, CollisionMeshComp,
    //     PreviousLocation, CurrentLocation, CollisionMeshComp->GetComponentQuat(), Params)
    // Owner(캐릭터) 제외, HitActors 중복 제외
    // 히트 시 OnAttackHit.Broadcast(HitActor, HitResult)
};
```

**에디터 시각화:**

- `CollisionMeshComp`는 `bHiddenInGame = true`이지만, 에디터 뷰포트에서는 표시됨
- `CollisionMeshAsset`에 Simple Collision이 설정된 메시를 할당하면 뷰포트에서 콜리전 도형 확인 가능 (`Show → Collision` 옵션)

---

### 2. `AWeaponBase` (AActor 파생)

필드에서 줍거나 버릴 수 있는 장착형 무기.

```cpp
UCLASS(BlueprintType, Blueprintable)
class AWeaponBase : public AActor
{
    // --- 컴포넌트 ---

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon)
    TObjectPtr<UStaticMeshComponent> WeaponMesh;   // 무기 외형 (항상 표시)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon)
    TObjectPtr<UAttackCollisionComponent> AttackCollision;  // 충돌 검출

    // --- 설정 ---

    UPROPERTY(EditDefaultsOnly, Category=Weapon)
    EWeaponType WeaponType;                         // OneHanded / TwoHanded

    UPROPERTY(EditDefaultsOnly, Category=Weapon)
    FName AttachSocketName;                         // 장착 소켓 (HandR 등)

    // 이 무기를 장착했을 때 UPlayerAttackComponent가 사용할 공격 정의 테이블.
    // FPlayerAttackDefinitionRow 구조 사용 — 각 상태(State)마다 개별 AnimMontage 지정.
    UPROPERTY(EditDefaultsOnly, Category=Weapon)
    TObjectPtr<UDataTable> AttackDefinitionTable;

    // --- 장착 / 해제 ---

    void OnEquipped(ACharacter* OwnerChar);
    void OnUnequipped();
};
```

> `AttackCollision->AttackTag`는 BP 서브클래스에서 설정한다. (예: 양손무기 BP → `Attack.Source.Default`)
>
> `AttackCollision->CollisionMeshAsset`에는 WeaponMesh와 동일한 에셋을 사용하거나,
> Simple Collision만 갖는 전용 저폴리 메시를 별도로 만들 수 있다.

**에디터 시각화:**

- `WeaponMesh`에 메시를 할당하면 무기 외형이 바로 편집 창에서 표시됨
- `AttackCollision->CollisionMeshComp`는 Hidden이지만 에디터 뷰포트 `Show → Collision`에서 도형 확인 가능
- 캐릭터 BP에서 무기 미리보기: `BP_MainCharacter` → Components 패널 → **Add Child Actor Component** → 무기 BP 클래스 할당 → 소켓 연결 (아래 참조)

---

### 3. `UAttackCollisionManagerComponent` (UActorComponent 파생)

캐릭터의 모든 공격 소스(맨몸 + 무기)의 `UAttackCollisionComponent`와 AttackTag 라우팅을 관리한다.
맨몸 충돌은 `RegisterPersistentCollision`으로 등록하며, 무기 충돌은 장착/해제 시 자동으로 추가·제거된다.

```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class UAttackCollisionManagerComponent : public UActorComponent
{
    // --- 외부 참조 (BeginPlay에서 GetOwner()->FindComponentByClass) ---
    TObjectPtr<UCombatComponentBase> CombatComp;
    TObjectPtr<UPlayerAttackComponent> AttackComp;

    // --- 상태 ---

    UPROPERTY()
    TObjectPtr<AWeaponBase> EquippedWeapon = nullptr;  // null = 맨손 상태

    // 무기 장착 전 AttackDefinitionTable 백업 (해제 시 복원용)
    UPROPERTY()
    TObjectPtr<UDataTable> DefaultAttackTable;

    // AttackTag → 현재 활성 UAttackCollisionComponent
    TMap<FGameplayTag, UAttackCollisionComponent*> ActiveCollisionMap;

    // --- 공개 메서드 ---

    // AnimNotifyState에서 호출
    UAttackCollisionComponent* FindAttackCollision(const FGameplayTag& AttackTag) const;

    // 충돌 컴포넌트를 AttackTag 기준으로 등록한다.
    // 맨몸 충돌(손·발)은 AMainCharacter::BeginPlay에서 컴포넌트별로 한 번씩 호출.
    // Comp->GetAttackTag()가 유효해야 한다.
    void RegisterPersistentCollision(UAttackCollisionComponent* Comp);

    UFUNCTION(BlueprintCallable, Category=Weapon)
    void EquipWeapon(AWeaponBase* Weapon);

    UFUNCTION(BlueprintCallable, Category=Weapon)
    void UnequipWeapon();

    UFUNCTION(BlueprintPure, Category=Weapon)
    bool HasWeaponEquipped() const { return EquippedWeapon != nullptr; }

    // --- 내부 ---
private:
    // UAttackCollisionComponent::OnAttackHit에 구독하여 실제 데미지 처리
    void HandleHit(AActor* HitActor, const FHitResult& HitResult);
    // 1. AttackComp->MakeCurrentAttackPayload()
    // 2. Payload.ImpactLocation = HitResult.ImpactPoint
    // 3. CombatComp->SendAttack(HitActor, Payload)
};
```

**장착 / 해제 흐름:**

```
EquipWeapon(WeaponActor)
├── DefaultAttackTable = AttackComp->GetDefaultAttackTable()  // 현재 테이블 백업
├── AttackComp->SetAttackTable(WeaponActor->AttackDefinitionTable)  // 무기 전용 테이블로 교체
├── WeaponActor->OnEquipped(Character)  // AttachToComponent(HandR 등)
├── WeaponActor->AttackCollision->OnAttackHit 구독
├── ActiveCollisionMap.Add(WeaponActor->AttackCollision->GetAttackTag(),
│                          WeaponActor->AttackCollision)  // 예: Attack.Source.Default
└── EquippedWeapon = WeaponActor

UnequipWeapon()
├── AttackComp->SetAttackTable(DefaultAttackTable)  // 맨손 테이블 복원
├── DefaultAttackTable = nullptr
├── ActiveCollisionMap.Remove(EquippedWeapon->AttackCollision->GetAttackTag())
├── OnAttackHit 구독 해제
├── EquippedWeapon->OnUnequipped()  // 분리 후 드롭 또는 파괴
└── EquippedWeapon = nullptr
```

> 맨몸 태그(`Attack.Source.Hand/Foot.*`)는 `RegisterPersistentCollision`에서 등록된 이후 변경되지 않는다.
> 무기의 AttackTag(`Attack.Source.Default` 등)는 완전히 별개 키로 추가/제거된다.

---

### 4. `AnimNotifyState_AttackCollision` (UAnimNotifyState 파생)

몽타주에 배치하여 공격 구간 동안 충돌 검출을 활성화/비활성화.

```cpp
UCLASS(meta=(DisplayName="Attack Collision"))
class UAnimNotifyState_AttackCollision : public UAnimNotifyState
{
    // 에디터에서 몽타주를 보면서 직접 지정
    UPROPERTY(EditAnywhere, Category=Attack)
    FGameplayTag AttackTag;   // 예: Attack.Source.Hand.R, Attack.Source.Default 등

    virtual void NotifyBegin(...) override;
    // auto* Mgr = Owner->FindComponentByClass<UAttackCollisionManagerComponent>();
    // if (auto* Collision = Mgr->FindAttackCollision(AttackTag)) Collision->ActivateTrace();

    virtual void NotifyEnd(...) override;
    // if (auto* Collision = ...) Collision->DeactivateTrace();

    virtual FString GetNotifyName_Implementation() const override;
    // → "AttackCollision: {AttackTag의 짧은 이름}"
};
```

---

## 기존 클래스 수정

### `AMainCharacter` 수정

```cpp
// 추가 컴포넌트
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
TObjectPtr<UAttackCollisionManagerComponent> AttackCollisionManagerComp;

// 맨손 충돌 (HandL, HandR 소켓에 부착)
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
TObjectPtr<UAttackCollisionComponent> HandCollisionLeft;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
TObjectPtr<UAttackCollisionComponent> HandCollisionRight;

// 맨발 충돌 (FootL, FootR 소켓에 부착)
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
TObjectPtr<UAttackCollisionComponent> FootCollisionLeft;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
TObjectPtr<UAttackCollisionComponent> FootCollisionRight;
```

**생성자에서:**

- 각 CollisionComponent를 CreateDefaultSubobject로 생성
- `SetupAttachment(GetMesh(), "HandL/HandR/FootL/FootR")` 로 소켓에 부착
- `AttackTag` 각각 설정 (`Attack.Source.Hand.L` 등)
- `CollisionMeshAsset` 은 에디터(BP_MainCharacter)에서 할당

**BeginPlay에서:**

```cpp
AttackCollisionManagerComp->RegisterPersistentCollision(HandCollisionLeft);
AttackCollisionManagerComp->RegisterPersistentCollision(HandCollisionRight);
AttackCollisionManagerComp->RegisterPersistentCollision(FootCollisionLeft);
AttackCollisionManagerComp->RegisterPersistentCollision(FootCollisionRight);
```

---

### `UPlayerAttackComponent` 수정

각 공격 액션마다 별도의 AnimMontage를 사용하므로, `AttackDefinitionTable`에서 상태 태그별로 몽타주를 조회한다.
무기 장착 시 테이블을 교체할 수 있도록 getter/setter를 추가한다.

```cpp
// 추가 메서드
UFUNCTION(BlueprintCallable, Category=Attack)
void SetAttackTable(UDataTable* NewTable);  // EquipWeapon/UnequipWeapon에서 호출

UFUNCTION(BlueprintPure, Category=Attack)
UDataTable* GetDefaultAttackTable() const { return AttackDefinitionTable; }
```

> `AttackDefinitionTable`의 접근 지정자는 현행 유지 (`EditDefaultsOnly`). setter를 통해서만 변경한다.
>
> `SetAttackTable` 호출 시 `MontageCache`를 새 테이블 기준으로 재구성한다.

---

## 커스텀 Trace Channel 설정 방법

### 1. Project Settings에서 채널 추가

`Edit → Project Settings → Engine → Collision → Trace Channels → New Trace Channel`

| 항목 | 값 |
| --- | --- |
| Name | `WeaponTrace` |
| Default Response | `Ignore` |

추가 후 `Config/DefaultEngine.ini`에 아래가 자동 생성됨:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="WeaponTrace")
```

### 2. Object Type별 Response 설정

`Project Settings → Collision → Object Channels / Preset`에서 각 오브젝트 타입의 WeaponTrace Response를 변경한다.

| Object / Profile | WeaponTrace Response |
| --- | --- |
| `Pawn` | `Block` |
| `PhysicsBody` | `Block` |
| `WorldStatic` | `Ignore` |
| `WorldDynamic` | `Block` (파괴 가능 오브젝트 포함 시) |
| 플레이어 자신 | `Ignore` (캐릭터 Capsule/Mesh Profile에서 직접 설정) |

> 플레이어를 제외하는 더 간단한 방법: `ComponentSweepMulti` 의 `FComponentQueryParams`에
> `AddIgnoredActor(GetOwner()->GetAttachParentActor())` 를 전달하면
> 캐릭터 자신에 붙은 모든 컴포넌트가 자동으로 제외된다.

### 3. C++ 코드에서 채널 참조

채널 번호는 프로젝트에서 추가한 순서에 따라 달라지므로, 이름으로 조회하는 방식을 권장한다.

```cpp
// UAttackCollisionComponent::OnRegister 또는 BeginPlay에서
const FName ChannelName = TEXT("WeaponTrace");
ECollisionChannel Ch = UCollisionProfile::Get()
    ->ConvertToCollisionChannel(false, ChannelName);
TraceChannel = Ch;
```

또는 `DefaultEngine.ini`에서 번호를 확인한 뒤 상수로 사용:

```cpp
TraceChannel = ECC_GameTraceChannel1;  // WeaponTrace가 첫 번째로 추가된 경우
```

---

## 에디터 수동 작업

### 공격 몽타주 설정 (맨손)

공격 액션마다 별도의 AnimMontage를 사용한다. `AttackDefinitionTable` (DataTable)에서
각 상태 태그(State)에 대응하는 몽타주를 지정하고, 해당 몽타주에 `Attack Collision` AnimNotifyState를 배치한다.

| 공격 액션 예시 | AttackTag | AnimNotifyState 배치 구간 |
| --- | --- | --- |
| L1 (오른손 직권) | `Attack.Source.Hand.R` | 손이 뻗어나가는 시작 ~ 되돌아오기 전 |
| L2 (왼손 직권) | `Attack.Source.Hand.L` | 동일 |
| L3 이후 | 애니메이션 보고 결정 | |

### 무기별 몽타주 설정

무기 장착 시 사용하는 `AWeaponBase::AttackDefinitionTable`에서 각 상태 태그에 대응하는 몽타주를 지정하고,
해당 몽타주에 무기 자체의 AttackTag를 가진 AnimNotifyState를 배치한다.

| 무기 종류 | AttackTag | 비고 |
| --- | --- | --- |
| 양손무기 | `Attack.Source.Default` | 무기 전체가 하나의 충돌 |
| 한손무기 | `Attack.Source.Default` (또는 별도 태그) | 무기 BP에서 결정 |

### 캐릭터 Blueprint에서 무기 미리보기

에디터에서 장착 모습을 확인하는 방법:

1. `BP_MainCharacter` 에디터 열기
2. **Components** 패널 → **Add** → `Child Actor Component` 추가
3. Details → `Child Actor Class` = 미리볼 무기 BP 설정
4. Details → `Transform` → Parent Socket = `HandR` 설정
5. 이 컴포넌트는 **미리보기 전용** — 런타임에는 `AttackCollisionManagerComp->EquipWeapon()`으로 관리

> Child Actor Component의 `bVisibleInEditor = true`, `bHiddenInGame = true` 설정을 권장.

### BP_MainCharacter에서 CollisionMeshAsset 할당

`BP_MainCharacter` → Components 패널에서 각 CollisionComponent 선택 → Details → `CollisionMeshAsset`에 사용할 StaticMesh 할당.

- `HandCollisionLeft/Right`: 손 크기의 구체/박스 Simple Collision을 가진 메시 (별도 제작 또는 엔진 기본 도형)
- `FootCollisionLeft/Right`: 발 크기의 박스 Simple Collision을 가진 메시

---

## 파일 변경 요약

| 작업 | 파일 | 비고 |
| --- | --- | --- |
| 신규 | `Public/Combat/AttackCollisionComponent.h` | 충돌 검출 컴포넌트 |
| 신규 | `Private/Combat/AttackCollisionComponent.cpp` | |
| 신규 | `Public/Weapon/WeaponBase.h` | 장착형 무기 Actor |
| 신규 | `Private/Weapon/WeaponBase.cpp` | |
| 신규 | `Public/Combat/AttackCollisionManagerComponent.h` | 공격 충돌 관리 컴포넌트 |
| 신규 | `Private/Combat/AttackCollisionManagerComponent.cpp` | |
| 신규 | `Public/Animation/AnimNotifyState_AttackCollision.h` | 공격 구간 NotifyState |
| 신규 | `Private/Animation/AnimNotifyState_AttackCollision.cpp` | |
| 수정 | `Public/Player/MainCharacter.h` | 컴포넌트 5종 추가 |
| 수정 | `Private/Player/MainCharacter.cpp` | 생성/부착/초기화 |
| 수정 | `Public/Player/PlayerAttackComponent.h` | `SetAttackTable` / `GetDefaultAttackTable` 추가 |
| 수정 | `Private/Player/PlayerAttackComponent.cpp` | |
| 수정 | `Source/WP_Sifu/WP_Sifu.Build.cs` | Include path 추가 (`Combat`, `Weapon`) |
| 에디터 | Project Settings → Collision | WeaponTrace 채널 추가 |
| 에디터 | `Config/DefaultEngine.ini` | GameplayTag 추가 (`Attack.Source.*`) |
| 에디터 | `AttackDefinitionTable` (각 공격 몽타주) | AnimNotifyState 배치 |
| 에디터 | `BP_MainCharacter` | CollisionMeshAsset 할당, 무기 미리보기 설정 |
