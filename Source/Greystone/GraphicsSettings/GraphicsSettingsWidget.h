#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphicsSettingsWidget.generated.h"

/**
 * Graphics Settings Widget
 * Base class for the main graphics settings menu UI.
 * Binds to GraphicsSettingsSubsystem for all data and functionality.
 */
UCLASS(Abstract)
class GREYSTONE_API UGraphicsSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called when the widget is constructed */
	virtual void NativeConstruct() override;

	/** Called every frame to update UI elements */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========== Button Handlers (Called from UMG) ==========

	/** Apply button clicked - applies settings and shows confirmation if needed */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void OnApplyClicked();

	/** Cancel button clicked - reverts all pending changes and closes menu */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void OnCancelClicked();

	/** Reset button clicked - resets all settings to defaults */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void OnResetToDefaultsClicked();

	// ========== Data Refresh ==========

	/** Refresh all UI elements from current settings */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	virtual void RefreshAllSettings();

	/** Populate resolution dropdown with supported resolutions */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	virtual void PopulateResolutionOptions();

	// ========== Events (Implement in Blueprint) ==========

	/** Event called when settings should be refreshed in UI */
	UFUNCTION(BlueprintImplementableEvent, Category = "Graphics Settings")
	void OnSettingsRefreshed();

	/** Event called when menu should close */
	UFUNCTION(BlueprintImplementableEvent, Category = "Graphics Settings")
	void OnMenuClosed();

	/** Event called when resolution options are populated */
	UFUNCTION(BlueprintImplementableEvent, Category = "Graphics Settings")
	void OnResolutionOptionsPopulated(const TArray<FIntPoint>& Resolutions);

protected:
	/** Cached reference to the graphics settings subsystem */
	UPROPERTY(BlueprintReadOnly, Category = "Graphics Settings")
	TObjectPtr<class UGraphicsSettingsSubsystem> SettingsSubsystem;

	/** Get the settings subsystem (lazy initialization) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	class UGraphicsSettingsSubsystem* GetSettingsSubsystem();
};
