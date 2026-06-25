// Fill out your copyright notice in the Description page of Project Settings.


#include "FallingProjectileSpawner.h"
#include "FallingProjectile.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

// Sets default values
AFallingProjectileSpawner::AFallingProjectileSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnInterval = 1.0f;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
	SpawningBox->SetBoxExtent(FVector(200.0f, 200.0f, 50.0f));
	SpawningBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AFallingProjectileSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AFallingProjectileSpawner::SpawnProjectile,
			SpawnInterval,
			true);
	}
}

void AFallingProjectileSpawner::SpawnProjectile()
{
	if (!ProjectileClass) return;

	GetWorld()->SpawnActor<AFallingProjectile>(
		ProjectileClass,
		GetRandomPointInVolume(),
		FRotator::ZeroRotator);
}

FVector AFallingProjectileSpawner::GetRandomPointInVolume() const
{
	const FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	const FVector BoxOrigin = SpawningBox->GetComponentLocation();

	return BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}
