// Fill out your copyright notice in the Description page of Project Settings.


#include "AvatarSDKMetaperson2Import.h"
#include "SlateOptMacros.h"
#include <AvatarSDKMetaperson2ImportUtils.h>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAvatarSDKMetaperson2Import::Construct(const FArguments& InArgs)
{
	ChildSlot
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Top)
	[
		SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(FText::FromString("Import Avatar"))
			.OnClicked(FOnClicked::CreateSP(this, &SAvatarSDKMetaperson2Import::OnLoadAnimationButtonClicked))
	];
}
FReply SAvatarSDKMetaperson2Import::OnLoadAnimationButtonClicked()
{
	//FString AvatarPath(TEXT("I:\\tasks\\z3d\\task20241003-mpc2ue\\mpc_exported\\avatar\\model.fbx"));
	FString AvatarPath(TEXT("I:\\tasks\\z3d\\task20241812-fbx-import-ue-again\\avatar (22)\\avatar\\model.fbx"));
	const FString SkeletalMeshDestinationDir = TEXT("/Game/MetapersonAvatars/");
	FString SkeletalMeshDestination = SkeletalMeshDestinationDir + TEXT("SM_Metaperson_0");
	UAvatarSDKMetaperson2ImportUtils* Importer = NewObject<UAvatarSDKMetaperson2ImportUtils>();
	bool bSuccess = false;
	Importer->ImportSkeletalMesh(AvatarPath, SkeletalMeshDestination, bSuccess);
	if (bSuccess) {

	}
	return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
