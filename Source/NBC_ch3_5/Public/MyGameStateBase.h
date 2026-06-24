#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

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