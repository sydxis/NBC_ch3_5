// Fill out your copyright notice in the Description page of Project Settings.


#include "HealingItem.h"

#include "MyCharacter.h"


// Sets default values
AHealingItem::AHealingItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	HealAmount = 20.0f;
	ItemType = "Healing";
}

void AHealingItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Green,FString::Printf(TEXT("Player Gained %f HP"), HealAmount));
		if (AMyCharacter* PlayerCharacter = Cast<AMyCharacter>(Activator))
		{
			PlayerCharacter->AddHealth(HealAmount);
		}
	}
	DestroyItem();
}

// Called when the game starts or when spawned
void AHealingItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHealingItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

