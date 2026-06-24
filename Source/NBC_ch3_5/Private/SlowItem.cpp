// Fill out your copyright notice in the Description page of Project Settings.


#include "SlowItem.h"
#include "MyPawn.h"


// Sets default values
ASlowItem::ASlowItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ItemType = "Slow";
}

void ASlowItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Blue,FString::Printf(TEXT("Player Slowed")));
		if (AMyPawn* PlayerCharacter = Cast<AMyPawn>(Activator))
		{
			PlayerCharacter->GetSlow();
		}
	}
	DestroyItem();
}

// Called when the game starts or when spawned
void ASlowItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlowItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

