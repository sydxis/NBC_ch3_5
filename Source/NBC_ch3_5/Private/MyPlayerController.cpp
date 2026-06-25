// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MyGameState.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

AMyPlayerController::AMyPlayerController()
{
	InputMappingContext = nullptr;
	IA_Move = nullptr;
	IA_Look = nullptr;
	IA_Jump = nullptr;
	HUDWidgetInstance = nullptr;
	MainMenuWidgetClass = nullptr;
	MainMenuWidgetInstance = nullptr;
	GameOverWidgetClass = nullptr;
	GameOverWidgetInstance = nullptr;

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
	
	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
	{
		ShowMainMenu(false);
	}
}

UUserWidget* AMyPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}


void AMyPlayerController::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetClass)
	{		
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
		
		if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			if (bIsRestart)
			{
				ButtonText->SetText(FText::FromString(TEXT("Restart")));
			}
			else
			{
				ButtonText->SetText(FText::FromString(TEXT("Start")));
			}
		}
		
		if (bIsRestart)
		{
			UFunction* PlayAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
			if (PlayAnimFunc)
			{
				MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
			}
			
			if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName("TotalScoreText")))
			{
				if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
				{
					TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Total Score: %d"), MyGameInstance->TotalScore)));
				}
			}
		}
		
		
	}
}

void AMyPlayerController::ShowGameOver(bool bCleared)
{
	if (!GameOverWidgetClass)
	{
		ShowMainMenu(true);
		return;
	}

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (GameOverWidgetInstance)
	{
		GameOverWidgetInstance->RemoveFromParent();
		GameOverWidgetInstance = nullptr;
	}

	GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);
	if (!GameOverWidgetInstance)
	{
		return;
	}

	GameOverWidgetInstance->AddToViewport();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (UTextBlock* GameOverText = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName(TEXT("GameOverText"))))
	{
		GameOverText->SetText(FText::FromString(bCleared ? TEXT("Game Clear!") : TEXT("Game Over")));
	}

	if (UTextBlock* ButtonText = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
	{
		ButtonText->SetText(FText::FromString(TEXT("Restart")));
	}

	if (UTextBlock* TotalScoreText = Cast<UTextBlock>(GameOverWidgetInstance->GetWidgetFromName(TEXT("TotalScoreText"))))
	{
		if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Total Score: %d"), MyGameInstance->TotalScore)));
		}
	}

	if (UFunction* PlayAnimFunc = GameOverWidgetInstance->FindFunction(FName("PlayGameOverAnim")))
	{
		GameOverWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
	}
}

void AMyPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			
			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
			
			AMyGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyGameState>() : nullptr;
			if (MyGameState)
			{
				MyGameState->UpdateHUD();
			}
		}
	}
}

void AMyPlayerController::StartGame()
{
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		MyGameInstance->CurrentLevelIndex = 0;
		MyGameInstance->TotalScore = 0;
	}
	
	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}