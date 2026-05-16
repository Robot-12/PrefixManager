// Author: Robot 12

#include "PrefixManager.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Factories/Factory.h"
#include "InputCoreTypes.h"
#include "Input/Events.h" 
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetToolsModule.h"
#include "Customization/PrefixSettingsCustomization.h"
#include "Settings/PrefixSettingsEditor.h"
#include "Settings/PrefixSettingsProject.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Text/SInlineEditableTextBlock.h" 
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "PrefixManagerPlugin"

void FPrefixManagerModule::AddSafeTicker(TFunction<bool(float)> InFunction, float Delay)
{
    TSharedPtr<FTSTicker::FDelegateHandle> HandlePtr = MakeShared<FTSTicker::FDelegateHandle>();
    
    *HandlePtr = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, InFunction, HandlePtr](float DeltaTime) -> bool
    {
        const bool bContinue = InFunction(DeltaTime);
        
        if (!bContinue && HandlePtr.IsValid())
        {
            ActiveTickerHandles.Remove(*HandlePtr);
        }
        
        return bContinue; 
    }), Delay);

    ActiveTickerHandles.Add(*HandlePtr);
}

void FPrefixManagerModule::StartupModule()
{
    FEditorDelegates::OnNewAssetCreated.AddRaw(this, &FPrefixManagerModule::HandleOnNewAssetCreated);
    
    if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
    {
        IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        AssetRegistry.OnAssetRenamed().AddRaw(this, &FPrefixManagerModule::HandleOnAssetRenamed);
        AssetRegistry.OnAssetAdded().AddRaw(this, &FPrefixManagerModule::HandleOnAssetAdded);
    }

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().OnFocusChanging().AddRaw(this, &FPrefixManagerModule::HandleOnFocusChanging);
    }
    
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        "PrefixSettingsProject",
        FOnGetDetailCustomizationInstance::CreateStatic(&FPrefixSettingsCustomization::MakeInstance)
    );
}

void FPrefixManagerModule::ShutdownModule()
{
    for (FTSTicker::FDelegateHandle& Handle : ActiveTickerHandles)
    {
        if (Handle.IsValid())
        {
            FTSTicker::GetCoreTicker().RemoveTicker(Handle);
        }
    }
    ActiveTickerHandles.Empty();

    FEditorDelegates::OnNewAssetCreated.RemoveAll(this);
    
    if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
    {
        IAssetRegistry& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        AssetRegistry.OnAssetRenamed().RemoveAll(this);
        AssetRegistry.OnAssetAdded().RemoveAll(this);
    }

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().OnFocusChanging().RemoveAll(this);
    }
    
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout("PrefixSettingsProject");
    }
}

void FPrefixManagerModule::HandleOnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
{
    const UPrefixSettingsEditor* EditorSettings = GetDefault<UPrefixSettingsEditor>();
    if (!EditorSettings || !EditorSettings->bEnableLiveUIValidation) return;

    if (!NewFocusedWidget.IsValid() || NewFocusedWidget->GetType() != FName("SEditableText")) return;
    
    AddSafeTicker([this, NewFocusedWidget](float DeltaTime) -> bool
    {
        if (!NewFocusedWidget.IsValid()) return false;

        if (!FModuleManager::Get().IsModuleLoaded("ContentBrowser")) return false;
        const FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
        TArray<FAssetData> SelectedAssets;
        ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

        const UClass* AssetClass = nullptr;
        const UClass* ClassType = nullptr;

        if (SelectedAssets.Num() == 1) 
        {
            const FAssetData& SelectedAsset = SelectedAssets[0];
            UPrefixSettingsProject::ResolveAssetClassAndType(SelectedAsset.FastGetAsset(true), SelectedAsset.GetClass(), AssetClass, ClassType);
        }
        else if (PendingAssetClass != nullptr)
        {
            AssetClass = PendingAssetClass;
            ClassType = PendingClassType;
            
            PendingAssetClass = nullptr;
            PendingClassType = nullptr;
        }
        else
        {
            return false; 
        }

        const UPrefixSettingsProject* ProjectSettings = GetDefault<UPrefixSettingsProject>();
        if (!ProjectSettings) return false;

        const FPrefixClass* MatchedPrefixData = ProjectSettings->GetRuleForClass(AssetClass, ClassType);
        if (!MatchedPrefixData) return false;

        FString ExpectedPrefix = MatchedPrefixData->Prefix;
        FString ExpectedSuffix = MatchedPrefixData->Suffix;

        TSharedPtr<SWidget> CurrentWidget = NewFocusedWidget;
        TSharedPtr<SInlineEditableTextBlock> InlineTextWidget = nullptr;
        int32 SafeDepthLimit = 100; 

        while (CurrentWidget.IsValid() && SafeDepthLimit > 0)
        {
            if (CurrentWidget->GetType() == FName("SInlineEditableTextBlock"))
            {
                InlineTextWidget = StaticCastSharedPtr<SInlineEditableTextBlock>(CurrentWidget);
                break;
            }
            CurrentWidget = CurrentWidget->GetParentWidget();
            SafeDepthLimit--;
        }

        if (InlineTextWidget.IsValid())
        {
            InlineTextWidget->SetOnVerifyTextChanged(FOnVerifyTextChanged::CreateLambda([ExpectedPrefix, ExpectedSuffix](const FText& InNewText, FText& OutErrorMessage) -> bool
            {
                const FString NewNameStr = InNewText.ToString();

                const FString InvalidChars = TEXT("\"' ,./\\!:|&<>\n\r\t@#?");
                for (int32 i = 0; i < InvalidChars.Len(); ++i)
                {
                    FString CharStr = InvalidChars.Mid(i, 1);
                    if (NewNameStr.Contains(CharStr))
                    {
                        if (CharStr == TEXT(" ")) 
                            OutErrorMessage = NSLOCTEXT("PrefixManagerPlugin", "Error_Whitespace", "Name may not contain whitespace characters (space)");
                        else 
                            OutErrorMessage = FText::Format(NSLOCTEXT("PrefixManagerPlugin", "Error_InvalidChars", "Name may not contain the following characters: {0}"), FText::FromString(CharStr));
                        return false;
                    }
                }

                const UPrefixSettingsEditor* LiveSettings = GetDefault<UPrefixSettingsEditor>();
                if (LiveSettings && !LiveSettings->bEnableLiveUIValidation) return true;

                bool bValidPrefix = ExpectedPrefix.IsEmpty() || NewNameStr.StartsWith(ExpectedPrefix);
                bool bValidSuffix = ExpectedSuffix.IsEmpty() || NewNameStr.EndsWith(ExpectedSuffix);

                if (!bValidPrefix || !bValidSuffix)
                {
                    OutErrorMessage = FText::Format(
                        NSLOCTEXT("PrefixManagerPlugin", "Error_InvalidFormat", "Invalid name format! Expected: {0}...{1}"),
                        FText::FromString(ExpectedPrefix),
                        FText::FromString(ExpectedSuffix)
                    );
                    return false;
                }
                
                return true; 
            }));
        }

        return false; 
    });
}

void FPrefixManagerModule::HandleOnNewAssetCreated(UFactory* Factory)
{
    if (!Factory || !Factory->GetSupportedClass()) return;

    UPrefixSettingsProject::ResolveAssetClassAndType(Factory, Factory->GetSupportedClass(), PendingAssetClass, PendingClassType);

    const UPrefixSettingsEditor* EditorSettings = GetDefault<UPrefixSettingsEditor>();
    if (EditorSettings && !EditorSettings->bEnableAutoPrefixer) return;

    if (const UPrefixSettingsProject* ProjectSettings = GetDefault<UPrefixSettingsProject>())
    {
       if (const FPrefixClass* MatchedPrefixData = ProjectSettings->GetRuleForClass(PendingAssetClass, PendingClassType))
       {
           const FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
           TArray<FString> SelectedPaths;
           ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPaths);
           
           FString CurrentPath = (SelectedPaths.Num() > 0) ? SelectedPaths[0] : TEXT("/Game");
           if (CurrentPath.StartsWith(TEXT("/All"))) CurrentPath.RemoveFromStart(TEXT("/All"));
           if (CurrentPath.IsEmpty()) CurrentPath = TEXT("/Game");

           const FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
           
           FString BaseName = MatchedPrefixData->Prefix + Factory->GetDefaultNewAssetName() + MatchedPrefixData->Suffix;

           FString OutPackageName;
           FString OutAssetName;
           const FString FullBaseName = FPaths::Combine(CurrentPath, BaseName);
           
           AssetToolsModule.Get().CreateUniqueAssetName(FullBaseName, TEXT(""), OutPackageName, OutAssetName);
           
           AddSafeTicker([OutAssetName](float DeltaTime) -> bool
           {
               if (const TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetKeyboardFocusedWidget(); FocusedWidget.IsValid())
               {
                   if (FocusedWidget->GetType() == FName("SEditableText"))
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
           }, EditorSettings->AutoPrefixDelay); 
       }
    }
}

void FPrefixManagerModule::HandleOnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath)
{
    ExecuteNamingAutoCorrection(AssetData);
}

void FPrefixManagerModule::HandleOnAssetAdded(const FAssetData& AssetData)
{
    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    if (AssetRegistry.IsLoadingAssets()) return;

    ExecuteNamingAutoCorrection(AssetData);
}

void FPrefixManagerModule::ExecuteNamingAutoCorrection(const FAssetData& AssetData)
{
    const FString CurrentObjectPath = AssetData.GetObjectPathString();

    if (CurrentObjectPath.Contains(TEXT("RenameTmp")))
    {
        return;
    }
    
    if (ActiveReverts.Contains(CurrentObjectPath))
    {
        ActiveReverts.Remove(CurrentObjectPath);
        return;
    }

    const UPrefixSettingsEditor* EditorSettings = GetDefault<UPrefixSettingsEditor>();
    if (EditorSettings && !EditorSettings->bEnableNamingAutoCorrection) return;

    if (AssetData.PackagePath.ToString().StartsWith(TEXT("/Engine/"))) return;

    const UPrefixSettingsProject* ProjectSettings = GetDefault<UPrefixSettingsProject>();
    if (!ProjectSettings) return;

    UObject* LoadedAsset = AssetData.GetAsset();
    if (!LoadedAsset) return;

    const UClass* AssetClass = nullptr;
    const UClass* ClassType = nullptr;
    UPrefixSettingsProject::ResolveAssetClassAndType(LoadedAsset, LoadedAsset->GetClass(), AssetClass, ClassType);

    if (const FPrefixClass* MatchedPrefixData = ProjectSettings->GetRuleForClass(AssetClass, ClassType))
    {
        const FString NewName = AssetData.AssetName.ToString();
        const bool bValidPrefix = MatchedPrefixData->Prefix.IsEmpty() || NewName.StartsWith(MatchedPrefixData->Prefix);
        const bool bValidSuffix = MatchedPrefixData->Suffix.IsEmpty() || NewName.EndsWith(MatchedPrefixData->Suffix);

        if (!bValidPrefix || !bValidSuffix)
        {
            FString FinalName = NewName;
            if (!bValidPrefix) FinalName = MatchedPrefixData->Prefix + FinalName;
            if (!bValidSuffix) FinalName = FinalName + MatchedPrefixData->Suffix;

            FString CurrentPackagePath = AssetData.PackagePath.ToString();
            FString TargetActionPath = FString::Printf(TEXT("%s/%s.%s"), *CurrentPackagePath, *FinalName, *FinalName);
            
            ActiveReverts.Add(TargetActionPath);

            const FText MessageStr = FText::Format(
                LOCTEXT("Notification_AutoFixed", "Auto-fixed asset name to: {0}"),
                FText::FromString(FinalName)
            );

            FNotificationInfo Info(MessageStr);
            Info.ExpireDuration = 4.0f;
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Info"));
            const TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
            if (Notification.IsValid()) Notification->SetCompletionState(SNotificationItem::CS_Success);

            TWeakObjectPtr<UObject> WeakAsset = LoadedAsset;
            
            AddSafeTicker([this, WeakAsset, CurrentPackagePath, FinalName, TargetActionPath](float DeltaTime) -> bool
            {
                if (UObject* AssetToRename = WeakAsset.Get())
                {
                    const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
                    TArray<FAssetRenameData> AssetsToRename;
                    AssetsToRename.Emplace(AssetToRename, CurrentPackagePath, FinalName);
                    
                    AssetToolsModule.Get().RenameAssets(AssetsToRename);
                }
                else
                {
                    ActiveReverts.Remove(TargetActionPath);
                }
                return false;
            }, 0.0f);
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPrefixManagerModule, PrefixManager)