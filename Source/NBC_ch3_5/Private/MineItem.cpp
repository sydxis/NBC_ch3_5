// Fill out your copyright notice in the Description page of Project Settings.


#include "MineItem.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMineItem::AMineItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ExplosionDelay = 5.0f;
	ExplosionRadius = 300.0f;
	ExplosionDamage = 30.0f;
	ItemType = "Mine";
	
	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExplosionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ExplosionCollision->SetupAttachment(Scene);
}

void AMineItem::ActivateItem(AActor* Activator)
{
	GetWorld()->GetTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&AMineItem::Explode,
		ExplosionDelay,
		false);
}

void AMineItem::Explode()
{
	TArray<AActor*> OverlappingActors;
	ExplosionCollision->GetOverlappingActors(OverlappingActors);
	
	for(AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Red,FString::Printf(TEXT("Player Gained %f Damage"), ExplosionDamage));
			UGameplayStatics::ApplyDamage(
				  Actor,              
				  ExplosionDamage,     
				  nullptr,                   
				  this,                       
				  UDamageType::StaticClass() 
			  );
		}
	}
	DestroyItem();
}


// Called when the game starts or when spawned
void AMineItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMineItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

