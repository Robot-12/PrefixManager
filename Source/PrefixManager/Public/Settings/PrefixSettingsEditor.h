// Author: Robot 12

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PrefixSettingsEditor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnValidationStateChanged, bool);

UCLASS(config=EditorSettings, meta=(DisplayName="Prefix Settings"))
class PREFIXMANAGER_API UPrefixSettingsEditor : public UDeveloperSettings
{
	GENERATED_BODY()

	explicit UPrefixSettingsEditor(const FObjectInitializer& InitializerModule);
    
public:
	// User toggle to locally disable the Editor subsystem hook for asset creation.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Asset Creation", meta = (ToolTip = "Determines whether the automatic prefixing feature is enabled for your local Editor."))
	bool bEnableAutoPrefixer = true;

	// Slate UI injection delay. Requires tweaking on slower machines where SEditableText takes longer to focus.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Asset Creation", meta = (ClampMin = "0.01", ClampMax = "1.0", ToolTip = "The delay (in seconds) before the auto-prefixer attempts to inject the text into the creation field. Increase this if the prefix doesn't appear."))
	float AutoPrefixDelay = 0.1f;

	// LIVE UI TOGGLE: Disables the red tooltip and Enter blocking when typing.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Live UI Validation", meta = (DisplayName = "Enable Live UI Validation", ToolTip = "Determines whether live UI validation (red tooltip and blocking Enter) is active."))
	bool bEnableLiveUIValidation = true;

	// NAMING AUTO-CORRECTION: Automatically appends missing prefixes/suffixes upon asset finalization (creation, escape, or rename).
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Naming Policy", meta = (ToolTip = "If enabled, the plugin automatically ensures conventions are met by appending missing prefixes/suffixes whenever an asset is created, renamed, or when renaming is canceled via Escape. If disabled, naming non-conformities are ignored."))
	bool bEnableNamingAutoCorrection = true;
	
	// VALIDATION TOGGLE: Disables local error checking when writing files.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Data Validation (Save)", meta = (ToolTip = "Determines whether the Data Validation rules are enforced when saving assets."))
	bool bEnableValidation = false;
	
	FOnValidationStateChanged OnValidationStateChangedEvent;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};