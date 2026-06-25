// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MyGameState.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 컨트롤러 회전: 기존 폰과 동일하게 카메라 Yaw에 몸이 따라가도록 유지
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 캡슐(루트)은 ACharacter가 생성한 것을 사용
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	// 스켈레탈 메시(ACharacter 기본 제공)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// 이동 관련 값을 CharacterMovementComponent로 위임 (중력/바닥체크/점프 자동 처리)
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = false;
	MoveComp->MaxWalkSpeed = MoveSpeed;
	MoveComp->JumpZVelocity = JumpZVelocity;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	OverheadSlow = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadSlow"));
	OverheadSlow->SetupAttachment(GetMesh());
	OverheadSlow->SetWidgetSpace(EWidgetSpace::Screen);

	OverheadReverse = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadReverse"));
	OverheadReverse->SetupAttachment(GetMesh());
	OverheadReverse->SetWidgetSpace(EWidgetSpace::Screen);

	MaxHealth = 100.0f;
    Health = MaxHealth;
}


// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 에디터/블루프린트에서 수정한 값을 무브먼트 컴포넌트에 반영
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
		MoveComp->JumpZVelocity = JumpZVelocity;
	}

	UpdateOverheadHP();

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
		EnhancedInput->BindAction(PC->IA_Move, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
	}
	if (PC->IA_Look)
	{
		EnhancedInput->BindAction(PC->IA_Look, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
	}
	if (PC->IA_Jump)
	{
		EnhancedInput->BindAction(PC->IA_Jump, ETriggerEvent::Started, this, &AMyCharacter::StartJump);
	}
}


void AMyCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>() * InputMult;

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MoveValue.Y);
	AddMovementInput(Right, MoveValue.X);
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X * LookSensitivity);
	AddControllerPitchInput(LookAxis.Y * LookSensitivity);
}

void AMyCharacter::StartJump(const FInputActionValue& Value)
{
	// ACharacter::Jump() 가 JumpZVelocity와 접지 상태를 알아서 처리
	Jump();
}


int32 AMyCharacter::GetHealth() const
{
	return Health;
}

void AMyCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
	UE_LOG(LogTemp, Log, TEXT("Health increased to: %f"), Health);
}

float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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

void AMyCharacter::OnDeath()
{
	AMyGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyGameState>() : nullptr;
	if (MyGameState)
	{
		MyGameState->OnGameOver();
	}
	UE_LOG(LogTemp, Error, TEXT("Character is Dead!"));
}

void AMyCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}


void AMyCharacter::GetSlow()
{
	if (!SlowTimer.IsValid())
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = MoveSpeed * 0.5f;
		}
	}

	GetWorldTimerManager().SetTimer(
		SlowTimer,
		this,
		&AMyCharacter::EndSlowTimer,
		5.0f,
		false);
}

void AMyCharacter::EndSlowTimer()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
	}
	GetWorldTimerManager().ClearTimer(SlowTimer);
}

void AMyCharacter::ReverseInput()
{
	if (!ReverseInputTimer.IsValid())
	{
		InputMult = -1;
	}

	GetWorldTimerManager().SetTimer(
		ReverseInputTimer,
		this,
		&AMyCharacter::EndReverseInput,
		5.0f,
		false);
}

void AMyCharacter::EndReverseInput()
{
	InputMult = 1;
	GetWorldTimerManager().ClearTimer(ReverseInputTimer);
}

float AMyCharacter::GetSlowRemaining()
{
	return GetWorldTimerManager().GetTimerRemaining(SlowTimer);
}

float AMyCharacter::GetReverseRemaining()
{
	return GetWorldTimerManager().GetTimerRemaining(ReverseInputTimer);
}
