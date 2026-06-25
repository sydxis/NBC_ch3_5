// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserTrap.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALaserTrap::ALaserTrap()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveSpeed = 200.0f;
	DamagePerSecond = 50.0f;
	bMovingToEnd = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	StartTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StartTrigger"));
	StartTrigger->SetupAttachment(Scene);
	StartTrigger->SetBoxExtent(FVector(50.0f));
	StartTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StartTrigger->SetHiddenInGame(true);

	EndTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EndTrigger"));
	EndTrigger->SetupAttachment(Scene);
	EndTrigger->SetBoxExtent(FVector(50.0f));
	EndTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EndTrigger->SetHiddenInGame(true);
	EndTrigger->SetRelativeLocation(FVector(500.0f, 0.0f, 0.0f));

	LaserPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LaserPivot"));
	LaserPivot->SetupAttachment(Scene);

	LaserCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LaserCollision"));
	LaserCollision->SetupAttachment(LaserPivot);
	LaserCollision->SetCapsuleSize(20.0f, 200.0f);
	LaserCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
	LaserMesh->SetupAttachment(LaserPivot);
	LaserMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ALaserTrap::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = StartTrigger->GetComponentLocation();
	EndLocation = EndTrigger->GetComponentLocation();

	LaserPivot->SetWorldLocation(StartLocation);
	bMovingToEnd = true;
	TargetLocation = EndLocation;
}

// Called every frame
void ALaserTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrentLocation = LaserPivot->GetComponentLocation();
	const float Step = MoveSpeed * DeltaTime;

	if (FVector::Dist(CurrentLocation, TargetLocation) <= Step)
	{
		LaserPivot->SetWorldLocation(TargetLocation);
		bMovingToEnd = !bMovingToEnd;
		TargetLocation = bMovingToEnd ? EndLocation : StartLocation;
	}
	else
	{
		const FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
		LaserPivot->SetWorldLocation(CurrentLocation + Direction * Step);
	}

	ApplyLaserDamage(DeltaTime);
}

void ALaserTrap::ApplyLaserDamage(float DeltaTime)
{
	TArray<AActor*> OverlappingActors;
	LaserCollision->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				DamagePerSecond * DeltaTime,
				nullptr,
				this,
				UDamageType::StaticClass());
		}
	}
}
