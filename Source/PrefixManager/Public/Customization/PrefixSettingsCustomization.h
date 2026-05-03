// Author: Robot 12

#pragma once

#include "IDetailCustomization.h"
#include "Widgets/Views/SListView.h"

class FPrefixSettingsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;

	void OnItemDeleted(const TSharedPtr<IPropertyHandle>& Item);

private:
	FReply OnResetButtonClicked();
	FReply OnAddButtonClicked();

	TSharedPtr<IPropertyHandle> ArrayPropertyHandle;
	TArray<TSharedPtr<IPropertyHandle>> ArrayElements;
	TSharedPtr<SListView<TSharedPtr<IPropertyHandle>>> ListView;
	
	void RefreshArray();
	TSharedRef<class ITableRow> OnGenerateRow(TSharedPtr<IPropertyHandle> Item, const TSharedRef<class STableViewBase>& OwnerTable);
};