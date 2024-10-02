// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AvatarSDK2MetapersonStyle.h"

class FAvatarSDKMetaperson2Commands : public TCommands<FAvatarSDKMetaperson2Commands>
{
public:

	FAvatarSDKMetaperson2Commands()
		: TCommands<FAvatarSDKMetaperson2Commands>(TEXT("AvatarSDKMetaperson2"), NSLOCTEXT("Contexts", "AvatarSDKMetaperson2", "Avatar SDK Metaperson Plugin"), NAME_None, FAvatarSDK2MetapersonStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};