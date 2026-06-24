// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MyGameState.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"


// Sets default values
AMyPawn::AMyPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);
	CapsuleComp->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComp->SetSimulatePhysics(false);
	
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	SkeletalMeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	SkeletalMeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SkeletalMeshComp->SetSimulatePhysics(false);
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = true;
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(SkeletalMeshComp);
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	
	OverheadSlow = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadSlow"));
	OverheadSlow->SetupAttachment(SkeletalMeshComp);
	OverheadSlow->SetWidgetSpace(EWidgetSpace::Screen);
	
	OverheadReverse = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadReverse"));
	OverheadReverse->SetupAttachment(SkeletalMeshComp);
	OverheadReverse->SetWidgetSpace(EWidgetSpace::Screen);
	
	MaxHealth = 100.0f;
    Health = MaxHealth;
}


// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateOverheadHP();
	
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PerformGroundCheck();

	if (bIsGrounded)
	{
		if (Velocity.Z < 0.0f)
		{
			Velocity.Z = 0.0f;
		}
	}
	else
	{
		Velocity.Z -= GravityZ * DeltaTime;
	}

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector HorizontalVel = (Forward * MoveInput.Y + Right * MoveInput.X) * MoveSpeed;
	Velocity.X = HorizontalVel.X;
	Velocity.Y = HorizontalVel.Y;

	const FVector DeltaMove = Velocity * DeltaTime;
	FHitResult Hit;
	AddActorWorldOffset(DeltaMove, /*bSweep=*/true, &Hit);

	if (Hit.bBlockingHit)
	{
		Velocity = FVector::VectorPlaneProject(Velocity, Hit.ImpactNormal);
	}

	MoveInput = FVector2D::ZeroVector;
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
	if (!EnhancedInput || !PC)
	{
		return;
	}

	if (PC->IA_Move)
	{
		EnhancedInput->BindAction(PC->IA_Move, ETriggerEvent::Triggered, this, &AMyPawn::Move);
	}
	if (PC->IA_Look)
	{
		EnhancedInput->BindAction(PC->IA_Look, ETriggerEvent::Triggered, this, &AMyPawn::Look);
	}
	if (PC->IA_Jump)
	{
		EnhancedInput->BindAction(PC->IA_Jump, ETriggerEvent::Started, this, &AMyPawn::StartJump);
	}
}


void AMyPawn::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>() * InputMult;
}

void AMyPawn::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X * LookSensitivity);
	AddControllerPitchInput(LookAxis.Y * LookSensitivity);
}

void AMyPawn::StartJump(const FInputActionValue& Value)
{
	if (bIsGrounded)
	{
		Velocity.Z = JumpZVelocity;
		bIsGrounded = false;
	}
}


void AMyPawn::PerformGroundCheck()
{
	if (!CapsuleComp)
	{
		return;
	}

	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, CapsuleHalfHeight + GroundCheckDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bIsGrounded = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}


int32 AMyPawn::GetHealth() const
{
	return Health;
}

void AMyPawn::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
	UE_LOG(LogTemp, Log, TEXT("Health increased to: %f"), Health);
}

float AMyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();
	UE_LOG(LogTemp, Warning, TEXT("Health decreased to: %f"), Health);

	if (Health <= 0.0f)
	{
		OnDeath();
	}

	return ActualDamage;
}

void AMyPawn::OnDeath()
{
	AMyGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyGameState>() : nullptr;
	if (MyGameState)
	{
		MyGameState->OnGameOver();
	}
	UE_LOG(LogTemp, Error, TEXT("Character is Dead!"));
}

void AMyPawn::UpdateOverheadHP()
{
	if (!OverheadWidget) return;
	
	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;
	
	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}


void AMyPawn::GetSlow()
{
	if (!SlowTimer.IsValid())
	{
		MoveSpeed *= 0.5;
	}
	
	GetWorldTimerManager().SetTimer(
		SlowTimer,
		this,
		&AMyPawn::EndSlowTimer,
		5.0f,
		false);
	
	
	
}

void AMyPawn::EndSlowTimer()
{
	MoveSpeed *= 2;
	GetWorldTimerManager().ClearTimer(SlowTimer);
}

void AMyPawn::ReverseInput()
{
	if (!ReverseInputTimer.IsValid())
	{
		InputMult = -1;
	}
	
	GetWorldTimerManager().SetTimer(
		ReverseInputTimer,
		this,
		&AMyPawn::EndReverseInput,
		5.0f,
		false);
}

void AMyPawn::EndReverseInput()
{
	InputMult = 1;
	GetWorldTimerManager().ClearTimer(ReverseInputTimer);
}

float AMyPawn::GetSlowRemaining()
{
	return GetWorldTimerManager().GetTimerRemaining(SlowTimer);
}

float AMyPawn::GetReverseRemaining()
{
	return GetWorldTimerManager().GetTimerRemaining(ReverseInputTimer);
}

