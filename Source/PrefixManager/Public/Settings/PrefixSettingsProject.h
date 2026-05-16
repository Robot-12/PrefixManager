// Author: Robot 12

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PrefixSettingsProject.generated.h"

USTRUCT(BlueprintType)
struct FPrefixClass
{
	GENERATED_BODY()
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PrefixClass")
	FString Prefix;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PrefixClass")
	FString Suffix;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PrefixClass")
	UClass* AssetClass; 
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PrefixClass")
	bool bApplyToChildren = true;
};

UCLASS(config=Engine, defaultconfig)
class PREFIXMANAGER_API UPrefixSettingsProject : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPrefixSettingsProject();
	
	virtual void PostInitProperties() override;
	
	void ResetToDefaults();

	const FPrefixClass* GetRuleForClass(const UClass* TargetClass, const UClass* BaseClassType = nullptr) const;

	static void ResolveAssetClassAndType(UObject* ContextObject, const UClass* DefaultClass, const UClass*& OutAssetClass, const UClass*& OutClassType);
	
	UPROPERTY(Config, EditAnywhere, Category = "Naming Convention")
	TArray<FPrefixClass> Prefixes;
};