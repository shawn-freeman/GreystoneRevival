#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/GameUserSettings.h"
#include "GraphicsSettingsSubsystem.generated.h"

/**
 * Graphics Settings Subsystem
 * Centralized management for all graphics settings with Blueprint exposure.
 * Handles resolution, quality, fullscreen mode, and confirmation dialogs.
 */
UCLASS()
class GREYSTONE_API UGraphicsSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Resolution Management ==========

	/** Get all supported screen resolutions for the current display */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	TArray<FIntPoint> GetSupportedResolutions() const;

	/** Get the current screen resolution */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	FIntPoint GetCurrentResolution() const;

	/** Set screen resolution (requires Apply to take effect) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetResolution(FIntPoint NewResolution, bool bApplyNow = false);

	/** Parse a resolution string (e.g. "1920x1080") into an IntPoint */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static FIntPoint ParseResolutionString(const FString& ResolutionString);

	/** Convert a resolution IntPoint to a string (e.g. FIntPoint(1920,1080) -> "1920x1080") */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static FString ResolutionToString(FIntPoint Resolution);

	// ========== Fullscreen Mode ==========

	/** Set fullscreen mode (Fullscreen, Windowed, WindowedFullscreen) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetFullscreenMode(TEnumAsByte<EWindowMode::Type> Mode, bool bApplyNow = false);

	/** Get current fullscreen mode */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	TEnumAsByte<EWindowMode::Type> GetFullscreenMode() const;

	/** Convert a window mode string to enum (e.g. "Fullscreen" -> EWindowMode::Fullscreen) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static TEnumAsByte<EWindowMode::Type> StringToWindowMode(const FString& ModeString);

	/** Convert a window mode enum to string (e.g. EWindowMode::Fullscreen -> "Fullscreen") */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static FString WindowModeToString(TEnumAsByte<EWindowMode::Type> Mode);

	// ========== Quality Presets ==========

	/** Set overall quality preset (0=Low, 1=Medium, 2=High, 3=Epic) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetOverallQualityPreset(int32 Preset);

	/** Get overall quality preset (returns -1 if custom/mixed settings) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetOverallQualityPreset() const;

	// ========== Individual Scalability Settings ==========

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetViewDistanceQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetViewDistanceQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetShadowQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetShadowQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetAntiAliasingQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetAntiAliasingQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetPostProcessQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetPostProcessQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetTextureQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetTextureQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetEffectsQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetEffectsQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetFoliageQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetFoliageQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetShadingQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetShadingQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetGlobalIlluminationQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetGlobalIlluminationQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetReflectionQuality(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	int32 GetReflectionQuality() const;

	// ========== Resolution Scale ==========

	/** Set resolution scale percentage (0.5 = 50%, 1.0 = 100%) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetResolutionScale(float Percentage);

	/** Get resolution scale percentage (0.5 = 50%, 1.0 = 100%) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	float GetResolutionScale() const;

	// ========== VSync and Frame Rate ==========

	/** Enable or disable VSync */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetVSyncEnabled(bool bEnabled);

	/** Check if VSync is enabled */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	bool GetVSyncEnabled() const;

	/** Set frame rate limit (0 = unlimited) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SetFrameRateLimit(float Limit);

	/** Get current frame rate limit (0 = unlimited) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	float GetFrameRateLimit() const;

	/** Parse a framerate string (e.g. "60", "120", "Unlimited") to float (0 = unlimited) */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static float ParseFrameRateString(const FString& FrameRateString);

	/** Convert a framerate float to string (e.g. 60.0 -> "60 FPS", 0.0 -> "Unlimited") */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	static FString FrameRateToString(float FrameRate);

	// ========== Apply and Save ==========

	/** Apply all pending settings changes (optionally save to disk) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void ApplySettings(bool bSaveAfterApply = true);

	/** Save current settings to GameUserSettings.ini */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void SaveSettings();

	/** Revert all pending changes to last saved state */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void RevertChanges();

	/** Check if there are unsaved/pending changes */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	bool HasPendingChanges() const;

	/** Reset all settings to engine defaults */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void ResetToDefaults();

	// ========== Confirmation Dialog ==========

	/** Show confirmation dialog with countdown (for resolution/fullscreen changes) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void ShowConfirmationDialog(float TimeoutSeconds = 10.0f);

	/** Confirm and save the current settings (called from confirmation dialog) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void ConfirmSettingsChange();

	/** Revert to previous settings (called from confirmation dialog or timeout) */
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void RevertSettingsChange();

	/** Check if confirmation dialog is active */
	UFUNCTION(BlueprintPure, Category = "Graphics Settings")
	bool IsConfirmationDialogActive() const { return bIsWaitingForConfirmation; }

	// ========== Widget Management ==========

	/** Reference to the active confirmation dialog widget */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics Settings")
	TObjectPtr<class UConfirmationDialogWidget> ConfirmationDialogWidget;

	/** Widget class to spawn for confirmation dialog */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Graphics Settings")
	TSubclassOf<class UConfirmationDialogWidget> ConfirmationDialogClass;

private:
	/** Cached game user settings */
	UPROPERTY()
	TObjectPtr<UGameUserSettings> GameUserSettings;

	/** Previous resolution (for revert) */
	FIntPoint PreviousResolution;

	/** Previous window mode (for revert) */
	TEnumAsByte<EWindowMode::Type> PreviousWindowMode;

	/** Are we waiting for user confirmation? */
	bool bIsWaitingForConfirmation;

	/** Does the user have pending changes? */
	bool bHasPendingChanges;

	/** Timer handle for auto-revert countdown */
	FTimerHandle ConfirmationTimerHandle;

	/** Helper: Check if display settings changed (resolution or fullscreen mode) */
	bool HasDisplaySettingsChanged() const;

	/** Helper: Store current settings as previous (before applying changes) */
	void StorePreviousSettings();
};
