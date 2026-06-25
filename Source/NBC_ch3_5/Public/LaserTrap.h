// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserTrap.generated.h"

class UBoxComponent;
class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS()
class NBC_CH3_5_API ALaserTrap : public AActor
{
	GENERATED_BODY()

public:
	ALaserTrap();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	UBoxComponent* StartTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	UBoxComponent* EndTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	USceneComponent* LaserPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	UCapsuleComponent* LaserCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser|Component")
	UStaticMeshComponent* LaserMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser|Movement")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser|Damage")
	float DamagePerSecond;

private:
	void ApplyLaserDamage(float DeltaTime);

	FVector StartLocation;
	FVector EndLocation;
	FVector TargetLocation;
	bool bMovingToEnd;
};
