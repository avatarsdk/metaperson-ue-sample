/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, December 2024
*/

#include "AvatarSDK2MetapersonStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FAvatarSDK2MetapersonStyle::StyleInstance = nullptr;
#define IMAGE_PLUGIN_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush( FAvatarSDK2MetapersonStyle::InContent( RelativePath, ".svg" ), __VA_ARGS__ )
#define IMAGE_PLUGIN_BRUSH_PNG( RelativePath, ... ) FSlateImageBrush( FAvatarSDK2MetapersonStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
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
	Style->Set("AvatarSDKMetaperson2.OpenPluginWindow", new IMAGE_PLUGIN_BRUSH_PNG("Icons/logo", Icon20x20));
	Style->Set("AvatarSDKMetaperson2.MainLogo", new IMAGE_PLUGIN_BRUSH_PNG("Icons/metaperson-avatars-logo", FVector2D(360.0f, 120.0f)));

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
