/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, December 2024
*/

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SAvatarSDKMetaperson2Import : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAvatarSDKMetaperson2Import)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	FReply OnLoadAnimationButtonClicked();
	
protected:
	TSet<FString> GetExistingAvatars();
	const FString SkeletalMeshDestinationDir = TEXT("/Game/MetapersonAvatars/");
	FString GenerateAssetName();
	void UpdateBlueprintProperty(const FString& BlueprintPath, USkeletalMesh* SkeletalMesh);
protected:
	FSlateFontInfo TextFont = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Medium.ttf"), 18);
	FTextBlockStyle ButtonTextStyle = FTextBlockStyle()
		.SetFont(TextFont)
		.SetColorAndOpacity(FSlateColor(FLinearColor::White))
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor::Green);
};
