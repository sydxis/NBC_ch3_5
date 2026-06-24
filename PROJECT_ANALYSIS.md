# NBC_ch3_5 프로젝트 분석 문서

언리얼 엔진 C++ 프로젝트(`NBC_ch3_5`)의 핵심 시스템을 분석한 문서입니다.
아이템 스폰, 데이터테이블 기반 랜덤 스폰, 누적 확률 시스템, 캐릭터 체력,
데미지/회복 로직, 그리고 게임 모드/스테이트 기반 점수 관리까지를 다룹니다.

## 목차

1. [아이템 스폰 볼륨 만들기](#1-아이템-스폰-볼륨-만들기)
2. [데이터테이블로 아이템 랜덤 스폰](#2-데이터테이블로-아이템-랜덤-스폰)
3. [누적 확률 기반 시스템](#3-누적-확률-기반-시스템)
4. [캐릭터(MyPawn)에 체력 구현](#4-캐릭터mypawn에-체력-구현)
5. [데미지 및 회복 로직 처리](#5-데미지-및-회복-로직-처리)
6. [게임 스테이트와 게임 모드를 활용한 점수 관리 시스템](#6-게임-스테이트와-게임-모드를-활용한-점수-관리-시스템)
7. [발견된 이슈 / 개선 권장 사항](#7-발견된-이슈--개선-권장-사항)

---

## 프로젝트 클래스 구조 한눈에 보기

```
AActor
 └─ ABaseItem (IItemInterface 구현)         아이템 공통 베이스
     ├─ ACoinItem                            점수 획득 코인 (PointValue)
     │   ├─ ASmallCoinItem  (10점)
     │   └─ ABigCoinItem    (50점)
     ├─ AHealingItem                         체력 회복 (HealAmount)
     └─ AMineItem                            지뢰 - 데미지 (ExplosionDamage)

 └─ ASpawnVolume                             아이템 스폰 볼륨 (BoxComponent + DataTable)

APawn
 └─ AMyPawn                                  플레이어 캐릭터 (체력/이동/점프)

AGameStateBase
 └─ AMyGameStateBase                         전역 점수(Score) 관리

AGameMode
 └─ AMyGameMode                              규칙/클래스 등록

APlayerController
 └─ AMyPlayerController                      Enhanced Input 소유
```

| 데이터 구조체 | 역할 |
|---|---|
| `FItemSpawnRow` (FTableRowBase 상속) | 데이터테이블 한 행. `ItemName`, `ItemClass`, `SpawnChance` |
| `IItemInterface` | 아이템 공통 인터페이스(오버랩/활성화/타입) |

---

## 1. 아이템 스폰 볼륨 만들기

`ASpawnVolume`은 박스 영역(`UBoxComponent`) 내부의 임의 지점에 아이템을 스폰하는
액터입니다. 박스의 크기(Extent)와 위치를 기준으로 랜덤 좌표를 계산합니다.

**핵심 구성 요소** (`SpawnVolume.h`)

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
USceneComponent* Scene;          // 루트 컴포넌트
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
UBoxComponent* SpawningBox;      // 스폰 영역을 정의하는 박스

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning")
UDataTable* ItemDataTable;       // 스폰할 아이템 목록 데이터테이블

UFUNCTION(BlueprintCallable, Category="Spawning")
void SpawnRandomItem();          // 외부(블루프린트)에서 호출되는 진입점
```

**생성자에서의 컴포넌트 구성** (`SpawnVolume.cpp`)

```cpp
ASpawnVolume::ASpawnVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    SetRootComponent(Scene);

    SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
    SpawningBox->SetupAttachment(Scene);

    ItemDataTable = nullptr;
}
```

**박스 내부 랜덤 좌표 계산**

박스의 스케일이 적용된 Extent(반지름 개념)와 컴포넌트 월드 위치를 받아,
각 축마다 `-Extent ~ +Extent` 범위의 난수를 더해 균등 분포 좌표를 만듭니다.

```cpp
FVector ASpawnVolume::GetRandomPointInVolume() const
{
    const FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
    const FVector BoxOrigin = SpawningBox->GetComponentLocation();

    return BoxOrigin + FVector(
        FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
        FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
    );
}
```

**실제 스폰**

```cpp
void ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
    if (!ItemClass) return;

    GetWorld()->SpawnActor<AActor>(
        ItemClass,
        GetRandomPointInVolume(),
        FRotator::ZeroRotator
    );
}
```

### 에디터 사용 흐름
1. 레벨에 `BP_SpawnVolume`(또는 C++ 액터)을 배치한다.
2. 디테일 패널의 `SpawningBox`에서 Box Extent를 조정해 스폰 범위를 지정한다.
3. `ItemDataTable`에 아이템 목록 데이터테이블을 할당한다.
4. 타이머나 블루프린트 이벤트에서 `SpawnRandomItem()`을 주기적으로 호출한다.

---

## 2. 데이터테이블로 아이템 랜덤 스폰

스폰 대상 아이템을 코드에 하드코딩하지 않고 **데이터테이블**로 관리합니다.
한 행(Row)은 `FItemSpawnRow` 구조체로 정의됩니다.

**행 구조체 정의** (`ItemSpawnRow.h`)

```cpp
USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemName;                  // 아이템 이름(식별용)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ItemClass;   // 스폰할 액터 클래스

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnChance;               // 스폰 가중치(확률)
};
```

> `FTableRowBase`를 상속해야 언리얼 에디터에서 이 구조체를 행 타입으로 하는
> 데이터테이블 에셋을 생성할 수 있습니다.

**데이터테이블 예시**

| RowName | ItemName | ItemClass | SpawnChance |
|---------|----------|-----------|-------------|
| Small   | SmallCoin | BP_SmallCoinItem | 50 |
| Big     | BigCoin   | BP_BigCoinItem   | 20 |
| Heal    | Healing   | BP_HealingItem   | 20 |
| Mine    | Mine      | BP_MineItem      | 10 |

**선택된 행으로 스폰하기**

`GetRandomItem()`이 확률에 따라 한 행을 골라 반환하고,
`TSubclassOf`에서 실제 `UClass*`를 꺼내 스폰합니다.

```cpp
void ASpawnVolume::SpawnRandomItem()
{
    if (FItemSpawnRow* SelectedRow = GetRandomItem())
    {
        if (UClass* ActualClass = SelectedRow->ItemClass.Get())
        {
            SpawnItem(ActualClass);
        }
    }
}
```

### 데이터테이블 방식의 장점
- 디자이너가 코드 수정 없이 아이템 종류·확률을 조정 가능
- 새 아이템 추가 시 행만 추가하면 됨 (코드 변경 불필요)
- 밸런싱 데이터를 한 곳(에셋)에서 관리

---

## 3. 누적 확률 기반 시스템

각 행의 `SpawnChance`를 **가중치**로 보고, 가중치 비례 확률로 아이템을 선택합니다.
구현은 "누적 확률(Cumulative Probability)" 방식입니다.

**알고리즘** (`SpawnVolume.cpp` - `GetRandomItem()`)

```cpp
FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
    if (!ItemDataTable) return nullptr;

    // 1) 모든 행 가져오기
    TArray<FItemSpawnRow*> AllRows;
    static const FString ContextString(TEXT("ItemSpawnContext"));
    ItemDataTable->GetAllRows(ContextString, AllRows);
    if (AllRows.IsEmpty()) return nullptr;

    // 2) 전체 확률(가중치) 합 구하기
    float TotalChance = 0.0f;
    for (const FItemSpawnRow* Row : AllRows)
    {
        if (Row)
        {
            TotalChance += Row->SpawnChance;
        }
    }

    // 3) 0 ~ TotalChance 사이의 난수 추출
    const float RandValue = FMath::FRandRange(0.0f, TotalChance);
    float AccumulateChance = 0.0f;

    // 4) 누적 확률이 난수를 넘어서는 첫 행을 선택
    for (FItemSpawnRow* Row : AllRows)
    {
        AccumulateChance += Row->SpawnChance;
        if (RandValue <= AccumulateChance)
        {
            return Row;
        }
    }
    return nullptr;
}
```

### 동작 원리 (가중치 50 / 20 / 20 / 10, 합계 100 예시)

```
구간:   [0 ........ 50] (51 ... 70] (71 ... 90] (91 ... 100]
아이템:  SmallCoin       BigCoin     Healing     Mine
누적합:  50              70          90          100
```

- `RandValue`를 `0~100`에서 뽑는다.
- 누적합을 순서대로 더하며, **난수 ≤ 누적합**이 처음 성립하는 행을 반환한다.
- 가중치가 큰 아이템일수록 차지하는 구간이 넓어 더 자주 선택된다.

### 특징
- 확률의 합이 1(또는 100)일 필요가 없다. **상대 가중치**만 맞으면 된다.
- 행을 추가/삭제해도 `TotalChance`가 자동으로 재계산되어 자연스럽게 동작한다.

---

## 4. 캐릭터(MyPawn)에 체력 구현

`AMyPawn`은 `APawn`을 직접 상속한 플레이어 캐릭터로, 이동·점프(커스텀 중력)와
함께 체력 시스템을 가집니다.

**체력 관련 프로퍼티 / 함수** (`MyPawn.h`)

```cpp
// 최대 체력 (에디터에서 조정 가능)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
float MaxHealth;

// 현재 체력 (읽기 전용으로 노출)
UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Health")
float Health;

// 현재 체력 조회
UFUNCTION(BlueprintPure, Category="Health")
int32 GetHealth() const;

// 체력 회복
UFUNCTION(BlueprintCallable, Category="Health")
void AddHealth(float Amount);

// 사망 처리
UFUNCTION(BlueprintCallable, Category="Health")
virtual void OnDeath();

// 데미지 수신 (AActor 오버라이드)
virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                         AController* EventInstigator, AActor* DamageCauser) override;
```

**초기화** (`MyPawn.cpp` 생성자)

```cpp
MaxHealth = 100.0f;
Health    = MaxHealth;   // 시작 시 풀피
```

**현재 체력 조회**

```cpp
int32 AMyPawn::GetHealth() const
{
    return Health;   // float을 int32로 반환 (UI 표시용)
}
```

> 체력은 내부적으로 `float`이지만 `GetHealth()`는 `int32`로 반환합니다.
> UI에 정수 체력을 표시하기 위한 설계입니다.

---

## 5. 데미지 및 회복 로직 처리

체력의 증감은 두 경로로 이루어지며, 둘 다 `FMath::Clamp`로 `0 ~ MaxHealth`
범위를 벗어나지 않도록 보호합니다.

### 5-1. 회복 로직 — `AddHealth()`

```cpp
void AMyPawn::AddHealth(float Amount)
{
    // 최대 체력을 초과하지 않도록 Clamp
    Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("Health increased to: %f"), Health);
}
```

회복 아이템(`AHealingItem`)이 플레이어와 오버랩되면 이 함수를 호출합니다.

```cpp
// HealingItem.cpp
void AHealingItem::ActivateItem(AActor* Activator)
{
    if (Activator && Activator->ActorHasTag("Player"))
    {
        if (AMyPawn* PlayerCharacter = Cast<AMyPawn>(Activator))
        {
            PlayerCharacter->AddHealth(HealAmount);   // 기본 20.0f 회복
        }
    }
    DestroyItem();
}
```

### 5-2. 데미지 로직 — `TakeDamage()`

언리얼의 표준 데미지 파이프라인(`UGameplayStatics::ApplyDamage`)을 통해 호출되는
`TakeDamage()`를 오버라이드합니다.

```cpp
float AMyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                          AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                           EventInstigator, DamageCauser);

    // 체력 감소 (0 미만으로 떨어지지 않도록 Clamp)
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("Health decreased to: %f"), Health);

    // 체력이 0 이하이면 사망 처리
    if (Health <= 0.0f)
    {
        OnDeath();
    }
    return ActualDamage;
}

void AMyPawn::OnDeath()
{
    UE_LOG(LogTemp, Error, TEXT("Character is Dead!"));
    // 사망 후 로직 (리스폰, 게임오버 등 확장 지점)
}
```

### 5-3. 데미지를 유발하는 지뢰 — `AMineItem`

지뢰는 오버랩되면 일정 시간(`ExplosionDelay`) 뒤 폭발해, 폭발 반경 내 플레이어에게
`ApplyDamage`로 데미지를 전달합니다.

```cpp
// MineItem.cpp
void AMineItem::ActivateItem(AActor* Activator)
{
    // 타이머로 지연 폭발 예약
    GetWorld()->GetTimerManager().SetTimer(
        ExplosionTimerHandle, this, &AMineItem::Explode, ExplosionDelay, false);
}

void AMineItem::Explode()
{
    TArray<AActor*> OverlappingActors;
    ExplosionCollision->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->ActorHasTag("Player"))
        {
            UGameplayStatics::ApplyDamage(
                Actor,                      // 데미지 대상
                ExplosionDamage,            // 데미지 양 (기본 30.0f)
                nullptr,                    // Instigator (없음)
                this,                       // DamageCauser (지뢰)
                UDamageType::StaticClass()  // 기본 데미지 타입
            );
        }
    }
    DestroyItem();
}
```

### 데미지/회복 데이터 흐름 요약

```
[회복]  AHealingItem 오버랩 → ActivateItem() → AMyPawn::AddHealth()
                                                  └ Clamp(0, MaxHealth)

[데미지] AMineItem 오버랩 → 타이머 → Explode()
            → UGameplayStatics::ApplyDamage()
            → AMyPawn::TakeDamage() → Clamp(0, MaxHealth)
            → Health <= 0 이면 OnDeath()
```

---

## 6. 게임 스테이트와 게임 모드를 활용한 점수 관리 시스템

전역 점수는 **GameState**가 보관하고, 코인 아이템이 점수를 더하며,
**GameMode**가 규칙·사용할 클래스들을 등록하는 구조입니다.

### 6-1. 점수를 보관하는 GameState — `AMyGameStateBase`

```cpp
// MyGameStateBase.h
UCLASS()
class NBC_CH3_5_API AMyGameStateBase : public AGameStateBase
{
    GENERATED_BODY()
public:
    AMyGameStateBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Score")
    int32 Score;

    UFUNCTION(BlueprintPure, Category="Score")
    int32 GetScore() const;

    UFUNCTION(BlueprintCallable, Category="Score")
    void AddScore(int32 Amount);
};
```

```cpp
// MyGameStateBase.cpp
AMyGameStateBase::AMyGameStateBase() { Score = 0; }

int32 AMyGameStateBase::GetScore() const { return Score; }

void AMyGameStateBase::AddScore(int32 Amount) { Score += Amount; }
```

> **GameState를 쓰는 이유**: 점수는 특정 플레이어 폰이 아니라 "게임 전체"에 속하는
> 상태이므로, 레벨 내 어디서든 `GetGameState<AMyGameStateBase>()`로 접근할 수 있는
> GameState에 두는 것이 적절합니다.

### 6-2. 점수를 더하는 코인 아이템 — `ACoinItem` 계열

```cpp
// CoinItem.cpp
void ACoinItem::ActivateItem(AActor* Activator)
{
    if (Activator && Activator->ActorHasTag("Player"))
    {
        if (UWorld* World = GetWorld())
        {
            if (AMyGameStateBase* GameState = World->GetGameState<AMyGameStateBase>())
            {
                GameState->AddScore(PointValue);   // GameState에 점수 누적
            }
        }
        DestroyItem();
    }
}
```

코인은 상속을 통해 점수 값을 차등화합니다.

| 클래스 | PointValue | ItemType |
|--------|-----------|----------|
| `ACoinItem` (기본) | 0 | "DefaultCoin" |
| `ASmallCoinItem` | 10 | "SmallCoin" |
| `ABigCoinItem` | 50 | "BigCoin" |

```cpp
// SmallCoinItem.cpp 생성자
PointValue = 10;  ItemType = "SmallCoin";

// BigCoinItem.cpp 생성자
PointValue = 50;  ItemType = "BigCoin";
```

서브클래스는 `ActivateItem()`에서 `Super::ActivateItem(Activator)`만 호출해
부모의 점수 처리 로직을 그대로 재사용합니다.

### 6-3. 규칙을 정의하는 GameMode — `AMyGameMode`

```cpp
// MyGameMode.cpp
AMyGameMode::AMyGameMode()
{
    PlayerControllerClass = AMyPlayerController::StaticClass();
    DefaultPawnClass      = AMyPlayerController::StaticClass();   // ⚠ 7장 참고
    GameStateClass        = AMyPlayerController::StaticClass();   // ⚠ 7장 참고
}
```

GameMode는 게임에서 사용할 PlayerController / Pawn / GameState 클래스를 등록하는
역할을 합니다. (단, 현재 코드에는 잘못 등록된 부분이 있어 7장에서 설명합니다.)

### 점수 시스템 데이터 흐름 요약

```
플레이어가 코인과 오버랩
  → ABaseItem::OnItemStartOverlap (태그 "Player" 확인)
  → ACoinItem::ActivateItem(PointValue)
  → World->GetGameState<AMyGameStateBase>()->AddScore(PointValue)
  → AMyGameStateBase::Score 누적
  → UI 등에서 GetScore()로 조회
```

---

## 7. 발견된 이슈 / 개선 권장 사항

분석 중 확인된, 실행 시 문제가 될 수 있는 부분과 개선 포인트입니다.

### 🔴 (버그) `AMyGameMode` 생성자의 잘못된 클래스 등록

`MyGameMode.cpp`에서 `DefaultPawnClass`와 `GameStateClass`가 모두
`AMyPlayerController::StaticClass()`로 설정되어 있습니다.

```cpp
// 현재 (잘못됨)
PlayerControllerClass = AMyPlayerController::StaticClass();
DefaultPawnClass      = AMyPlayerController::StaticClass();   // 폰이어야 함
GameStateClass        = AMyPlayerController::StaticClass();   // 게임스테이트여야 함
```

이 상태면 폰으로 컨트롤러가 스폰되려 하고, GameState도 컨트롤러로 생성되어
`GetGameState<AMyGameStateBase>()`가 **항상 nullptr**을 반환합니다.
→ **코인을 먹어도 점수가 오르지 않습니다.**

```cpp
// 권장 수정
PlayerControllerClass = AMyPlayerController::StaticClass();
DefaultPawnClass      = AMyPawn::StaticClass();              // 플레이어 폰
GameStateClass        = AMyGameStateBase::StaticClass();     // 점수 보관 GameState
```

> 수정 시 `MyGameMode.cpp` 상단에 `#include "MyPawn.h"`,
> `#include "MyGameStateBase.h"`를 추가해야 합니다.

### 🟡 플레이어 태그("Player") 설정 필요

아이템 활성화 로직 전반이 `OtherActor->ActorHasTag("Player")`에 의존합니다.
`AMyPawn`에 `Tags.Add("Player")` 설정(생성자 또는 BeginPlay)이나 블루프린트에서
태그 부여가 누락되면 모든 아이템이 동작하지 않습니다.

### 🟡 `GetHealth()`의 float → int32 변환

체력을 `float`로 관리하면서 `GetHealth()`는 `int32`로 반환합니다. 소수점 체력이
잘려서 표시되므로, 의도된 동작인지 확인이 필요합니다. (UI 표시는 정수가 자연스러움)

### 🟢 (개선) 지뢰 폭발 반경 일관성

`AMineItem` 생성자에서 `ExplosionCollision->InitSphereRadius(ExplosionRadius)`를
호출하지만, `ExplosionRadius`를 에디터에서 변경해도 콜리전 반경은 생성자 시점 값으로
고정됩니다. 런타임 반영이 필요하면 `BeginPlay`에서 `SetSphereRadius`를 호출하세요.

---

## 부록: 주요 파일 위치

| 기능 | 파일 |
|------|------|
| 스폰 볼륨 | `Source/NBC_ch3_5/Public/SpawnVolume.h`, `Private/SpawnVolume.cpp` |
| 데이터테이블 행 | `Source/NBC_ch3_5/Public/ItemSpawnRow.h` |
| 아이템 베이스 | `Source/NBC_ch3_5/Public/BaseItem.h`, `Private/BaseItem.cpp` |
| 아이템 인터페이스 | `Source/NBC_ch3_5/ItemInterface.h` |
| 코인 / 회복 / 지뢰 | `CoinItem`, `SmallCoinItem`, `BigCoinItem`, `HealingItem`, `MineItem` |
| 플레이어 캐릭터 | `Source/NBC_ch3_5/Public/MyPawn.h`, `Private/MyPawn.cpp` |
| 게임 스테이트 | `Source/NBC_ch3_5/Public/MyGameStateBase.h`, `Private/MyGameStateBase.cpp` |
| 게임 모드 | `Source/NBC_ch3_5/Public/MyGameMode.h`, `Private/MyGameMode.cpp` |
| 플레이어 컨트롤러 | `Source/NBC_ch3_5/Public/MyPlayerController.h` |
