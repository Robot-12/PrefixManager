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
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Live UI Validation", meta = (ToolTip = "Determines whether live UI validation (red tooltip and blocking Enter) is active."))
	bool bEnableLiveUIValidation = true;

	// AUTO-REPAIR TOGGLE: Automatically adds missing prefixes/suffixes when renaming.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Auto Fix Renaming", meta = (ToolTip = "If enabled, renaming an asset without the required prefix/suffix will automatically append them. If disabled, incorrect renames are ignored."))
	bool bEnableAutoFixRenaming = true;
	
	// VALIDATION TOGGLE: Disables local error checking when writing files.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix | Data Validation (Save)", meta = (ToolTip = "Determines whether the Data Validation rules are enforced when saving assets."))
	bool bEnableValidation = false;
	
	FOnValidationStateChanged OnValidationStateChangedEvent;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};