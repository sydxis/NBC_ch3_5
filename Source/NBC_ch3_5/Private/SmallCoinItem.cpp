// Fill out your copyright notice in the Description page of Project Settings.


#include "SmallCoinItem.h"


// Sets default values
ASmallCoinItem::ASmallCoinItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PointValue = 10;
	ItemType = "SmallCoin";
}

void ASmallCoinItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
}

// Called when the game starts or when spawned
void ASmallCoinItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASmallCoinItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

