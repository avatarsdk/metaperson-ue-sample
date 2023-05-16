// Copyright Epic Games, Inc. All Rights Reserved.

#include "AvatarSDKMetaperson2.h"

#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2Module"

void FAvatarSDKMetaperson2Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FAvatarSDKMetaperson2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAvatarSDKMetaperson2Module, AvatarSDKMetaperson2)