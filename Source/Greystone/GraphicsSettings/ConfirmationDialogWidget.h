#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfirmationDialogWidget.generated.h"

/**
 * Confirmation Dialog Widget
 * 10-second countdown dialog for confirming display changes (resolution/fullscreen mode).
 * Auto-reverts if not confirmed within the timeout period.
 */
UCLASS(Abstract)
class GREYSTONE_API UConfirmationDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called when the widget is constructed */
	virtual void NativeConstruct() override;

	/** Called every frame to update countdown */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========== Initialization ==========

	/** Start the confirmation countdown */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void StartCountdown(float TimeoutSeconds = 10.0f);

	// ========== Button Handlers ==========

	/** Confirm button clicked - accept changes and close dialog */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void OnConfirmClicked();

	/** Revert button clicked - revert changes and close dialog */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void OnRevertClicked();

	// ========== Countdown Management ==========

	/** Get remaining time in seconds */
	UFUNCTION(BlueprintPure, Category = "Confirmation Dialog")
	float GetRemainingTime() const { return RemainingTime; }

	/** Get remaining time as integer seconds (for display) */
	UFUNCTION(BlueprintPure, Category = "Confirmation Dialog")
	int32 GetRemainingTimeInt() const { return FMath::CeilToInt(RemainingTime); }

	// ========== Events (Implement in Blueprint) ==========

	/** Event called when countdown timer updates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Confirmation Dialog")
	void OnCountdownTick(int32 SecondsRemaining);

	/** Event called when dialog should close */
	UFUNCTION(BlueprintImplementableEvent, Category = "Confirmation Dialog")
	void OnDialogClosed();

protected:
	/** Cached reference to the graphics settings subsystem */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation Dialog")
	TObjectPtr<class UGraphicsSettingsSubsystem> SettingsSubsystem;

	/** Remaining time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation Dialog")
	float RemainingTime;

	/** Total timeout duration */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation Dialog")
	float TimeoutDuration;

	/** Is countdown active? */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation Dialog")
	bool bIsCountdownActive;

	/** Get the settings subsystem (lazy initialization) */
	UFUNCTION(BlueprintPure, Category = "Confirmation Dialog")
	class UGraphicsSettingsSubsystem* GetSettingsSubsystem();

private:
	/** Auto-revert when countdown reaches zero */
	void OnCountdownExpired();
};
