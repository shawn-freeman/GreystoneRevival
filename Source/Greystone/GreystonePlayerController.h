// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GraphicsSettings/GraphicsSettingsWidget.h"
#include "GreystonePlayerController.generated.h"

class UUserWidget;

/**
 * Extended PlayerController with graphics settings menu integration.
 * Handles F10 key input to toggle settings menu.
 */
UCLASS()
class GREYSTONE_API AGreystonePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGreystonePlayerController();

	virtual void BeginPlay() override;

protected:
	/** Called to bind functionality to input */
	virtual void SetupInputComponent() override;

	// ========== Graphics Settings Widget ==========

	/** Reference to the graphics settings widget instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Graphics")
	TObjectPtr<UGraphicsSettingsWidget> GraphicsSettingsWidget;

	/** Class to use for the graphics settings widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Graphics")
	TSubclassOf<UGraphicsSettingsWidget> GraphicsSettingsWidgetClass;

	// ========== Base UI Widgets ==========

	/** Reference to the main HUD widget (always visible) */
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UUserWidget> UIRoot;

	/** Class to use for the main HUD */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> UIRootClass;

	/** Reference to the circuit routing minigame widget */
	UPROPERTY(BlueprintReadWrite, Category = "UI|Minigame")
	TObjectPtr<UUserWidget> CircuitRoutingWidget;

	/** Class to use for circuit routing minigame */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Minigame")
	TSubclassOf<UUserWidget> CircuitRoutingWidgetClass;

	// ========== UI Management Functions ==========

	/** Display the main HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void DisplayHUD();

	/** Check if any modal UI is currently open (graphics settings, minigames, etc) */
	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsModalUIOpen() const;

	/** Closes the graphics settings menu */
	UFUNCTION(BlueprintCallable, Category = "UI|Graphics")
	void CloseGraphicsSettingsMenu();

private:
	/** Handles F10 key press to toggle settings menu */
	UFUNCTION()
	void OnSettingsMenuToggle();

	/** Opens the graphics settings menu */
	UFUNCTION(BlueprintCallable, Category = "UI|Graphics", meta = (BlueprintProtected = "true"))
	void OpenGraphicsSettingsMenu();

	/** Store the current input mode before switching to UI */
	void StoreCurrentInputMode();

	/** Restore the previously stored input mode */
	void RestorePreviousInputMode();

	/** Stored input mode (to restore after closing settings) */
	bool bWasInGameOnlyMode;
	bool bWasShowingMouseCursor;
};
