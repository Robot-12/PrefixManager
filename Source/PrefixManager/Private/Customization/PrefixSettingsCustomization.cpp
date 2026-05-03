// Author: Robot 12

#include "Customization/PrefixSettingsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Settings/PrefixSettingsProject.h"

#define LOCTEXT_NAMESPACE "PrefixSettingsPlugin"

DECLARE_DELEGATE_OneParam(FOnRowDeleted, const TSharedPtr<IPropertyHandle>&);

class SPrefixTableRow : public SMultiColumnTableRow<TSharedPtr<IPropertyHandle>>
{
public:
    SLATE_BEGIN_ARGS(SPrefixTableRow) {}
        SLATE_EVENT(FOnRowDeleted, OnRowDeletedEvent)
    SLATE_END_ARGS()

    TSharedPtr<IPropertyHandle> ItemHandle;
    FOnRowDeleted OnRowDeletedEvent;
    
    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<IPropertyHandle>& InItemHandle)
    {
        ItemHandle = InItemHandle;
        OnRowDeletedEvent = InArgs._OnRowDeletedEvent;
        
        if (ItemHandle.IsValid() && ItemHandle->IsValidHandle())
        {
            if (TSharedPtr<IPropertyHandle> ClassHandle = ItemHandle->GetChildHandle("AssetClass"); ClassHandle.IsValid())
            {
                ClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SPrefixTableRow::OnAssetClassChanged));
            }
        }

        SMultiColumnTableRow<TSharedPtr<IPropertyHandle>>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
    }

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
    {
        if (!ItemHandle.IsValid() || !ItemHandle->IsValidHandle()) return SNullWidget::NullWidget;

        if (ColumnName == "Prefix")
        {
            return SNew(SBox).Padding(4.f)
            [ ItemHandle->GetChildHandle("Prefix")->CreatePropertyValueWidget() ];
        }
        if (ColumnName == "Suffix")
        {
            return SNew(SBox).Padding(4.f)
                [ ItemHandle->GetChildHandle("Suffix")->CreatePropertyValueWidget() ];
        }
        if (ColumnName == "Class")
        {
            return SNew(SBox).Padding(4.f)
                [ ItemHandle->GetChildHandle("AssetClass")->CreatePropertyValueWidget() ];
        }
        if (ColumnName == "Apply")
        {
            TSharedPtr<IPropertyHandle> ApplyHandle = ItemHandle->GetChildHandle("bApplyToChildren");
            TSharedPtr<IPropertyHandle> ClassHandle = ItemHandle->GetChildHandle("AssetClass");

            return SNew(SBox)
                .Padding(1.f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(SCheckBox)
                    .IsChecked_Lambda([ApplyHandle]() -> ECheckBoxState
                    {
                        if (bool bValue = false; ApplyHandle.IsValid() && ApplyHandle->GetValue(bValue) == FPropertyAccess::Success)
                        {
                            return bValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                        }
                        return ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([ApplyHandle](const ECheckBoxState NewState)
                    {
                        if (ApplyHandle.IsValid())
                        {
                            ApplyHandle->SetValue(NewState == ECheckBoxState::Checked);
                        }
                    })
                    .IsEnabled_Lambda([ClassHandle]() -> bool
                    {
                        if (!ClassHandle.IsValid() || !ClassHandle->IsValidHandle()) return true;

                        UObject* ObjectValue = nullptr;
                        ClassHandle->GetValue(ObjectValue);
                        
                        if (const UClass* SelectedClass = Cast<UClass>(ObjectValue))
                        {
                            if (SelectedClass->IsChildOf(UBlueprint::StaticClass()) || SelectedClass == UBlueprint::StaticClass())
                            {
                                return false; 
                            }
                        }
                        
                        return true;
                    })
                    .ToolTipText_Lambda([ClassHandle]() -> FText
                    {
                        if (!ClassHandle.IsValid() || !ClassHandle->IsValidHandle()) return FText::GetEmpty();

                        UObject* ObjectValue = nullptr;
                        ClassHandle->GetValue(ObjectValue);
                        
                        if (const UClass* SelectedClass = Cast<UClass>(ObjectValue))
                        {
                            if (SelectedClass->IsChildOf(UBlueprint::StaticClass()) || SelectedClass == UBlueprint::StaticClass())
                            {
                                return LOCTEXT("Tooltip_ApplyLocked", "This is a core Asset Type definition. Naming rules for structural types (like Blueprints, Enums, or Structs) are forced to apply to all generated assets.");
                            }
                        }
                        
                        return FText::GetEmpty();
                    })
                ];
        }
        if (ColumnName == "Actions")
        {
            return SNew(SBox).Padding(4.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SNew(SButton)
                    .ContentPadding(2.f)
                    .OnClicked(this, &SPrefixTableRow::OnDeleteClicked)
                    .ToolTipText(LOCTEXT("Tooltip_Delete", "Delete this Row"))
                    [
                        SNew(SImage)
                        .ColorAndOpacity(FSlateColor::UseForeground())
                        .Image(FAppStyle::Get().GetBrush("Icons.Delete"))
                    ]
                ];
        }

        return SNullWidget::NullWidget;
    }

private:
    FReply OnDeleteClicked() const
    {
        OnRowDeletedEvent.ExecuteIfBound(ItemHandle);
        return FReply::Handled();
    }
    
    void OnAssetClassChanged() const
    {
        if (!ItemHandle.IsValid() || !ItemHandle->IsValidHandle()) return;

        const TSharedPtr<IPropertyHandle> ClassHandle = ItemHandle->GetChildHandle("AssetClass");

        if (const TSharedPtr<IPropertyHandle> ApplyHandle = ItemHandle->GetChildHandle("bApplyToChildren"); ClassHandle.IsValid() && ApplyHandle.IsValid())
        {
            UObject* ObjectValue = nullptr;
            ClassHandle->GetValue(ObjectValue);

            if (const UClass* SelectedClass = Cast<UClass>(ObjectValue))
            {
                if (SelectedClass->IsChildOf(UBlueprint::StaticClass()) || SelectedClass == UBlueprint::StaticClass())
                {
                    ApplyHandle->SetValue(true);
                }
            }
        }
    }
};


// =========================================================================
//  MAIN SETTINGS CLASS IMPLEMENTATION (F Prefix Settings Customization)
// =========================================================================

TSharedRef<IDetailCustomization> FPrefixSettingsCustomization::MakeInstance()
{
    return MakeShareable(new FPrefixSettingsCustomization);
}

void FPrefixSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Naming Convention");

    ArrayPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UPrefixSettingsProject, Prefixes));

    DetailBuilder.HideProperty(ArrayPropertyHandle);

    RefreshArray();

    TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow)
            + SHeaderRow::Column("Prefix")
            .DefaultLabel(LOCTEXT("Column_Prefix", "Prefix"))
            .FillWidth(0.25f)
            .DefaultTooltip(LOCTEXT("Tooltip_Prefix", "The prefix string added to the beginning of the asset name (e.g., BP_)."))

            + SHeaderRow::Column("Suffix")
            .DefaultLabel(LOCTEXT("Column_Suffix", "Suffix"))
            .FillWidth(0.25f)
            .DefaultTooltip(LOCTEXT("Tooltip_Suffix", "The suffix string appended to the end of the asset name (e.g., _Base)."))

            + SHeaderRow::Column("Class")
            .DefaultLabel(LOCTEXT("Column_Class", "Asset Class"))
            .FillWidth(0.4f)
            .DefaultTooltip(LOCTEXT("Tooltip_Class", "The target Unreal Engine asset class for this naming convention."))

            + SHeaderRow::Column("Apply")
            .DefaultLabel(LOCTEXT("Column_Apply", "Apply To Children"))
            .FillWidth(0.05f)
            .DefaultTooltip(LOCTEXT("Tooltip_Apply", "Determines whether this naming rule should also apply to derived child classes."))

            + SHeaderRow::Column("Actions")
            .DefaultLabel(LOCTEXT("Column_Actions", "Actions"))
            .FillWidth(0.05f)
            .DefaultTooltip(LOCTEXT("Tooltip_Actions", "Row actions, such as deleting this naming convention rule."));

    Category.AddCustomRow(FText::FromString("Custom Table"))
    .WholeRowContent()
    [
        SNew(SVerticalBox)
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 10.f, 0.f, 5.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Padding(2.0f)
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("Button_AddRow", "+ Add New Row"))
                .ToolTipText(LOCTEXT("Tooltip_AddRow", "Adds a new, empty naming rule to the table. You will need to define the Asset Class, Prefix, and Suffix."))
                .OnClicked(this, &FPrefixSettingsCustomization::OnAddButtonClicked)
            ]
            + SHorizontalBox::Slot()
            .Padding(2.0f)
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("Button_Reset", "Reset To Defaults"))
                .ToolTipText(LOCTEXT("Tooltip_Reset", "Restores all naming conventions to their default project settings. Warning: This will overwrite your current custom entries!"))
                .OnClicked(this, &FPrefixSettingsCustomization::OnResetButtonClicked)
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SAssignNew(ListView, SListView<TSharedPtr<IPropertyHandle>>)
            .ListItemsSource(&ArrayElements)
            .OnGenerateRow(this, &FPrefixSettingsCustomization::OnGenerateRow)
            .HeaderRow(HeaderRow)
            .SelectionMode(ESelectionMode::None)
        ]
    ];
}

void FPrefixSettingsCustomization::RefreshArray()
{
    ArrayElements.Empty();
    if (ArrayPropertyHandle.IsValid() && ArrayPropertyHandle->IsValidHandle())
    {
        const TSharedPtr<IPropertyHandleArray> ArrayHandle = ArrayPropertyHandle->AsArray();
        uint32 NumElements = 0;
        ArrayHandle->GetNumElements(NumElements);
        for (uint32 i = 0; i < NumElements; ++i)
        {
            ArrayElements.Add(ArrayHandle->GetElement(i));
        }
    }
    
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> FPrefixSettingsCustomization::OnGenerateRow(TSharedPtr<IPropertyHandle> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(SPrefixTableRow, OwnerTable, Item)
        .OnRowDeletedEvent(this, &FPrefixSettingsCustomization::OnItemDeleted);
}

FReply FPrefixSettingsCustomization::OnAddButtonClicked()
{
    if (ArrayPropertyHandle.IsValid() && ArrayPropertyHandle->IsValidHandle())
    {
        ArrayPropertyHandle->AsArray()->AddItem();
        RefreshArray();
    }
    return FReply::Handled();
}

void FPrefixSettingsCustomization::OnItemDeleted(const TSharedPtr<IPropertyHandle>& Item)
{
    if (ArrayPropertyHandle.IsValid() && ArrayPropertyHandle->IsValidHandle())
    {
        int32 IndexToRemove = -1;
        for (int32 i = 0; i < ArrayElements.Num(); ++i)
        {
            if (ArrayElements[i] == Item)
            {
                IndexToRemove = i;
                break;
            }
        }

        if (IndexToRemove != -1)
        {
            ArrayPropertyHandle->AsArray()->DeleteItem(IndexToRemove);
            RefreshArray();
        }
    }
}

FReply FPrefixSettingsCustomization::OnResetButtonClicked()
{
    if (UPrefixSettingsProject* Settings = GetMutableDefault<UPrefixSettingsProject>())
    {
        Settings->ResetToDefaults();

        TWeakPtr<FPrefixSettingsCustomization> WeakSelf = StaticCastSharedRef<FPrefixSettingsCustomization>(AsShared());

        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakSelf](float DeltaTime)
        {
            if (const TSharedPtr<FPrefixSettingsCustomization> StrongThis = WeakSelf.Pin())
            {
                StrongThis->RefreshArray();
            }
            return false;
        }), 0.01f); 
    }
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE