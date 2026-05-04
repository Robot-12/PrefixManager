// Author: Robot 12

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PrefixSettingsEditor.generated.h"

UCLASS(config=EditorSettings, meta=(DisplayName="Prefix Settings"))
class PREFIXMANAGER_API UPrefixSettingsEditor : public UDeveloperSettings
{
	GENERATED_BODY()

	explicit UPrefixSettingsEditor(const FObjectInitializer& InitializerModule);
	
public:
	// User toggle to locally disable the Editor subsystem hook for asset creation.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix", meta = (ToolTip = "Determines whether the automatic prefixing feature is enabled for your local Editor."))
	bool bEnableAutoPrefixer = true;

	// Slate UI injection delay. Requires tweaking on slower machines where SEditableText takes longer to focus.
	UPROPERTY(Config, EditAnywhere, Category = "AutoPrefix", meta = (ClampMin = "0.01", ClampMax = "1.0", ToolTip = "The delay (in seconds) before the auto-prefixer attempts to inject the text into the creation field. Increase this if the prefix doesn't appear."))
	float AutoPrefixDelay = 0.1f;
};