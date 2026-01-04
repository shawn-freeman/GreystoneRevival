#include "GraphicsSettingsSubsystem.h"
#include "ConfirmationDialogWidget.h"
#include "Scalability.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

void UGraphicsSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache the game user settings
	GameUserSettings = GEngine->GetGameUserSettings();
	if (!GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("GraphicsSettingsSubsystem: Failed to get GameUserSettings!"));
	}

	// Initialize state
	bIsWaitingForConfirmation = false;
	bHasPendingChanges = false;

	// Auto-load confirmation dialog class if not set
	if (!ConfirmationDialogClass)
	{
		FSoftClassPath DialogClassPath(TEXT("/Game/UI/GraphicsSettings/WBP_ConfirmationDialogWidget.WBP_ConfirmationDialogWidget_C"));
		ConfirmationDialogClass = DialogClassPath.TryLoadClass<UConfirmationDialogWidget>();
		if (ConfirmationDialogClass)
		{
			UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsSubsystem: Auto-loaded ConfirmationDialogClass"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GraphicsSettingsSubsystem: Failed to auto-load ConfirmationDialogClass from /Game/UI/GraphicsSettings/WBP_ConfirmationDialogWidget"));
		}
	}

	// Store initial settings
	StorePreviousSettings();
}

void UGraphicsSettingsSubsystem::Deinitialize()
{
	// Clean up confirmation dialog if active
	if (ConfirmationDialogWidget && ConfirmationDialogWidget->IsInViewport())
	{
		ConfirmationDialogWidget->RemoveFromParent();
	}

	Super::Deinitialize();
}

// ========== Resolution Management ==========

TArray<FIntPoint> UGraphicsSettingsSubsystem::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	FScreenResolutionArray ResolutionArray;
	if (RHIGetAvailableResolutions(ResolutionArray, true))
	{
		for (const FScreenResolutionRHI& Resolution : ResolutionArray)
		{
			FIntPoint ResolutionPoint(Resolution.Width, Resolution.Height);
			
			// Filter out very low resolutions
			if (Resolution.Width >= 800 && Resolution.Height >= 600)
			{
				Resolutions.AddUnique(ResolutionPoint);
			}
		}
	}

	// Sort by width, then height
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) {
		if (A.X != B.X) return A.X < B.X;
		return A.Y < B.Y;
	});

	return Resolutions;
}

FIntPoint UGraphicsSettingsSubsystem::GetCurrentResolution() const
{
	if (!GameUserSettings) return FIntPoint(1920, 1080);
	return GameUserSettings->GetScreenResolution();
}

void UGraphicsSettingsSubsystem::SetResolution(FIntPoint NewResolution, bool bApplyNow)
{
	if (!GameUserSettings) return;

	GameUserSettings->SetScreenResolution(NewResolution);
	bHasPendingChanges = true;

	if (bApplyNow)
	{
		ApplySettings(false);
	}
}

FIntPoint UGraphicsSettingsSubsystem::ParseResolutionString(const FString& ResolutionString)
{
	FString LeftPart, RightPart;

	// Split by 'x' delimiter
	if (ResolutionString.Split(TEXT("x"), &LeftPart, &RightPart, ESearchCase::IgnoreCase))
	{
		// Convert strings to integers
		int32 Width = FCString::Atoi(*LeftPart);
		int32 Height = FCString::Atoi(*RightPart);

		// Validate reasonable values
		if (Width > 0 && Height > 0 && Width <= 7680 && Height <= 4320)
		{
			return FIntPoint(Width, Height);
		}
	}

	// Return default resolution if parsing failed
	UE_LOG(LogTemp, Warning, TEXT("Failed to parse resolution string: %s. Using default 1920x1080"), *ResolutionString);
	return FIntPoint(1920, 1080);
}

FString UGraphicsSettingsSubsystem::ResolutionToString(FIntPoint Resolution)
{
	return FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
}

// ========== Fullscreen Mode ==========

void UGraphicsSettingsSubsystem::SetFullscreenMode(TEnumAsByte<EWindowMode::Type> Mode, bool bApplyNow)
{
	if (!GameUserSettings) return;

	GameUserSettings->SetFullscreenMode(Mode);
	bHasPendingChanges = true;

	if (bApplyNow)
	{
		ApplySettings(false);
	}
}

TEnumAsByte<EWindowMode::Type> UGraphicsSettingsSubsystem::GetFullscreenMode() const
{
	if (!GameUserSettings) return EWindowMode::Windowed;
	return GameUserSettings->GetFullscreenMode();
}

TEnumAsByte<EWindowMode::Type> UGraphicsSettingsSubsystem::StringToWindowMode(const FString& ModeString)
{
	// Case-insensitive comparison
	FString ModeLower = ModeString.ToLower();

	if (ModeLower.Contains(TEXT("fullscreen")) || ModeLower.Contains(TEXT("exclusive")))
	{
		return EWindowMode::Fullscreen;
	}
	else if (ModeLower.Contains(TEXT("borderless")) || ModeLower.Contains(TEXT("windowed fullscreen")))
	{
		return EWindowMode::WindowedFullscreen;
	}
	else if (ModeLower.Contains(TEXT("windowed")))
	{
		return EWindowMode::Windowed;
	}

	// Default to windowed
	UE_LOG(LogTemp, Warning, TEXT("Unknown window mode string: %s. Using Windowed."), *ModeString);
	return EWindowMode::Windowed;
}

FString UGraphicsSettingsSubsystem::WindowModeToString(TEnumAsByte<EWindowMode::Type> Mode)
{
	switch (Mode)
	{
	case EWindowMode::Fullscreen:
		return TEXT("Fullscreen");
	case EWindowMode::WindowedFullscreen:
		return TEXT("Borderless Windowed");
	case EWindowMode::Windowed:
		return TEXT("Windowed");
	default:
		return TEXT("Windowed");
	}
}

// ========== Quality Presets ==========

void UGraphicsSettingsSubsystem::SetOverallQualityPreset(int32 Preset)
{
	if (!GameUserSettings) return;

	// Clamp preset to valid range (0-3)
	Preset = FMath::Clamp(Preset, 0, 3);

	// Get current quality levels from Scalability system and set from preset
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.SetFromSingleQualityLevel(Preset);
	Scalability::SetQualityLevels(QualityLevels);
	
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetOverallQualityPreset() const
{
	if (!GameUserSettings) return -1;

	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();

	// Check if all quality levels are the same (preset) or mixed (custom)
	int32 BaseQuality = QualityLevels.ResolutionQuality;
	if (QualityLevels.ViewDistanceQuality == BaseQuality &&
		QualityLevels.AntiAliasingQuality == BaseQuality &&
		QualityLevels.ShadowQuality == BaseQuality &&
		QualityLevels.GlobalIlluminationQuality == BaseQuality &&
		QualityLevels.ReflectionQuality == BaseQuality &&
		QualityLevels.PostProcessQuality == BaseQuality &&
		QualityLevels.TextureQuality == BaseQuality &&
		QualityLevels.EffectsQuality == BaseQuality &&
		QualityLevels.FoliageQuality == BaseQuality &&
		QualityLevels.ShadingQuality == BaseQuality)
	{
		return BaseQuality; // All same = preset
	}

	return -1; // Mixed = custom
}

// ========== Individual Scalability Settings ==========

void UGraphicsSettingsSubsystem::SetViewDistanceQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.ViewDistanceQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetViewDistanceQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().ViewDistanceQuality;
}

void UGraphicsSettingsSubsystem::SetShadowQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.ShadowQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetShadowQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().ShadowQuality;
}

void UGraphicsSettingsSubsystem::SetAntiAliasingQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.AntiAliasingQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetAntiAliasingQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().AntiAliasingQuality;
}

void UGraphicsSettingsSubsystem::SetPostProcessQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.PostProcessQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetPostProcessQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().PostProcessQuality;
}

void UGraphicsSettingsSubsystem::SetTextureQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.TextureQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetTextureQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().TextureQuality;
}

void UGraphicsSettingsSubsystem::SetEffectsQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.EffectsQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetEffectsQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().EffectsQuality;
}

void UGraphicsSettingsSubsystem::SetFoliageQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.FoliageQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetFoliageQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().FoliageQuality;
}

void UGraphicsSettingsSubsystem::SetShadingQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.ShadingQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetShadingQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().ShadingQuality;
}

void UGraphicsSettingsSubsystem::SetGlobalIlluminationQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.GlobalIlluminationQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetGlobalIlluminationQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().GlobalIlluminationQuality;
}

void UGraphicsSettingsSubsystem::SetReflectionQuality(int32 Quality)
{
	if (!GameUserSettings) return;
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.ReflectionQuality = FMath::Clamp(Quality, 0, 3);
	Scalability::SetQualityLevels(QualityLevels);
	bHasPendingChanges = true;
}

int32 UGraphicsSettingsSubsystem::GetReflectionQuality() const
{
	if (!GameUserSettings) return 2;
	return Scalability::GetQualityLevels().ReflectionQuality;
}

// ========== Resolution Scale ==========

void UGraphicsSettingsSubsystem::SetResolutionScale(float Percentage)
{
	if (!GameUserSettings) return;
	
	// Clamp to 50-100% range
	Percentage = FMath::Clamp(Percentage, 0.5f, 1.0f);
	GameUserSettings->SetResolutionScaleNormalized(Percentage);
	bHasPendingChanges = true;
}

float UGraphicsSettingsSubsystem::GetResolutionScale() const
{
	if (!GameUserSettings) return 1.0f;
	return GameUserSettings->GetResolutionScaleNormalized();
}

// ========== VSync and Frame Rate ==========

void UGraphicsSettingsSubsystem::SetVSyncEnabled(bool bEnabled)
{
	if (!GameUserSettings) return;
	GameUserSettings->SetVSyncEnabled(bEnabled);
	bHasPendingChanges = true;
}

bool UGraphicsSettingsSubsystem::GetVSyncEnabled() const
{
	if (!GameUserSettings) return false;
	return GameUserSettings->IsVSyncEnabled();
}

void UGraphicsSettingsSubsystem::SetFrameRateLimit(float Limit)
{
	if (!GameUserSettings) return;
	GameUserSettings->SetFrameRateLimit(Limit);
	bHasPendingChanges = true;
}

float UGraphicsSettingsSubsystem::GetFrameRateLimit() const
{
	if (!GameUserSettings) return 0.0f;
	return GameUserSettings->GetFrameRateLimit();
}

float UGraphicsSettingsSubsystem::ParseFrameRateString(const FString& FrameRateString)
{
	// Check for unlimited keywords
	FString LowerString = FrameRateString.ToLower();
	if (LowerString.Contains(TEXT("unlimited")) ||
		LowerString.Contains(TEXT("uncapped")) ||
		LowerString.Contains(TEXT("none")) ||
		LowerString == TEXT("0"))
	{
		return 0.0f;
	}

	// Parse numeric value (handles "60", "60 FPS", "120fps", etc.)
	float Value = FCString::Atof(*FrameRateString);

	// Validate reasonable range
	if (Value > 0.0f && Value <= 500.0f)
	{
		return Value;
	}

	// Default to unlimited if parsing failed
	UE_LOG(LogTemp, Warning, TEXT("Failed to parse frame rate string: %s. Using Unlimited (0)."), *FrameRateString);
	return 0.0f;
}

FString UGraphicsSettingsSubsystem::FrameRateToString(float FrameRate)
{
	if (FrameRate <= 0.0f)
	{
		return TEXT("Unlimited");
	}

	// Round to nearest integer and format
	int32 RoundedRate = FMath::RoundToInt(FrameRate);
	return FString::Printf(TEXT("%d FPS"), RoundedRate);
}

// ========== Apply and Save ==========

void UGraphicsSettingsSubsystem::ApplySettings(bool bSaveAfterApply)
{
	if (!GameUserSettings) return;

	// Check if display settings changed (requires confirmation)
	bool bDisplayChanged = HasDisplaySettingsChanged();

	if (bDisplayChanged)
	{
		// Store previous settings before applying
		StorePreviousSettings();
	}

	// Apply settings immediately
	GameUserSettings->ApplySettings(false);

	if (bDisplayChanged)
	{
		// Show confirmation dialog for display changes
		ShowConfirmationDialog(10.0f);
	}
	else if (bSaveAfterApply)
	{
		// No confirmation needed, save directly
		SaveSettings();
	}

	bHasPendingChanges = false;
}

void UGraphicsSettingsSubsystem::SaveSettings()
{
	if (!GameUserSettings) return;
	GameUserSettings->SaveSettings();
	bHasPendingChanges = false;
}

void UGraphicsSettingsSubsystem::RevertChanges()
{
	if (!GameUserSettings) return;
	GameUserSettings->LoadSettings(true);
	bHasPendingChanges = false;
}

bool UGraphicsSettingsSubsystem::HasPendingChanges() const
{
	return bHasPendingChanges;
}

void UGraphicsSettingsSubsystem::ResetToDefaults()
{
	if (!GameUserSettings) return;
	GameUserSettings->SetToDefaults();
	bHasPendingChanges = true;
}

// ========== Confirmation Dialog ==========

void UGraphicsSettingsSubsystem::ShowConfirmationDialog(float TimeoutSeconds)
{
	if (!ConfirmationDialogClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GraphicsSettingsSubsystem: No ConfirmationDialogClass set!"));
		ConfirmSettingsChange(); // Auto-confirm if no dialog available
		return;
	}

	// Create dialog widget if not already exists
	if (!ConfirmationDialogWidget)
	{
		UWorld* World = GetGameInstance()->GetWorld();
		if (World)
		{
			ConfirmationDialogWidget = CreateWidget<UConfirmationDialogWidget>(World, ConfirmationDialogClass);
		}
	}

	if (ConfirmationDialogWidget)
	{
		// Add to viewport and start countdown
		ConfirmationDialogWidget->AddToViewport(100); // High Z-order for modal behavior
		ConfirmationDialogWidget->StartCountdown(TimeoutSeconds);
		bIsWaitingForConfirmation = true;
	}
}

void UGraphicsSettingsSubsystem::ConfirmSettingsChange()
{
	if (!GameUserSettings) return;

	// Confirm the video mode and save settings
	GameUserSettings->ConfirmVideoMode();
	SaveSettings();

	bIsWaitingForConfirmation = false;

	// Close dialog
	if (ConfirmationDialogWidget && ConfirmationDialogWidget->IsInViewport())
	{
		ConfirmationDialogWidget->RemoveFromParent();
	}
}

void UGraphicsSettingsSubsystem::RevertSettingsChange()
{
	if (!GameUserSettings) return;

	// Revert to previous resolution and window mode
	GameUserSettings->SetScreenResolution(PreviousResolution);
	GameUserSettings->SetFullscreenMode(PreviousWindowMode);
	GameUserSettings->ApplySettings(false);

	bIsWaitingForConfirmation = false;

	// Close dialog
	if (ConfirmationDialogWidget && ConfirmationDialogWidget->IsInViewport())
	{
		ConfirmationDialogWidget->RemoveFromParent();
	}
}

// ========== Helper Functions ==========

bool UGraphicsSettingsSubsystem::HasDisplaySettingsChanged() const
{
	if (!GameUserSettings) return false;

	FIntPoint CurrentRes = GameUserSettings->GetScreenResolution();
	EWindowMode::Type CurrentMode = GameUserSettings->GetFullscreenMode();

	return (CurrentRes != PreviousResolution) || (CurrentMode != PreviousWindowMode);
}

void UGraphicsSettingsSubsystem::StorePreviousSettings()
{
	if (!GameUserSettings) return;

	PreviousResolution = GameUserSettings->GetScreenResolution();
	PreviousWindowMode = GameUserSettings->GetFullscreenMode();
}
