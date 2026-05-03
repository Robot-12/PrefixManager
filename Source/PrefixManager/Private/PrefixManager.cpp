// Author: Robot 12

#include "PrefixManager.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Containers/Ticker.h"
#include "Factories/Factory.h"
#include "InputCoreTypes.h"
#include "Input/Events.h" 
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetToolsModule.h"
#include "WidgetBlueprint.h"
#include "Customization/PrefixSettingsCustomization.h"
#include "Factories/BlueprintFactory.h"
#include "Settings/PrefixSettingsEditor.h"
#include "Settings/PrefixSettingsProject.h"

void FPrefixManagerModule::StartupModule()
{
    FEditorDelegates::OnNewAssetCreated.AddRaw(this, &FPrefixManagerModule::HandleOnNewAssetCreated);
    
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    
    PropertyModule.RegisterCustomClassLayout(
        "PrefixSettingsProject",
        FOnGetDetailCustomizationInstance::CreateStatic(&FPrefixSettingsCustomization::MakeInstance)
    );
}

void FPrefixManagerModule::ShutdownModule()
{
    FEditorDelegates::OnNewAssetCreated.RemoveAll(this);
    
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout("PrefixSettingsProject");
    }
}

void FPrefixManagerModule::HandleOnNewAssetCreated(UFactory* Factory)
{
    if (!Factory || !Factory->GetSupportedClass()) return;
    
    const UPrefixSettingsEditor* EditorSettings = GetDefault<UPrefixSettingsEditor>();
    if (EditorSettings && !EditorSettings->bEnableAutoPrefixer) return;

    const UClass* AssetClass = Factory->GetSupportedClass();
    const UClass* ClassType = Factory->GetSupportedClass();
    
    if (ClassType->IsChildOf(UWidgetBlueprint::StaticClass()))
    {
        ClassType = UWidgetBlueprint::StaticClass();
    }

    if (const UBlueprintFactory* BPFactory = Cast<UBlueprintFactory>(Factory))
    {
        if (BPFactory->ParentClass != nullptr)
        {
            AssetClass = BPFactory->ParentClass;
        }
    }
    
    const FString ClassName = AssetClass->GetName();
    
    if (const UPrefixSettingsProject* ProjectSettings = GetDefault<UPrefixSettingsProject>())
    {
        const FPrefixClass* MatchedPrefixData = nullptr;
        
        const UClass* CurrentClass = AssetClass;
        bool bIsExactClass = true;
        
        while (CurrentClass)
        {
            for (const FPrefixClass& PrefixClass : ProjectSettings->Prefixes)
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
            
            if (MatchedPrefixData)
            {
                break;
            }
                
            CurrentClass = CurrentClass->GetSuperClass();
            bIsExactClass = false;
        }
        
        if (!MatchedPrefixData)
        {
            for (const FPrefixClass& PrefixClass : ProjectSettings->Prefixes)
            {
                if (PrefixClass.AssetClass == ClassType)
                {
                    MatchedPrefixData = &PrefixClass;
                    break;
                }
            }
        }

       if (MatchedPrefixData)
       {
           const FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
           TArray<FString> SelectedPaths;
           ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPaths);
           
           FString CurrentPath = (SelectedPaths.Num() > 0) ? SelectedPaths[0] : TEXT("/Game");
           
           if (CurrentPath.StartsWith(TEXT("/All")))
           {
               CurrentPath.RemoveFromStart(TEXT("/All"));
           }
           if (CurrentPath.IsEmpty()) CurrentPath = TEXT("/Game");

           const FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
           FString BaseName = *MatchedPrefixData->Prefix + Factory->GetDefaultNewAssetName() + MatchedPrefixData->Suffix;

           FString OutPackageName;
           FString OutAssetName;

           const FString FullBaseName = FPaths::Combine(CurrentPath, BaseName);
           
           AssetToolsModule.Get().CreateUniqueAssetName(FullBaseName, TEXT(""), OutPackageName, OutAssetName);
           
           //HACK: Unreal does not provide an API to edit the initial rename field in the content browser before the user approves the name
           FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([OutAssetName](float DeltaTime)
           {
               if (const TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetKeyboardFocusedWidget(); FocusedWidget.IsValid())
               {
                   if (const FName WidgetType = FocusedWidget->GetType(); WidgetType == "SEditableText")
                   {
                       if (const TSharedPtr<SEditableText> EditableTextWidget = StaticCastSharedPtr<SEditableText>(FocusedWidget))
                       {
                            EditableTextWidget->SetText(FText::FromString(OutAssetName));

                           const FModifierKeysState CtrlModifier{false,false,false,true,false,false,false,false,false};

                           const FKeyEvent SelectAllDown(EKeys::A, CtrlModifier, 0, false, 0, 0);
                           FSlateApplication::Get().ProcessKeyDownEvent(SelectAllDown);
                           const FKeyEvent SelectAllUp(EKeys::A, CtrlModifier, 0, false, 0, 0);
                           FSlateApplication::Get().ProcessKeyUpEvent(SelectAllUp);
                       }
                   }
               }
               
               return false;
           }), 0.01f); 
       }
    }
}

IMPLEMENT_MODULE(FPrefixManagerModule, PrefixManager)