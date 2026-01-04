// Copyright Epic Games, Inc. All Rights Reserved.

#include "GreystonePlayerController.h"
#include "GraphicsSettings/GraphicsSettingsWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"

AGreystonePlayerController::AGreystonePlayerController()
{
	// NOTE: Widget class paths are NOT set in constructor to avoid blocking editor startup.
	// These ConstructorHelpers were causing the editor to hang at 73% initialization.
	// Instead, set these widget classes in the Blueprint's Class Defaults:
	//   - GraphicsSettingsWidgetClass: /Game/UI/GraphicsSettings/WBP_GraphicsSettings
	//   - UIRootClass: /Game/UI/HUD/W_MyHUD
	//   - CircuitRoutingWidgetClass: /Game/Minigames/CircuitRouting/UI/WBP_CircuitMiniGame

	// Initialize input mode tracking
	bWasInGameOnlyMode = true;
	bWasShowingMouseCursor = false;
}

void AGreystonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Display HUD on start
	DisplayHUD();
}

void AGreystonePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// Bind F10 key to toggle settings menu
		InputComponent->BindKey(EKeys::F10, IE_Pressed, this, &AGreystonePlayerController::OnSettingsMenuToggle);
	}
}

// ========== UI Management ==========

void AGreystonePlayerController::DisplayHUD()
{
	// Create UIRoot if it doesn't exist
	if (!UIRoot && UIRootClass)
	{
		UIRoot = CreateWidget<UUserWidget>(this, UIRootClass);
		if (UIRoot)
		{
			UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Created UIRoot widget"));
		}
	}

	// Add to viewport if not already visible
	if (UIRoot && !UIRoot->IsInViewport())
	{
		UIRoot->AddToViewport(0); // Z-Order 0 (background HUD)
		UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Displayed HUD"));
	}
}

bool AGreystonePlayerController::IsModalUIOpen() const
{
	// Check if graphics settings is open
	if (GraphicsSettingsWidget && GraphicsSettingsWidget->IsInViewport())
	{
		return true;
	}

	// Check if circuit routing minigame is open
	if (CircuitRoutingWidget && CircuitRoutingWidget->IsInViewport())
	{
		return true;
	}

	return false;
}

void AGreystonePlayerController::OnSettingsMenuToggle()
{
	// If widget doesn't exist, create it
	if (!GraphicsSettingsWidget)
	{
		if (GraphicsSettingsWidgetClass)
		{
			GraphicsSettingsWidget = CreateWidget<UGraphicsSettingsWidget>(this, GraphicsSettingsWidgetClass);
			if (GraphicsSettingsWidget)
			{
				UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Created graphics settings widget"));
				OpenGraphicsSettingsMenu();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GreystonePlayerController: Failed to create graphics settings widget"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GreystonePlayerController: GraphicsSettingsWidgetClass is not set"));
		}
		return;
	}

	// Toggle widget visibility
	if (GraphicsSettingsWidget->IsInViewport())
	{
		CloseGraphicsSettingsMenu();
	}
	else
	{
		OpenGraphicsSettingsMenu();
	}
}

void AGreystonePlayerController::OpenGraphicsSettingsMenu()
{
	if (!GraphicsSettingsWidget)
	{
		return;
	}

	// Store current input mode before switching
	StoreCurrentInputMode();

	// Add to viewport
	GraphicsSettingsWidget->AddToViewport(10); // Z-Order 10

	// Set input mode to UI only
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GraphicsSettingsWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// Show mouse cursor
	SetShowMouseCursor(true);

	UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Opened graphics settings menu"));
}

void AGreystonePlayerController::CloseGraphicsSettingsMenu()
{
	if (!GraphicsSettingsWidget)
	{
		return;
	}

	// Remove from viewport
	GraphicsSettingsWidget->RemoveFromParent();

	// Restore previous input mode
	RestorePreviousInputMode();


	UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Closed graphics settings menu"));
}

// ========== Input Mode Management ==========

void AGreystonePlayerController::StoreCurrentInputMode()
{
	// Check if we're currently in game-only mode by checking if other modal UIs are open
	bWasInGameOnlyMode = !IsModalUIOpen();
	bWasShowingMouseCursor = bShowMouseCursor;

	UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Stored input mode - GameOnly: %s, ShowCursor: %s"),
		bWasInGameOnlyMode ? TEXT("true") : TEXT("false"),
		bWasShowingMouseCursor ? TEXT("true") : TEXT("false"));
}

void AGreystonePlayerController::RestorePreviousInputMode()
{
	// If another modal UI is still open, keep UI input mode
	if (IsModalUIOpen())
	{
		UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Other modal UI still open, keeping UI input mode"));
		return;
	}

	// Restore input mode based on what it was before
	if (bWasInGameOnlyMode)
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Restored Game Only input mode"));
	}
	else
	{
		// Was in UI mode, keep it that way (other UI might still be active)
		UE_LOG(LogTemp, Log, TEXT("GreystonePlayerController: Keeping UI input mode"));
	}

	// Restore cursor visibility
	SetShowMouseCursor(bWasShowingMouseCursor);
}
