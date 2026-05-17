// Author: Robot 12

#include "PrefixValidator.h"
#include "Settings/PrefixSettingsProject.h"
#include "Engine/Blueprint.h"
#include "WidgetBlueprint.h"
#include "Settings/PrefixSettingsEditor.h"

#define LOCTEXT_NAMESPACE "PrefixManagerPlugin"

UPrefixValidator::UPrefixValidator()
{
	bIsEnabled = true;
}

void UPrefixValidator::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (UPrefixSettingsEditor* Settings = GetMutableDefault<UPrefixSettingsEditor>())
		{
			bIsEnabled = Settings->bEnableValidation;
            
			Settings->OnValidationStateChangedEvent.AddUObject(this, &UPrefixValidator::HandleValidationSettingsChanged);
		}
	}
}

void UPrefixValidator::BeginDestroy()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (UPrefixSettingsEditor* Settings = GetMutableDefault<UPrefixSettingsEditor>())
		{
			Settings->OnValidationStateChangedEvent.RemoveAll(this);
		}
	}
	
	Super::BeginDestroy();
}
void UPrefixValidator::HandleValidationSettingsChanged(bool bIsNowEnabled)
{
	bIsEnabled = bIsNowEnabled;
}

bool UPrefixValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	const FString Path = InObject->GetPathName();
	if (Path.StartsWith(TEXT("/Engine/")))
	{
		return false;
	}

	return true;
}

EDataValidationResult UPrefixValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    if (!InAsset) return EDataValidationResult::NotValidated;

    const UPrefixSettingsProject* ProjectSettings = GetDefault<UPrefixSettingsProject>();
    if (!ProjectSettings || ProjectSettings->Prefixes.IsEmpty()) 
    {
       return EDataValidationResult::NotValidated;
    }
	
    const UClass* AssetClass = nullptr;
    const UClass* ClassType = nullptr;
    UPrefixSettingsProject::ResolveAssetClassAndType(InAsset, InAsset->GetClass(), AssetClass, ClassType);

    const FPrefixClass* MatchedPrefixData = ProjectSettings->GetRuleForClass(AssetClass, ClassType);

    if (!MatchedPrefixData)
    {
       AssetPasses(InAsset);
       return EDataValidationResult::Valid;
    }

    const FString AssetName = InAsset->GetName();
    const bool bValidPrefix = MatchedPrefixData->Prefix.IsEmpty() || AssetName.StartsWith(MatchedPrefixData->Prefix);
    const bool bValidSuffix = MatchedPrefixData->Suffix.IsEmpty() || AssetName.EndsWith(MatchedPrefixData->Suffix);

    if (!bValidPrefix || !bValidSuffix)
    {
       FText ErrorMsg = FText::Format(
          LOCTEXT("InvalidNameError", "Name '{0}' is invalid for class {1}."),
          FText::FromString(AssetName),
          FText::FromString(ClassType->GetName())
       );

       if (!bValidPrefix)
       {
          const FText PrefixError = FText::Format(
             LOCTEXT("MissingPrefixError", "Required prefix: '{0}'."),
             FText::FromString(MatchedPrefixData->Prefix)
          );
          ErrorMsg = FText::Join(FText::FromString(TEXT(" ")), ErrorMsg, PrefixError);
       }

       if (!bValidSuffix)
       {
          const FText SuffixError = FText::Format(
             LOCTEXT("MissingSuffixError", "Required suffix: '{0}'."),
             FText::FromString(MatchedPrefixData->Suffix)
          );
          ErrorMsg = FText::Join(FText::FromString(TEXT(" ")), ErrorMsg, SuffixError);
       }

       AssetFails(InAsset, ErrorMsg);
       return EDataValidationResult::Invalid;
    }

    AssetPasses(InAsset);
    return EDataValidationResult::Valid;
}
#undef LOCTEXT_NAMESPACE