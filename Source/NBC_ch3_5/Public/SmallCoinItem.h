// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoinItem.h"
#include "SmallCoinItem.generated.h"

UCLASS()
class NBC_CH3_5_API ASmallCoinItem : public ACoinItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASmallCoinItem();
	
	virtual void ActivateItem(AActor* Activator) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
