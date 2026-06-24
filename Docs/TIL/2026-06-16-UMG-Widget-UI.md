# TIL 2026-06-16 — UMG 위젯 / 실시간 HUD / 메뉴 흐름 / UI 애니메이션 / 3D 위젯

> 프로젝트: `NBC_ch3_5` (언리얼 C++)
> 오늘 주제: 위젯 기반 실시간 데이터 반영, 게임 흐름에 맞춘 메뉴, UI 애니메이션, 위젯 컴포넌트(3D 위젯)

---

## 0. 오늘 한 일 한눈에

| 주제 | 핵심 클래스 | 핵심 함수 |
|------|------------|-----------|
| 실시간 HUD 데이터 반영 | `AMyGameState` | `UpdateHUD()` + 타이머 |
| 게임 흐름 메뉴 구현 | `AMyPlayerController` | `ShowGameHUD()`, `ShowMainMenu()`, `StartGame()` |
| UI 애니메이션 호출 | `AMyPlayerController` | `ProcessEvent`로 `PlayGameOverAnim` 호출 |
| 3D 위젯(위젯 컴포넌트) | `AMyPawn` | `OverheadWidget`, `UpdateOverheadHP()` |
| 데이터 영속성(레벨 간) | `UMyGameInstance` | `TotalScore`, `CurrentLevelIndex` |

---

## 1. 위젯으로 실시간 게임 데이터 반영하기

### 패턴: C++에서 UMG 위젯의 자식 요소를 이름으로 찾아 갱신

위젯 인스턴스에서 `GetWidgetFromName(TEXT("..."))`으로 디자이너에서 배치한 자식 위젯을 찾고, `Cast<UTextBlock>`으로 변환해 `SetText`를 호출한다. **이때 위젯 BP의 해당 요소를 "Is Variable" 체크해서 이름이 살아있어야** C++에서 찾을 수 있다.

```cpp
// MyGameState.cpp — UpdateHUD()
if (UUserWidget* HUDWidget = MyPlayerController->GetHUDWidget())
{
    if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
    {
        float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
        TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
    }
    // Score, Level 동일 패턴...
}
```

### 실시간성: 타이머로 주기적 갱신

`BeginPlay`에서 0.1초 간격 반복 타이머를 걸어 HUD를 폴링 방식으로 갱신한다.

```cpp
// MyGameState.cpp — BeginPlay()
GetWorldTimerManager().SetTimer(
    HUDUpdateTimerHandle, this, &AMyGameState::UpdateHUD,
    0.1f, /*bLoop=*/true);
```

**배운 점 / 메모**
- `GetWidgetFromName`은 매 호출마다 트리를 탐색하므로 비싸다. 0.1초 폴링 정도는 괜찮지만, 더 잦은 갱신이나 위젯 요소가 많아지면 **위젯 BP에 `BindWidget` 프로퍼티를 두거나, 이벤트 기반(델리게이트)으로 push** 하는 방식이 더 효율적이다.
- 남은 시간은 직접 카운트하지 않고 `GetTimerRemaining(LevelTimerHandle)`로 레벨 타이머에서 읽어온다 → 단일 소스(SSOT) 유지.

---

## 2. 게임 흐름에 맞춘 메뉴 / HUD 전환

`AMyPlayerController`가 UI 전환의 컨트롤 타워 역할. **위젯을 띄울 때 항상 이전 위젯을 먼저 제거**하고, 입력 모드와 마우스 커서를 함께 전환하는 게 핵심.

### HUD 표시 (게임 플레이 중)

```cpp
// ShowGameHUD()
HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
HUDWidgetInstance->AddToViewport();

bShowMouseCursor = false;
SetInputMode(FInputModeGameOnly());   // 게임 입력만
```

### 메인/게임오버 메뉴 표시

```cpp
// ShowMainMenu(bool bIsRestart)
MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
MainMenuWidgetInstance->AddToViewport();

bShowMouseCursor = true;
SetInputMode(FInputModeUIOnly());     // UI 입력만 (마우스로 버튼 클릭)
```

### 흐름 정리

```
[MenuLevel 진입] → BeginPlay에서 맵 이름 검사 → ShowMainMenu(false)  // "Start"
        │  StartGame() → GameInstance 초기화 → OpenLevel("BasicLevel")
        ▼
[게임 레벨] → GameState::BeginPlay → StartLevel → ShowGameHUD()      // 플레이 HUD
        │  레벨 클리어/타임업 → EndLevel → 다음 레벨 OpenLevel
        ▼
[모든 레벨 종료 / 사망] → OnGameOver → SetPause(true) + ShowMainMenu(true) // "Restart"
```

**배운 점 / 메모**
- 같은 `ShowMainMenu` 함수를 `bIsRestart` 플래그로 **시작 메뉴/게임오버 메뉴 겸용**으로 재사용 → 버튼 텍스트("Start"/"Restart")와 애니메이션, 총점 표시 분기.
- 맵 이름으로 메뉴 표시를 판단(`GetMapName().Contains("MenuLevel")`)하는 건 간단하지만 **문자열 의존**이라 약함. 맵이 늘어나면 GameMode를 레벨별로 분리하거나 데이터로 관리하는 편이 안전.
- 입력 모드/커서를 위젯 전환과 **세트로** 처리하지 않으면 "메뉴 떴는데 마우스 안 보임" 같은 버그가 생긴다.

---

## 3. UI 애니메이션 — C++에서 UMG 애니메이션 트리거

UMG 애니메이션은 C++에 직접 노출되지 않으므로, **위젯 BP에 커스텀 이벤트(`PlayGameOverAnim`)를 만들고 그 안에서 Play Animation** 한 뒤, C++에서 `FindFunction` + `ProcessEvent`로 호출한다.

```cpp
// ShowMainMenu() — bIsRestart일 때만
UFunction* PlayAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
if (PlayAnimFunc)
{
    MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
}
```

**배운 점 / 메모**
- `ProcessEvent`는 함수 이름(문자열) 기반이라 **오타/리네임에 취약**. 위젯 BP 이벤트 이름과 정확히 일치해야 한다.
- 더 견고하게 하려면 위젯 BP의 부모를 C++ `UUserWidget` 파생 클래스로 만들고 `BlueprintImplementableEvent`로 선언하는 방식이 타입 안전하다. (지금은 빠르게 가는 `ProcessEvent` 방식)

---

## 4. 3D 위젯 (위젯 컴포넌트) — 머리 위 체력바

`AMyPawn`에 `UWidgetComponent`를 붙여 월드 공간에 위젯을 띄운다. 여기선 `Screen` 스페이스로 두어 항상 카메라를 향하게(빌보드) 했다.

```cpp
// MyPawn.cpp — 생성자
OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
OverheadWidget->SetupAttachment(SkeletalMeshComp);
OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);   // 화면 정렬(빌보드)
```

### 위젯 컴포넌트 내부 위젯 갱신

`UWidgetComponent::GetUserWidgetObject()`로 실제 위젯 인스턴스를 얻은 뒤, HUD와 동일하게 자식 텍스트를 찾아 갱신.

```cpp
// MyPawn.cpp — UpdateOverheadHP()
UUserWidget* Inst = OverheadWidget->GetUserWidgetObject();
if (!Inst) return;
if (UTextBlock* HPText = Cast<UTextBlock>(Inst->GetWidgetFromName(TEXT("OverHeadHP"))))
{
    HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
}
```

호출 시점은 **값이 바뀔 때만**(이벤트 기반): `AddHealth`, `TakeDamage`, 그리고 `BeginPlay` 초기화.

**배운 점 / 메모**
- 위젯 컴포넌트는 `GetUserWidgetObject()`가 **위젯이 아직 생성되기 전이면 nullptr**일 수 있어 null 체크 필수. (Widget Class를 디테일에서 지정해야 인스턴스가 생긴다)
- `EWidgetSpace::Screen` = 항상 화면을 향함(크기 일정). `World`로 두면 실제 3D 평면처럼 원근/가림이 적용된다 → 체력바는 보통 `Screen`이 가독성 좋음.
- HUD와 달리 머리 위 HP는 **변경 이벤트에서만 갱신**(폴링 X). 자주 안 바뀌는 값이라 적절한 선택.

---

## 5. 데이터 영속성 — 레벨이 바뀌어도 유지

레벨을 `OpenLevel`로 전환하면 액터/GameState는 파괴되므로, **레벨을 가로지르는 데이터는 `UGameInstance`에 보관**한다.

```cpp
// MyGameInstance — 레벨 전환에도 살아남는 데이터
UPROPERTY(...) int32 TotalScore;
UPROPERTY(...) int32 CurrentLevelIndex;
```

- 점수: `GameState::AddScore` → `GameInstance::AddToScore`로 위임해 누적.
- 레벨 인덱스: `EndLevel`에서 `GameInstance->CurrentLevelIndex`에 저장 → 다음 레벨 `StartLevel`에서 복원.

**배운 점 / 메모**
- GameState는 "현재 레벨의 진행 상태", GameInstance는 "세션 전체 누적 데이터"로 **수명 범위에 따라 책임 분리**.
- HUD의 Score는 GameState가 아니라 **GameInstance의 TotalScore를 읽어** 표시 → 레벨이 바뀌어도 점수가 이어진다.

---

## 6. 오늘의 종합 회고

- **공통 패턴**: `CreateWidget` → `AddToViewport` → `GetWidgetFromName` + `Cast` → `SetText`. HUD, 메뉴, 3D 위젯 모두 동일한 골격.
- **갱신 전략 두 가지**를 상황에 맞게 사용:
  - 자주 변하는 값(시간) → **타이머 폴링**(HUD).
  - 가끔 변하는 값(HP) → **이벤트 기반 push**(머리 위 위젯).
- **개선 여지(다음에 시도)**:
  1. `GetWidgetFromName` 문자열 의존 → 위젯 BP를 C++ 파생 클래스로 만들고 `BindWidget`/`BlueprintImplementableEvent`로 타입 안전하게.
  2. `ProcessEvent`로 애니메이션 호출 → 동일하게 C++ 이벤트 선언으로 대체.
  3. 맵 이름 문자열 분기(`Contains("MenuLevel")`) → GameMode/데이터 기반으로.
