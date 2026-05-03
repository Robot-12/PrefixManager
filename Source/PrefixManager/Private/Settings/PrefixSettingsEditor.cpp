// Author: Robot 12

#include "Settings/PrefixSettingsEditor.h"

UPrefixSettingsEditor::UPrefixSettingsEditor(const FObjectInitializer& InitializerModule)
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("Prefix Manager (User)");
}

FName UPrefixSettingsEditor::GetContainerName() const
{
	return FName("Editor");
}