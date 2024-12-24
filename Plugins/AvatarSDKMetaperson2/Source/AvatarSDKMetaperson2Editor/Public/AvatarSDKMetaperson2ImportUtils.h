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
    const FString SkeletonRef = TEXT("/AvatarSDKMetaperson2/Skeleton/Fbx/Fbx_Metaperson_Skeleton");
    const FString SmRef = TEXT("/AvatarSDKMetaperson2/Skeleton/Fbx/SM_Fbx_Metaperson");
protected:
    void SkeletalMeshSetRequiredBones(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> required_bones);
    void SkeletalMeshSetActiveBoneIndices(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> active_indices);
    void SkeletalMeshSetSoftVertices(USkeletalMesh* mesh, TArray<FSoftSkinVertex>& soft_vertices, int lod_index, int section_index);
    void SkeletalMeshSetSkeleton(USkeletalMesh* mesh, USkeleton* skeleton);
    void FixMesh(USkeletalMesh* mesh);
    void FixSkeletalMeshImportData(USkeletalMesh* mesh, USkeleton* oldSkeleton);
    void FixBonesInfluences(USkeletalMesh* Mesh, USkeleton* OldSkeleton);
    int32_t GetUpdatedBoneIndex(const USkeleton* old_skeleton, const USkeleton* new_skeleton, const TArray<uint16>& old_bone_map, TArray<uint16>& new_bone_map, int32_t index);

};