// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBC_ch3_5/ItemInterface.h"
#include "BaseItem.generated.h"

class USphereComponent;
class USoundBase;
class UParticleSystem;

UCLASS()
class NBC_CH3_5_API ABaseItem : public AActor, public IItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	
	virtual void OnItemStartOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;
	virtual void OnItemEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;
	
	virtual void ActivateItem(AActor* Activator) override;
	virtual FName GetItemType() const override;
	
	virtual void DestroyItem();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	USphereComponent *Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|FX")
	USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|FX")
	UParticleSystem* PickupEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|FX")
	float EffectLifeSpan = 2.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
