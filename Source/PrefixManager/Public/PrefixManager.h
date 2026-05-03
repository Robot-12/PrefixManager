// Author: Robot 12

#pragma once

#include "Modules/ModuleManager.h"

class FPrefixManagerModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void HandleOnNewAssetCreated(UFactory* Factory);
};
