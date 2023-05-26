// Copyright Epic Games, Inc. All Rights Reserved.

#include "AvatarSDKMetaperson2.h"
#include "AvatarSDKRuntimeSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"


#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2Module"

void FAvatarSDKMetaperson2Module::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Avatar SDK Metaperson 2",
			LOCTEXT("AvatarSDKSettingsName", "Avatar SDK Metaperson 2"),
			LOCTEXT("AvatarSDKSettingsDescription", "Configure the Avatar SDK plugin."),
			GetMutableDefault<UAvatarSDKRuntimeSettings>()
		);

	}
}

void FAvatarSDKMetaperson2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAvatarSDKMetaperson2Module, AvatarSDKMetaperson2)