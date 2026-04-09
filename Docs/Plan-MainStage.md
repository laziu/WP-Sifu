# 메인 스테이지 구현 계획

타이틀 화면과 게임 스테이지 사이의 실행 흐름을 구현한다.

## 현재 상태

- `Lvl_Title`: 타이틀 화면
  - 게임 스테이지 시작, 게임 종료 버튼이 있다.
- `Lvl_MainStage`: 게임 스테이지
  - 맵에 player start, 약 10명의 enemy가 사전에 배치되어 있다.
  - player와 enemy는 각자 로직이 완성되어 있다.
  - player actor의 PlayerDeathHandlerComponent에는 플레이어가 총 몇번 죽었는지를 저장하는 DeathCount 변수가 있다.

## 전체 실행 흐름

1. 게임은 `Lvl_Title`에서 시작한다.
2. 게임 시작 버튼을 누르면 화면이 0.5초동안 fade out되고, '로드 중' 화면이 fade in 된다. background에선 `Lvl_MainStage`를 로드할 것이다.
3. 로드 중 화면이 3초 이상 표시되었고 게임 스테이지가 로드 완료됐을 때, '로드 중' 화면을 없에고 `Lvl_MainStage` 화면을 fade in 시킨다.
   1. `Lvl_MainStage`는 가장 처음에 최고 기록(죽음 횟수)를 보여준다. 이 때 Cinematic 연출 효과를 주기 위해 화면의 위/아래에 black bar가 그려져있다. 카메라는 player start의 약간 앞에서 풍경을 보여주고 있다.
   2. 약 5초 뒤에, 2초 동안 Player Actor에 붙은 Camera로 ease-in-and-out 방식으로 시점이 전환된다. 그와 동시에 최고 기록 표시가 fade out 되고, 위/아래 black bar는 바깥으로 사라지면서 카메라의 시야가 위아래로 넓어지는 효과를 준다.
   3. 카메라의 시야가 완전히 전환됐을 때부터 player를 조작할 수 있다.
4. `Lvl_MainStage` 내의 모든 Enemy가 죽었을 때, 게임의 속도를 0.5배속으로 재생하면서 도전 완료 창이 뜬다. 도전 완료 창은 반투명한 배경으로 전체화면을 가리며, 죽음 횟수와 '다시 시작', '타이틀로 돌아가기' 버튼을 보여준다.
5. '다시 시작'이나 '타이틀로 돌아가기' 버튼을 누르면 화면이 즉시 암전되고, '로드 중' 화면을 fade in 한다. 각자의 버튼 목적에 따라 처음과 같이 레벨이 로드된다.

## 구현 예상도

1. 화면 fade-in, fade-out, black bar 등은 user widget에서 animation을 설정해 구현한다.
2. 카메라 이동 및 메인 스테이지 시작과 끝 처리는 LevelScriptActor 또는 GameMode를 사용해 구현한다.
3. 각 버튼은 위에서 구현된 LevelScriptActor 또는 GameMode에 구현된 event를 호출하도록 구현한다.
