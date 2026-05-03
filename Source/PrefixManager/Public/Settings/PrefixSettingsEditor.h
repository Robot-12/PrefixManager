// Author: Robot 12

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PrefixSettingsEditor.generated.h"

/**
 * 
 */
UCLASS(config=Editor, meta=(DisplayName="Prefix Settings"))
class PREFIXMANAGER_API UPrefixSettingsEditor : public UDeveloperSettings
{
	GENERATED_BODY()

	explicit UPrefixSettingsEditor(const FObjectInitializer& InitializerModule);
	
	virtual FName GetContainerName() const override;
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bEnableAutoPrefixer = true;
};
