/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, December 2024
*/


#include "AvatarSDKMetaperson2Import.h"
#include "SlateOptMacros.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AvatarSDK2MetapersonStyle.h"
#include <AvatarSDKMetaperson2ImportUtils.h>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAvatarSDKMetaperson2Import::Construct(const FArguments& InArgs)
{
	ChildSlot
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Fill)[
		SNew(SBox)
			
			//	.MinDesiredWidth(500.0)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HeightOverride(500.0)
			.WidthOverride(500.0)[
		SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0.0f, 35.0f, 0.0f, 5.0f)
			.AutoHeight()[
				SNew(SImage)
					.Image(FAvatarSDK2MetapersonStyle::Get().GetBrush("AvatarSDKMetaperson2.MainLogo"))
			] + 
			SVerticalBox::Slot().AutoHeight()
					.MaxHeight(80.0f)
					.VAlign(EVerticalAlignment::VAlign_Fill)
					.Padding(0.0f, 35.0f, 0.0f, 5.0f)[
		SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.TextStyle(&ButtonTextStyle)
			.Text(FText::FromString("Import Avatar"))
			.OnClicked(FOnClicked::CreateSP(this, &SAvatarSDKMetaperson2Import::OnLoadAnimationButtonClicked))
		]]
	];
}

TSet<FString> SAvatarSDKMetaperson2Import::GetExistingAvatars()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter;

	Filter.PackagePaths.Add(*SkeletalMeshDestinationDir);
	Filter.bRecursivePaths = true;

	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	TSet<FString> Existing;
	for (auto Data : AssetData) {
		Existing.Add(FPaths::GetPathLeaf(Data.PackagePath.ToString()));
	}
	return Existing;
}

FString SAvatarSDKMetaperson2Import::GenerateAssetName()
{
	auto Existing = GetExistingAvatars();
	FString Variant = (TEXT("Metaperson_0"));
	int Num = 1;
	while (Existing.Contains(Variant)) {
		Variant = FString::Printf(TEXT("Metaperson_%d"), Num++);
	}
	return Variant;
}

void SAvatarSDKMetaperson2Import::UpdateBlueprintProperty(const FString& BlueprintPath, USkeletalMesh* SkeletalMesh)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
    if (!Blueprint) {
        return;
    }

	AActor* DefaultActor = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());
	if (!DefaultActor)
	{
		return;
	}
	auto Components = DefaultActor->GetComponents();

	for(auto Component : Components)
    {
        if (Component->IsA(USkeletalMeshComponent::StaticClass()))
        {
            USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component);
			if(!SkeletalMeshComponent) {
                continue;
            }
			if (SkeletalMeshComponent->GetName() == TEXT("FbxMesh")) {
				SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
				SkeletalMeshComponent->SetVisibility(true);
			} else {
				SkeletalMeshComponent->SetVisibility(false);
			}
        }
    }	
}

FReply SAvatarSDKMetaperson2Import::OnLoadAnimationButtonClicked()
{
	auto AssetName = GenerateAssetName();
	FString SkeletalMeshDestination = SkeletalMeshDestinationDir / AssetName / AssetName;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	FString DialogTitle(TEXT("Import FBX Metaperson Model"));
	TArray<FString> OutFiles;
	bool bOpenRes = DesktopPlatform->OpenFileDialog(ParentWindowPtr, DialogTitle, TEXT(""), TEXT(""), TEXT("Fbx files (*.fbx)|*.fbx"), EFileDialogFlags::None, OutFiles);
	if(!bOpenRes || OutFiles.Num() == 0) {
        return FReply::Handled();
    }
	FString AvatarPath = OutFiles[0];
	UAvatarSDKMetaperson2ImportUtils* Importer = NewObject<UAvatarSDKMetaperson2ImportUtils>();

	bool bSuccess = false;
	auto FbxImportedSkeletalMesh = Importer->ImportSkeletalMesh(AvatarPath, SkeletalMeshDestination, bSuccess);
	if (!bSuccess) {
		return FReply::Handled();
	}	
	FString BlueprintPath = TEXT("/AvatarSDKMetaperson2/ThirdPerson/Blueprints/BP_ThirdPersonCharacter");
	UpdateBlueprintProperty(BlueprintPath, FbxImportedSkeletalMesh);
	return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
