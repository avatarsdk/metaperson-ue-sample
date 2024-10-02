// Copyright Epic Games, Inc. All Rights Reserved.

#include "AvatarSDKMetaperson2Commands.h"

#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2EditorModule"

void FAvatarSDKMetaperson2Commands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Avatar SDK Metaperson", "Import Avatar SDK Avatar", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
