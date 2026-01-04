#include "ConfirmationDialogWidget.h"
#include "GraphicsSettingsSubsystem.h"

void UConfirmationDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get the settings subsystem
	SettingsSubsystem = GetSettingsSubsystem();

	// Initialize state
	bIsCountdownActive = false;
	RemainingTime = 0.0f;
}

void UConfirmationDialogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update countdown if active
	if (bIsCountdownActive)
	{
		RemainingTime -= InDeltaTime;

		// Call Blueprint event to update UI
		OnCountdownTick(GetRemainingTimeInt());

		// Check if countdown expired
		if (RemainingTime <= 0.0f)
		{
			OnCountdownExpired();
		}
	}
}

// ========== Initialization ==========

void UConfirmationDialogWidget::StartCountdown(float TimeoutSeconds)
{
	TimeoutDuration = TimeoutSeconds;
	RemainingTime = TimeoutSeconds;
	bIsCountdownActive = true;

	// Initial UI update
	OnCountdownTick(GetRemainingTimeInt());
}

// ========== Button Handlers ==========

void UConfirmationDialogWidget::OnConfirmClicked()
{
	if (!SettingsSubsystem) return;

	// Stop countdown
	bIsCountdownActive = false;

	// Confirm settings change
	SettingsSubsystem->ConfirmSettingsChange();

	// Close dialog
	OnDialogClosed();
}

void UConfirmationDialogWidget::OnRevertClicked()
{
	if (!SettingsSubsystem) return;

	// Stop countdown
	bIsCountdownActive = false;

	// Revert settings change
	SettingsSubsystem->RevertSettingsChange();

	// Close dialog
	OnDialogClosed();
}

// ========== Subsystem Access ==========

UGraphicsSettingsSubsystem* UConfirmationDialogWidget::GetSettingsSubsystem()
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

// ========== Private Functions ==========

void UConfirmationDialogWidget::OnCountdownExpired()
{
	if (!SettingsSubsystem) return;

	// Stop countdown
	bIsCountdownActive = false;

	// Auto-revert settings
	SettingsSubsystem->RevertSettingsChange();

	// Close dialog
	OnDialogClosed();
}
