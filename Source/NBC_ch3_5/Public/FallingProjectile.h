// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class NBC_CH3_5_API AFallingProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFallingProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	UStaticMeshComponent* SphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float FallSpeed;
};
