// Fill out your copyright notice in the Description page of Project Settings.


#include "AvatarSDKMetaperson2Import.h"
#include "SlateOptMacros.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"

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
	const FString SkeletalMeshDestinationDir = TEXT("/Game/MetapersonAvatars/");
	FString SkeletalMeshDestination = SkeletalMeshDestinationDir + TEXT("SM_Metaperson_0");

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
