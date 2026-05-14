// Author: Robot 12

#include "Settings/PrefixSettingsEditor.h"

UPrefixSettingsEditor::UPrefixSettingsEditor(const FObjectInitializer& InitializerModule)
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("Prefix Manager (User)");
}

#if WITH_EDITOR
void UPrefixSettingsEditor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UPrefixSettingsEditor, bEnableValidation))
	{
		OnValidationStateChangedEvent.Broadcast(bEnableValidation);
	}
}
#endif