# TIL — NBC_ch3_5 게임 흐름/상태 관리 구현 (2026-06-15)

> 엔진: Unreal Engine 5.7 · 모듈: `NBC_ch3_5`
> 오늘의 주제: **게임 프레임워크 3총사(GameMode · GameState · GameInstance)를 활용한 게임 루프 구축**

---

## 0. 오늘 구현한 것 한눈에 보기

- 🎮 **GameMode** — 규칙/클래스 등록 (Pawn, Controller, GameState 지정)
- 📊 **GameState** — 레벨 진행/점수/코인 카운트 등 **게임 진행 상태**와 **게임 루프**
- ♻️ **GameInstance** — 맵이 바뀌어도 살아남는 **누적 데이터**(TotalScore 등) 유지
- 🪙 **CoinItem** — 코인 획득 시 점수 가산 + 코인 수집 카운트 통지
- 📦 **SpawnVolume** — 레벨 시작 시 데이터테이블 기반 확률 아이템 스폰
- 🔁 **게임 흐름** — 3레벨 / 각 30초 / 코인 전부 획득 시 즉시 다음 레벨 / 3레벨 종료 후 게임오버 / 맵 전환

### 프레임워크 역할 분담 (수명 주기 기준)

```
UGameInstance   ── 게임 실행 ~ 종료까지 1개 (맵 전환에도 파괴 X) → 누적 데이터 보관
      │
AGameMode       ── 서버에만 존재, 맵마다 새로 생성 → "규칙"과 클래스 등록
      │
AGameState      ── 모든 클라가 공유, 맵마다 새로 생성 → "현재 진행 상태" + 게임 루프
      │
ACoinItem 등    ── 월드에 스폰되는 액터 → GameState에 점수/수집 통지
```

> 핵심 직관: **"맵이 바뀌면 사라져도 되는 상태"는 GameState**, **"맵이 바뀌어도 유지돼야 하는 데이터"는 GameInstance**.

---

# 파트 1. GameMode — 규칙과 클래스 등록

`AMyGameMode`는 어떤 Pawn/Controller/GameState 클래스를 쓸지 **생성자에서 등록**만 한다. 게임 규칙의 "엔트리포인트" 역할.

```cpp
AMyGameMode::AMyGameMode()
{
    PlayerControllerClass = AMyPlayerController::StaticClass();
    DefaultPawnClass      = AMyPawn::StaticClass();
    GameStateClass        = AMyGameState::StaticClass();   // ★ 커스텀 GameState 연결
}
```

### 💡 배운 점
- `GameStateClass`를 지정해야 `World->GetGameState<AMyGameState>()`로 내 커스텀 상태에 접근 가능.
- GameMode는 **서버에만** 존재 → 클라이언트가 공유해야 하는 값은 GameMode가 아니라 **GameState**에 둔다.
- 프로젝트에 `AMyGameStateBase`(GameStateBase 상속)도 있지만, 실제 게임 루프는 **`AMyGameState`(GameState 상속)** 가 담당. → GameMode가 등록한 건 후자.

---

# 파트 2. GameState — 게임 루프의 심장

`AMyGameState`가 레벨 타이머, 코인 카운트, 레벨 인덱스, 맵 전환까지 전부 관리한다.

### 보유 상태 변수

| 변수 | 의미 |
|------|------|
| `Score` | 현재 점수 |
| `SpawnedCoinCount` | 이번 레벨에 스폰된 코인 수 |
| `CollectedCoinCount` | 이번 레벨에서 획득한 코인 수 |
| `LevelDuration` | 레벨 제한 시간 (30초) |
| `CurrentLevelIndex` | 현재 레벨 인덱스 (0부터) |
| `MaxLevels` | 최대 레벨 수 (3) |
| `LevelMapNames` | 레벨별 맵 이름 배열 |
| `LevelTimerHandle` | 제한 시간 타이머 핸들 |

### 게임 루프 흐름도

```
BeginPlay()
   └─> StartLevel()
          ├─ 코인 카운트 초기화
          ├─ SpawnVolume 찾아 40개 아이템 스폰 (코인이면 SpawnedCoinCount++)
          └─ 30초 타이머 시작 ──► OnLevelTimeUp() ──┐
                                                     ├─► EndLevel()
   코인 획득 ──► OnCoinCollected()                   │
          └─ CollectedCoinCount++                    │
          └─ 전부 모았으면(>= Spawned) ──────────────┘
                                                     │
                              EndLevel() ────────────┘
                                 ├─ 타이머 Clear
                                 ├─ CurrentLevelIndex++
                                 ├─ 마지막 레벨 초과? ─► OnGameOver()
                                 └─ 다음 맵 유효? ─► OpenLevel(다음 맵)
```

### ① 레벨 시작 + 아이템 스폰

```cpp
void AMyGameState::StartLevel()
{
    SpawnedCoinCount = 0;
    CollectedCoinCount = 0;

    TArray<AActor*> FoundVolumes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

    const int32 ItemToSpawn = 40;
    for (int32 i = 0; i < ItemToSpawn; i++)
    {
        if (FoundVolumes.Num() > 0)
        {
            ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
            if (SpawnVolume)
            {
                AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
                // 스폰된 게 코인 종류면 카운트 → 클리어 조건의 분모
                if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
                {
                    SpawnedCoinCount++;
                }
            }
        }
    }

    GetWorldTimerManager().SetTimer(
        LevelTimerHandle, this, &AMyGameState::OnLevelTimeUp, LevelDuration, false);
}
```

### ② 코인 전부 모으면 즉시 다음 레벨

```cpp
void AMyGameState::OnCoinCollected()
{
    CollectedCoinCount++;
    // 스폰된 코인을 전부 모았으면 30초를 기다리지 않고 바로 종료
    if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
    {
        EndLevel();
    }
}
```

### ③ 레벨 종료 → 다음 맵 or 게임오버

```cpp
void AMyGameState::EndLevel()
{
    GetWorldTimerManager().ClearTimer(LevelTimerHandle);  // ★ 타이머 정리 필수
    CurrentLevelIndex++;

    if (CurrentLevelIndex >= MaxLevels)   // 3레벨 다 끝남
    {
        OnGameOver();
        return;
    }
    if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
    {
        UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);  // 맵 전환
    }
    else
    {
        OnGameOver();
    }
}
```

### 💡 배운 점
- **타이머 종료 경로와 코인-클리어 경로가 둘 다 `EndLevel()`로 수렴** → 종료 처리를 한 곳에 모아 중복/누락 방지.
- `EndLevel()` 진입 시 **`ClearTimer` 먼저** 호출해야, 코인으로 일찍 끝났을 때 남은 타이머가 다음 레벨에서 또 발동하는 버그를 막는다.
- `IsValidIndex()`로 맵 배열 범위를 검사 → 맵이 부족해도 크래시 없이 게임오버로 안전하게 폴백.
- `SpawnedCoinCount > 0` 가드: 코인이 하나도 안 스폰된 레벨에서 `0 >= 0`이 참이 되어 **즉시 종료되는 버그** 방지.

---

# 파트 3. CoinItem — 점수 획득 로직

코인은 플레이어가 먹으면 **GameState에 점수를 더하고, 코인 수집을 통지한 뒤 자신을 파괴**한다.

```cpp
void ACoinItem::ActivateItem(AActor* Activator)
{
    if (Activator && Activator->ActorHasTag("Player"))   // 플레이어만 획득 가능
    {
        if (UWorld* World = GetWorld())
        {
            if (AMyGameState* GameState = World->GetGameState<AMyGameState>())
            {
                GameState->AddScore(PointValue);     // 점수 가산
                GameState->OnCoinCollected();        // 클리어 카운트 통지
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
                    FString::Printf(TEXT("Player Gained %d Points"), PointValue));
            }
        }
        DestroyItem();
    }
}
```

- `ABigCoinItem` → `PointValue = 50`, `ASmallCoinItem` → 10점 등 자식이 점수만 다르게 설정.
- 자식은 `Super::ActivateItem(Activator)`만 호출 → 획득 로직은 부모 `ACoinItem` 한 곳에 집중.

### 💡 배운 점
- 액터(코인)가 게임 진행 상태(GameState)에 **단방향으로 통지**하는 구조 → 코인은 "내가 몇 번째인지" 몰라도 됨. 책임 분리.
- `ActorHasTag("Player")`로 획득 주체를 검증 → 다른 액터의 우발적 오버랩 차단.

---

# 파트 4. SpawnVolume — 확률 기반 아이템 스폰

데이터테이블(`FItemSpawnRow`)에서 **누적 확률(weighted random)** 로 아이템 클래스를 골라 박스 볼륨 내 랜덤 위치에 스폰.

```cpp
// 1) 전체 확률 합   2) 0~합 사이 난수   3) 누적하며 구간에 걸리는 Row 선택
const float RandValue = FMath::FRandRange(0.0f, TotalChance);
float AccumulateChance = 0.0f;
for (FItemSpawnRow* Row : AllRows)
{
    AccumulateChance += Row->SpawnChance;
    if (RandValue <= AccumulateChance) return Row;   // 가중치 비례 선택
}
```

- `FItemSpawnRow`: `ItemName` / `ItemClass(TSubclassOf<AActor>)` / `SpawnChance` 3필드 구조체(`FTableRowBase` 상속).
- 스폰 위치는 `BoxComponent`의 Extent 범위에서 `FRandRange`로 추출.

### 💡 배운 점
- **누적 확률 기법**: 각 확률을 더해가며 난수가 걸리는 첫 구간을 고르면, 확률 값 비율대로 자연스럽게 가중 선택됨.
- 스폰 데이터를 코드가 아닌 **데이터테이블(에디터)** 로 분리 → 밸런싱을 빌드 없이 조정 가능.

---

# 파트 5. GameInstance — 맵을 넘나드는 데이터 유지

`OpenLevel()`로 맵이 바뀌면 GameMode/GameState/모든 액터는 **파괴되고 새로 생성**된다. 이때 살아남아야 하는 누적값은 `UMyGameInstance`에 둔다.

```cpp
UCLASS()
class NBC_CH3_5_API UMyGameInstance : public UGameInstance
{
    UPROPERTY(...) int32 TotalScore;          // 전체 누적 점수
    UPROPERTY(...) int32 CurrentLevelIndex;   // 진행 중 레벨 (맵 전환에도 유지)

    UFUNCTION(BlueprintCallable, Category = "GameData")
    void AddToScore(int32 Amount);            // TotalScore += Amount
};
```

### 💡 배운 점
- **GameState vs GameInstance 수명**:
  - `GameState.Score` → 맵 전환 시 리셋됨 (그 레벨 한정).
  - `GameInstance.TotalScore` → 게임 실행 내내 유지 (전체 누적).
- 레벨 클리어 점수를 다음 레벨로 이어가려면 `EndLevel()`에서 GameState 점수를 **GameInstance에 합산**하는 흐름이 필요 (다음 연결 포인트).
- GameInstance는 프로젝트 설정에서 **Game Instance Class로 등록**해야 실제로 사용됨.

---

## 🧩 오늘의 핵심 정리

1. **GameMode = 규칙/등록, GameState = 진행 상태/루프, GameInstance = 영속 데이터** — 수명 주기로 역할을 가른다.
2. 게임 루프의 종료 경로(타이머 만료 / 코인 전부 획득)를 `EndLevel()` 한 곳으로 모아 **상태 전이를 단순화**.
3. 타이머는 **다 쓰면 즉시 `ClearTimer`** — 조기 종료 시 잔여 타이머 폭발 방지.
4. 코인 같은 월드 액터는 **GameState에 단방향 통지**만 — 진행 상태를 직접 알 필요 없게 책임 분리.
5. 맵 전환(`OpenLevel`)은 모든 액터를 날리므로, 유지할 값은 반드시 **GameInstance**로.

---

> 다음 단계 후보: EndLevel 시 GameState 점수 → GameInstance 누적 반영 / 게임오버 UI / 남은 시간·점수 HUD 표시
