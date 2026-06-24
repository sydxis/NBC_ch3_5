// Fill out your copyright notice in the Description page of Project Settings.


#include "CoinItem.h"
#include "MyGameState.h"

ACoinItem::ACoinItem()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PointValue = 0;
	ItemType = "DefaultCoin";
}

void ACoinItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (UWorld* World = GetWorld())
		{
			if (AMyGameState* GameState = World->GetGameState<AMyGameState>())
			{
				GameState->AddScore(PointValue);
				GameState->OnCoinCollected();
				GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Yellow,FString::Printf(TEXT("Player Gained %d Points"), PointValue));
			}
		}
	DestroyItem();
	}
}

// Called when the game starts or when spawned
void ACoinItem::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACoinItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

