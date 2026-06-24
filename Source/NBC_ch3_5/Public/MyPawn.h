// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

class UWidgetComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class NBC_CH3_5_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* OverheadWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadSlow;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadReverse;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
	
protected:
	void Move(const FInputActionValue& Value);    
	void Look(const FInputActionValue& Value);  
	void StartJump(const FInputActionValue& Value);

	void PerformGroundCheck();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LookSensitivity = 1.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpZVelocity = 600.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityZ = 980.0f;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundCheckDistance = 5.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	float Health;

	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void OnDeath();
	
	void UpdateOverheadHP();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
private:
	FVector Velocity = FVector::ZeroVector;
	FVector2D MoveInput = FVector2D::ZeroVector;
	bool bIsGrounded = false;
	FTimerHandle SlowTimer;
	FTimerHandle ReverseInputTimer;
	
	float InputMult = 1.0f;
	
public:
	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);
	
	void GetSlow();
	void EndSlowTimer();
	void ReverseInput();
	void EndReverseInput();
	
	UFUNCTION(BlueprintCallable, Category = "Timer")
	float GetSlowRemaining();
	
	UFUNCTION(BlueprintCallable, Category = "Timer")
	float GetReverseRemaining();
};