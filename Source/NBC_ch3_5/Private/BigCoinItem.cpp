// Fill out your copyright notice in the Description page of Project Settings.


#include "BigCoinItem.h"


// Sets default values
ABigCoinItem::ABigCoinItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PointValue = 50;
	ItemType = "BigCoin";
}

void ABigCoinItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
}

// Called when the game starts or when spawned
void ABigCoinItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABigCoinItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

