// Copyright Epic Games, Inc. All Rights Reserved.

#include "AvatarSDK2MetapersonStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FAvatarSDK2MetapersonStyle::StyleInstance = nullptr;
#define IMAGE_PLUGIN_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush( FAvatarSDK2MetapersonStyle::InContent( RelativePath, ".svg" ), __VA_ARGS__ )

void FAvatarSDK2MetapersonStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAvatarSDK2MetapersonStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAvatarSDK2MetapersonStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AvatarSDK2MetapersonStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FAvatarSDK2MetapersonStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("AvatarSDK2MetapersonStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AvatarSDKMetaperson2")->GetBaseDir() / TEXT("Resources"));

	//Style->Set("AvatarSDKMetaperson2.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("AvatarSDKMetaperson2.OpenPluginWindow", new IMAGE_PLUGIN_BRUSH_SVG("Icons/film", Icon20x20));

	return Style;
}

FString FAvatarSDK2MetapersonStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("AvatarSDKMetaperson2"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

void FAvatarSDK2MetapersonStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAvatarSDK2MetapersonStyle::Get()
{
	return *StyleInstance;
}