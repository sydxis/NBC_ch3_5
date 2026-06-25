// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_5_API AMyGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AMyGameState();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Score")
	int32 Score;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	float LevelDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level|Spawn")
	int32 BaseItemCount = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level|Spawn")
	int32 ItemCountPerLevel = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level|Time")
	float BaseLevelDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level|Time")
	float LevelDurationPerLevel = -5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;
	
	FTimerHandle LevelTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;
	
	
	
	UFUNCTION(BlueprintPure, Category="Score")
	int32 GetScore() const;

	UFUNCTION(BlueprintCallable, Category="Score")
	void AddScore(int32 Amount);
	
	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver(bool bCleared = false);
	
	void StartLevel();
	void OnLevelTimeUp();
	void OnCoinCollected();
	void EndLevel();
	void UpdateHUD();
	
};
