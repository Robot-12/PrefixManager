// Author: Robot 12

#pragma once

#include "Modules/ModuleManager.h"

class FPrefixManagerModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void HandleOnNewAssetCreated(UFactory* Factory);
	void HandleOnAssetAdded(const FAssetData& AssetData);
	void HandleOnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath);
	
private:
	void HandleOnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget);
	void AddSafeTicker(TFunction<bool(float)> InFunction, float Delay = 0.0f);
	void ExecuteNamingAutoCorrection(const FAssetData& AssetData);
	
	TSet<FString> ActiveReverts;
	TArray<FTSTicker::FDelegateHandle> ActiveTickerHandles;
	const UClass* PendingAssetClass = nullptr;
	const UClass* PendingClassType = nullptr;
};