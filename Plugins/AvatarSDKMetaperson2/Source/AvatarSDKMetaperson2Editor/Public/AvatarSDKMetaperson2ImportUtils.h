#pragma once

#include "CoreMinimal.h"
#include "Engine/SkeletalMesh.h" 
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AvatarSDKMetaperson2ImportUtils.generated.h"

UCLASS()
class UAvatarSDKMetaperson2ImportUtils : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    USkeleton* Skeleton;

    UPROPERTY()
    USkeletalMesh* ReferenseSM;
    UAvatarSDKMetaperson2ImportUtils() {
        Skeleton = Cast<USkeleton>(StaticLoadObject(USkeleton::StaticClass(), nullptr, *SkeletonRef));
        ReferenseSM = Cast <USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *SmRef));
    }
    static UAssetImportTask* CreateImportTask(const FString& SrcPath, const FString& DstPath, UFactory* ExtraFactory, UObject* ExtraOptions, bool& bOutSuccess);
    static UObject* ProcessImportTask(UAssetImportTask* InTask, bool& bOutSuccess);
    USkeletalMesh* ImportSkeletalMesh(FString& SrcPath, FString& DstPath, bool& bOutSuccess, bool bRtMaterilas = false);
    UFUNCTION()
    FString GetMaterialName(UMaterialInterface* Material, const int32 MaterialIndex, const FString& SlotName);
    UFUNCTION()
    FString GetTextureName(UTexture* Texture, UMaterialInterface* Material, const FString& MaterialPath, const FString& ParamName);
protected:
    const FString SkeletonRef = TEXT("/AvatarSDKMetaperson2/Skeleton/Metaperson2_Template_Skeleton");
    const FString SmRef = TEXT("/Game/MetapersonAvatars/model");
};