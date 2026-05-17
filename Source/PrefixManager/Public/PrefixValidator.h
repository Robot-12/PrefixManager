// Author: Robot 12

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "PrefixValidator.generated.h"

UCLASS()
class PREFIXMANAGER_API UPrefixValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UPrefixValidator();

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) override;

	virtual void PostInitProperties() override;
	
	virtual void BeginDestroy() override;

private:
	void HandleValidationSettingsChanged(bool bIsNowEnabled);

};