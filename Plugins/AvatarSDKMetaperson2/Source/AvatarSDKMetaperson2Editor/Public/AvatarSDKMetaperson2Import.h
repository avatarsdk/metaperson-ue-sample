// Fill out your copyright notice in the Description page of Project Settings.

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
};
