// Fill out your copyright notice in the Description page of Project Settings.


#include "ReverseItem.h"
#include "MyCharacter.h"


// Sets default values
AReverseItem::AReverseItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ItemType = "Reverse";
}

void AReverseItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Red,FString::Printf(TEXT("Player Input Reverse")));
		if (AMyCharacter* PlayerCharacter = Cast<AMyCharacter>(Activator))
		{
			PlayerCharacter->ReverseInput();
		}
	}
	DestroyItem();
}

// Called when the game starts or when spawned
void AReverseItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AReverseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

