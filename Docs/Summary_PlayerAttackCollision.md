# 플레이어 공격 충돌 검출 시스템 — 구현 요약 및 에디터 작업 가이드

## 구현 요약

### 신규 파일

| 파일 | 역할 |
|---|---|
| `Public/Combat/AttackCollisionComponent.h` / `.cpp` | 공격 부위 충돌 검출 컴포넌트 (`USceneComponent` 파생). 내부 `UStaticMeshComponent`의 Simple Collision 도형으로 `ComponentSweepMulti` 수행. 매 틱 PreviousLocation→CurrentLocation Sweep하며, 히트 시 `OnAttackHit` 브로드캐스트. |
| `Public/Combat/AttackCollisionManagerComponent.h` / `.cpp` | 캐릭터의 모든 `UAttackCollisionComponent`를 `AttackTag` 기준 TMap으로 관리. 히트 이벤트를 `HandleHit`에서 수신하여 `MakeCurrentAttackPayload` → `SendAttack` 파이프라인으로 라우팅. |
| `Public/Weapon/WeaponBase.h` / `.cpp` | 장착형 무기 Actor. `UStaticMeshComponent`(외형) + `UAttackCollisionComponent`(충돌) + `AttackDefinitionTable`(무기 전용 공격 테이블) 포함. `EWeaponType` enum 정의. |
| `Public/Animation/AnimNotifyState_AttackCollision.h` / `.cpp` | 몽타주에 배치하여 공격 구간 동안 `AttackTag`에 대응하는 `UAttackCollisionComponent`의 `ActivateTrace()` / `DeactivateTrace()`를 호출. |

### 기존 파일 변경

| 파일 | 변경 내용 |
|---|---|
| `MainCharacter.h` / `.cpp` | `AttackCollisionManagerComp` + 4개 맨몸 충돌 컴포넌트(`HandCollisionLeft/Right`, `FootCollisionLeft/Right`) 추가. 생성자에서 소켓 부착 + AttackTag 설정, `BeginPlay`에서 Manager에 등록. |
| `PlayerAttackComponent.h` / `.cpp` | `SetAttackTable(UDataTable*)` / `GetDefaultAttackTable()` 추가. 무기 장착/해제 시 공격 테이블을 교체·복원할 수 있음. `BeginPlay`에서 기본 테이블을 `DefaultAttackDefinitionTable`에 백업. |
| `WP_Sifu.Build.cs` | 변경 없음 (include path 충돌 문제로 `Combat/`, `Weapon/` 경로를 추가하지 않고, 기존 `WP_Sifu/Public` 상대경로로 포함) |

### 설계 포인트

- **CollisionMeshComp의 Collision 설정**: `ECollisionEnabled::QueryOnly` 사용 (NoCollision이 아님). `ComponentSweepMulti`가 유효한 `BodyInstance`를 필요로 하기 때문. 모든 Response를 Ignore로 설정하되 `ECC_Pawn`만 Block으로 두어, 자동 충돌/오버랩은 발생하지 않으면서 수동 Sweep은 정상 동작.
- **에디터 시각화**: `CollisionMeshComp`는 항상 `bHiddenInGame = true`. 에디터에서의 표시 여부는 `bShowCollisionMeshInEditor` (기본값 `true`) 프로퍼티로 토글.
- **include 경로**: `CombatInteractionComponentBase.h` 파일명이 `Combat`으로 시작하여 UHT의 경로 해석과 충돌하므로, `WP_Sifu/Public/Combat`을 include path에 추가하지 않음. 대신 `#include "Combat/AttackCollisionComponent.h"` 형식의 상대경로를 사용.

---

## 에디터 수동 작업 (반드시 수행해야 함)

아래 작업은 코드에서 자동화할 수 없으며, UE5 에디터에서 직접 수행해야 합니다.

---

### 1. WeaponTrace 커스텀 Trace Channel 추가

**경로**: `Edit → Project Settings → Engine → Collision`

1. **Trace Channels** 섹션에서 `New Trace Channel` 클릭
2. 아래 값 설정:

   | 항목 | 값 |
   |---|---|
   | Name | `WeaponTrace` |
   | Default Response | `Ignore` |

3. **확인** 후 `Config/DefaultEngine.ini`에 자동으로 아래가 추가됨:
   ```ini
   +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="WeaponTrace")
   ```

> ⚠️ 코드에서 `TraceChannel`의 기본값이 `ECC_GameTraceChannel1`로 설정되어 있습니다. WeaponTrace가 **첫 번째**로 추가된 커스텀 채널이어야 합니다. 이미 다른 커스텀 채널이 있다면, `UAttackCollisionComponent`의 `TraceChannel` 기본값을 해당 채널 번호로 변경하거나, `BP_MainCharacter`에서 각 충돌 컴포넌트의 `TraceChannel` 프로퍼티를 수동으로 지정하세요.

---

### 2. Object Type별 WeaponTrace Response 설정

**경로**: `Project Settings → Engine → Collision → Preset`

각 Collision Preset에서 WeaponTrace 채널의 Response를 설정합니다:

| Object / Profile | WeaponTrace Response | 이유 |
|---|---|---|
| `Pawn` | **Block** | 적 캐릭터 히트 감지 |
| `PhysicsBody` | **Block** | 파괴 가능 오브젝트 (선택) |
| `WorldStatic` | Ignore | 벽·바닥 등은 무시 |
| `WorldDynamic` | Block 또는 Ignore | 파괴 가능 오브젝트 포함 여부에 따라 |
| 플레이어 자신 | Ignore | `ComponentSweepMulti`에서 `AddIgnoredActor(GetOwner())`로 이미 제외되지만, 프로필에서도 Ignore 권장 |

---

### 3. 스켈레탈 메시 소켓 확인/생성

**경로**: 콘텐츠 브라우저 → `ART_FROM_SIFU/Player/SK_M_MainChar_01` → 더블클릭 → Skeleton Tree

코드에서 다음 소켓 이름으로 부착하고 있으므로, 해당 소켓이 존재하는지 확인합니다:

| 소켓 이름 | 위치 | 부착 대상 |
|---|---|---|
| `HandL` | 왼손 본 자식 | `HandCollisionLeft` |
| `HandR` | 오른손 본 자식 | `HandCollisionRight` |
| `FootL` | 왼발 본 자식 | `FootCollisionLeft` |
| `FootR` | 오른발 본 자식 | `FootCollisionRight` |

**소켓이 없는 경우:**
1. Skeleton Editor 열기
2. 해당 본(예: `hand_l`) 우클릭 → `Add Socket`
3. 이름을 `HandL` 등으로 지정
4. Transform을 주먹/발 중심에 맞게 조정
5. 저장

> ⚠️ 소켓 이름이 스켈레톤의 실제 이름과 다를 경우, `MainCharacter.cpp` 생성자의 `SetupAttachment(GetMesh(), TEXT("HandL"))` 등의 소켓 이름을 실제 이름으로 수정하세요.

---

### 4. CollisionMesh(충돌 도형) StaticMesh 에셋 준비

각 맨몸 충돌 컴포넌트에 할당할 **Simple Collision이 포함된 StaticMesh**가 필요합니다.

**간단한 방법 (엔진 기본 도형 활용):**
1. 콘텐츠 브라우저에서 우클릭 → `Add/Import Content` → `Geometry` → `Cube` 또는 `Sphere`
2. 크기를 손/발에 맞게 스케일 조정 (예: 손 = ~15cm 구체, 발 = ~20×10×8cm 박스)
3. StaticMesh Editor → `Collision` 메뉴 → `Add Box Simplified Collision` 또는 `Add Sphere Simplified Collision`
4. 저장

**권장 에셋 구성:**

| 용도 | 추천 도형 | 대략적 크기 |
|---|---|---|
| 손 (`HandL/R`) | 구체 또는 박스 | 반지름 ~8~12cm |
| 발 (`FootL/R`) | 박스 | 20×10×8cm |

> 정밀도보다는 게임플레이 느낌이 중요합니다. 도형이 너무 크면 멀리서도 히트가 발생하고, 너무 작으면 히트를 놓칩니다. 테스트하면서 조정하세요.

---

### 5. BP_MainCharacter에서 CollisionMeshAsset 할당

**경로**: 콘텐츠 브라우저 → `Blueprints/Player/BP_MainCharacter` → 더블클릭

1. **Components** 패널에서 각 충돌 컴포넌트 선택:
   - `HandCollisionLeft`
   - `HandCollisionRight`
   - `FootCollisionLeft`
   - `FootCollisionRight`

2. **Details** 패널에서:
   - `Collision Mesh Asset` → 위에서 준비한 StaticMesh 할당
   - `Trace Channel` → `WeaponTrace` (ECC_GameTraceChannel1) 확인
   - `Attack Tag` → 자동 설정됨 (코드에서 생성자에 설정). 확인만 하면 됨
   - `Show Collision Mesh In Editor` → `true` (디버그 시 에디터에서 도형 확인 가능)

3. 뷰포트에서 `Show → Collision` 활성화하면 각 소켓 위치에 충돌 도형이 표시됨

---

### 6. 공격 몽타주에 AnimNotifyState 배치

각 공격 몽타주 에셋을 열어 **Attack Collision** NotifyState를 배치합니다.

**작업 순서:**

1. 몽타주 에셋 열기 (예: `AM_L1_Jab` 등)
2. 타임라인 하단의 **Notifies** 트랙에서 우클릭 → `Add Notify State` → `Attack Collision`
3. 공격 판정이 시작되는 프레임 ~ 끝나는 프레임까지 드래그하여 구간 설정
4. NotifyState 선택 → **Details**에서 `Attack Tag` 설정:

| 공격 액션 예시 | Attack Tag 값 | 설명 |
|---|---|---|
| L1 (오른손 직권) | `Attack.Source.Hand.R` | 오른손이 뻗어나가는 구간 |
| L2 (왼손 직권) | `Attack.Source.Hand.L` | 왼손이 뻗어나가는 구간 |
| L3 (오른 돌려차기) | `Attack.Source.Foot.R` | 오른발 회전 구간 |
| H1 (무거운 펀치) | `Attack.Source.Hand.R` | 애니메이션 보고 판단 |
| 무기 공격 | `Attack.Source.Default` | 무기 장착 시 사용할 몽타주 |

**구간 설정 팁:**
- **시작점**: 팔다리가 전방으로 뻗기 시작하는 프레임
- **끝점**: 팔다리가 되돌아오기 직전 (리커버리 시작 전)
- 너무 넓으면 히트 판정이 관대해지고, 너무 좁으면 빠른 움직임에서 히트를 놓침
- 기존 `Combo Transition Window` NotifyState와 겹쳐도 무방 (독립 동작)

---

### 7. (선택) 무기 Blueprint 작성

무기 시스템은 잠정 보류이지만, 나중에 필요할 때의 절차입니다:

1. 콘텐츠 브라우저 → 우클릭 → `Blueprint Class` → Parent: `WeaponBase`
2. **WeaponMesh** 에 무기 StaticMesh 할당
3. **AttackCollision** 선택:
   - `Attack Tag` = `Attack.Source.Default`
   - `Collision Mesh Asset` = 무기와 동일 메시 또는 전용 Simple Collision 메시
   - `Trace Channel` = WeaponTrace
4. **Details**:
   - `Weapon Type` = OneHanded / TwoHanded
   - `Attach Socket Name` = `HandR` (또는 적절한 소켓)
   - `Attack Definition Table` = 무기 전용 DataTable (기존과 동일한 `FPlayerAttackDefinitionRow` 구조)
5. 런타임에서 `AttackCollisionManagerComp->EquipWeapon(WeaponActor)`으로 장착

---

### 8. (선택) 에디터에서 무기 장착 미리보기

1. `BP_MainCharacter` 에디터 → Components → `Add` → `Child Actor Component`
2. Details → `Child Actor Class` = 미리볼 무기 BP
3. Details → `Parent Socket` = `HandR`
4. `Hidden In Game` = `true` (런타임에서는 `EquipWeapon`으로 관리)

> 이 컴포넌트는 에디터 미리보기 전용이며, 게임 실행 시에는 자동으로 숨겨집니다.

---

## 테스트 절차

1. **빌드 확인**: 프로젝트가 성공적으로 컴파일되는지 확인 ✅ (완료)
2. **소켓 확인**: PIE(Play In Editor) 실행 후 `ShowDebug` 또는 에디터 뷰포트에서 충돌 도형이 손·발 위치에 잘 따라다니는지 확인
3. **Sweep 동작 확인**: 에디터에서 `show collision` 활성화 후 공격 몽타주 재생. 콘솔에서 `p.VisualizeTraces 1` 로 Sweep 궤적 확인 가능
4. **히트 확인**: 적 캐릭터(IAttackable 구현체)를 배치하고 공격하여 `OnAttackReceived` 이벤트 및 데미지 적용 확인
5. **중복 히트 방지**: 한 번의 공격(ActivateTrace ~ DeactivateTrace 구간)에서 동일 액터에 대해 1회만 히트가 발생하는지 확인

