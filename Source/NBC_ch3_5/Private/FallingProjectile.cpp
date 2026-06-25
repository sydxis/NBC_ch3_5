// Fill out your copyright notice in the Description page of Project Settings.


#include "FallingProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFallingProjectile::AFallingProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Damage = 20.0f;
	FallSpeed = 300.0f;
	InitialLifeSpan = 5.0f;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(50.0f);
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SphereMesh->SetupAttachment(SphereCollision);
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AFallingProjectile::OnProjectileOverlap);
}

// Called when the game starts or when spawned
void AFallingProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = FVector(0.0f, 0.0f, -FallSpeed);
	}
}

void AFallingProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			nullptr,
			this,
			UDamageType::StaticClass());

		Destroy();
	}
}
