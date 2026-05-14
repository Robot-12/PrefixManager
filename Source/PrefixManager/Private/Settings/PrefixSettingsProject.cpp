// Author: Robot 12

#include "Settings/PrefixSettingsProject.h"

#include "WidgetBlueprint.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/Font.h"
#include "Engine/UserDefinedEnum.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundMix.h"
#include "StructUtils/UserDefinedStruct.h"


UPrefixSettingsProject::UPrefixSettingsProject()
{
	CategoryName = TEXT("Plugins"); 
	SectionName = TEXT("Prefix Manager (Rules)"); 
}

void UPrefixSettingsProject::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (Prefixes.IsEmpty())
	{
		ResetToDefaults();
	}
}

void UPrefixSettingsProject::ResetToDefaults()
{
	Prefixes.Empty();
	FPrefixClass Rule;

	// --- COMMON ---
	Rule.Prefix = TEXT("BP_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UBlueprint::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("M_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UMaterial::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("SM_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UStaticMesh::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("SK_"); Rule.Suffix = TEXT(""); Rule.AssetClass = USkeletalMesh::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("T_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UTexture2D::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("VFX_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UParticleSystem::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("L_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UWorld::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- MATERIALS ---
	Rule.Prefix = TEXT("MI_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UMaterialInstanceConstant::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("MF_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UMaterialFunction::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("PM_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UPhysicalMaterial::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- UI ---
	Rule.Prefix = TEXT("WBP_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UWidgetBlueprint::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("Font_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UFont::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- ANIMATIONS ---
	Rule.Prefix = TEXT("SK_"); Rule.Suffix = TEXT("_Skeleton"); Rule.AssetClass = USkeleton::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("ABP_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UAnimBlueprint::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("A_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UAnimSequence::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("AM_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UAnimMontage::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("BS_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UBlendSpace::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("AO_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UAimOffsetBlendSpace::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- AUDIO ---
	Rule.Prefix = TEXT("SFX_"); Rule.Suffix = TEXT(""); Rule.AssetClass = USoundWave::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("SFX_"); Rule.Suffix = TEXT("_Cue"); Rule.AssetClass = USoundCue::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("Mix_"); Rule.Suffix = TEXT(""); Rule.AssetClass = USoundMix::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("ATT_"); Rule.Suffix = TEXT(""); Rule.AssetClass = USoundAttenuation::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- AI ---
	Rule.Prefix = TEXT("BT_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UBehaviorTree::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("BB_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UBlackboardData::StaticClass(); 
	Prefixes.Add(Rule);
	
	// --- MISC / PHYSICS ---
	Rule.Prefix = TEXT("DT_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UDataTable::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("E_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UUserDefinedEnum::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("S_"); Rule.Suffix = TEXT(""); Rule.AssetClass = UUserDefinedStruct::StaticClass(); 
	Prefixes.Add(Rule);
	
	Rule.Prefix = TEXT("SK_"); Rule.Suffix = TEXT("_Phys"); Rule.AssetClass = UPhysicsAsset::StaticClass(); 
	Prefixes.Add(Rule);

	SaveConfig();
}

const FPrefixClass* UPrefixSettingsProject::GetRuleForClass(const UClass* TargetClass, const UClass* BaseClassType) const
{
	const FPrefixClass* MatchedPrefixData = nullptr;
	const UClass* CurrentClass = TargetClass;
	bool bIsExactClass = true;

	while (CurrentClass)
	{
		for (const FPrefixClass& PrefixClass : Prefixes)
		{
			if (PrefixClass.AssetClass == CurrentClass)
			{
				if (bIsExactClass || PrefixClass.bApplyToChildren)
				{
					MatchedPrefixData = &PrefixClass;
					break;
				}
			}
		}
            
		if (MatchedPrefixData) break;
                
		CurrentClass = CurrentClass->GetSuperClass();
		bIsExactClass = false;
	}

	if (!MatchedPrefixData && BaseClassType)
	{
		for (const FPrefixClass& PrefixClass : Prefixes)
		{
			if (PrefixClass.AssetClass == BaseClassType)
			{
				MatchedPrefixData = &PrefixClass;
				break;
			}
		}
	}

	return MatchedPrefixData;
}