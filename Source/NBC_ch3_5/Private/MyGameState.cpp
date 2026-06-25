// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "MyPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "MyGameInstance.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

AMyGameState::AMyGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 120.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
}

int32 AMyGameState::GetScore() const
{
	return Score;
}

void AMyGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (MyGameInstance)
		{
			MyGameInstance->AddToScore(Amount);
		}
	}
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUD();
	StartLevel();
	
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&AMyGameState::UpdateHUD,
		0.1f,
		true
	);
}

void AMyGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			MyPlayerController->ShowGameHUD();
		}
	}
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (MyGameInstance)
		{
			CurrentLevelIndex = MyGameInstance->CurrentLevelIndex;
		}
	}
	
	
		SpawnedCoinCount = 0;
		CollectedCoinCount = 0;
	
		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	
		const int32 ItemToSpawn = FMath::Max(0, BaseItemCount + CurrentLevelIndex * ItemCountPerLevel);
		LevelDuration = FMath::Max(1.0f, BaseLevelDuration + CurrentLevelIndex * LevelDurationPerLevel);
		
		for (int32 i = 0; i < ItemToSpawn; i++)
		{
				if (FoundVolumes.Num() > 0)
						{
						ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
						if (SpawnVolume)
						{
								AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
								if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
								{
										SpawnedCoinCount++;
								}
						}				
				}
		}
	
		GetWorldTimerManager().SetTimer(
			LevelTimerHandle,
			this,
			&AMyGameState::OnLevelTimeUp,
			LevelDuration,
			false
		);
	
		UpdateHUD();
	
		UE_LOG(LogTemp, Warning, TEXT("Level %d Start!, Spawned %d coin"),
			CurrentLevelIndex + 1,
			SpawnedCoinCount);
}

void AMyGameState::OnLevelTimeUp()
{
	OnGameOver();
}

void AMyGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d / %d"), 
	CollectedCoinCount,
	SpawnedCoinCount)
	
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndLevel();
	}
}

void AMyGameState::EndLevel()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (MyGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			MyGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}
	
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver(true);
		return;
	}
		
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver(true);
	}
}

void AMyGameState::OnGameOver(bool bCleared)
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			MyPlayerController->SetPause(true);
			MyPlayerController->ShowGameOver(bCleared);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Game Over!! (Cleared=%d)"), bCleared ? 1 : 0);
}

void AMyGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController);
		{
			if (UUserWidget* HUDWidget = MyPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}
				
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance);
						if (MyGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), MyGameInstance->TotalScore)));
						}
					}
				}
				
				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}
			}
		}
	}
}