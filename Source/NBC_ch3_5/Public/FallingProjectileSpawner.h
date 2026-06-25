// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingProjectileSpawner.generated.h"

class UBoxComponent;
class AFallingProjectile;

UCLASS()
class NBC_CH3_5_API AFallingProjectileSpawner : public AActor
{
	GENERATED_BODY()

public:
	AFallingProjectileSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AFallingProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnInterval;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnProjectile();

	FVector GetRandomPointInVolume() const;

private:
	FTimerHandle SpawnTimerHandle;
};
