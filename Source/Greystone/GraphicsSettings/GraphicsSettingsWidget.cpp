#include "GraphicsSettingsWidget.h"
#include "GraphicsSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UGraphicsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get the settings subsystem
	SettingsSubsystem = GetSettingsSubsystem();

	if (SettingsSubsystem)
	{
		// Populate UI with current settings
		RefreshAllSettings();
		PopulateResolutionOptions();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GraphicsSettingsWidget: Failed to get SettingsSubsystem!"));
	}
}

void UGraphicsSettingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Optional: Update UI elements that need real-time refresh
}

// ========== Button Handlers ==========

void UGraphicsSettingsWidget::OnApplyClicked()
{
	if (!SettingsSubsystem) return;

	// Apply settings (will show confirmation dialog if display settings changed)
	SettingsSubsystem->ApplySettings(true);

	// If no confirmation needed, close the menu
	if (!SettingsSubsystem->IsConfirmationDialogActive())
	{
		OnMenuClosed();
	}
}

void UGraphicsSettingsWidget::OnCancelClicked()
{
	if (!SettingsSubsystem) return;

	// Revert any pending changes
	SettingsSubsystem->RevertChanges();

	// Close the menu
	OnMenuClosed();
}

void UGraphicsSettingsWidget::OnResetToDefaultsClicked()
{
	if (!SettingsSubsystem) return;

	// Reset all settings to defaults
	SettingsSubsystem->ResetToDefaults();

	// Refresh UI to show new values
	RefreshAllSettings();
}

// ========== Data Refresh ==========

void UGraphicsSettingsWidget::RefreshAllSettings()
{
	if (!SettingsSubsystem) return;

	// Call Blueprint event to update UI elements
	OnSettingsRefreshed();
}

void UGraphicsSettingsWidget::PopulateResolutionOptions()
{
	if (!SettingsSubsystem) return;

	// Get supported resolutions
	TArray<FIntPoint> Resolutions = SettingsSubsystem->GetSupportedResolutions();

	// Call Blueprint event to populate UI dropdown
	OnResolutionOptionsPopulated(Resolutions);
}

// ========== Subsystem Access ==========

UGraphicsSettingsSubsystem* UGraphicsSettingsWidget::GetSettingsSubsystem()
{
	if (SettingsSubsystem)
	{
		return SettingsSubsystem;
	}

	// Get game instance and retrieve subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		SettingsSubsystem = GameInstance->GetSubsystem<UGraphicsSettingsSubsystem>();
	}

	return SettingsSubsystem;
}
