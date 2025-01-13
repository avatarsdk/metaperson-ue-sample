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
#include "Engine/SkeletalMesh.h" 
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "AvatarSDKMetaperson2ImportUtils.generated.h"

UCLASS()
class UAvatarSDKMetaperson2ImportUtils : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    USkeleton* Skeleton;
    USkeleton* GetSkeleton() {
        if (!IsValid(Skeleton)) {
            
            Skeleton = LoadObject<USkeleton>(NULL, *SkeletonRef, NULL, LOAD_None, NULL);
            
        }
        check(IsValid(Skeleton));
        return Skeleton;
    }
    USkeletalMesh* GetReferenseSM() {
        if (!IsValid(ReferenseSM)) {
            ReferenseSM = Cast<USkeletalMesh>(StaticLoadObject(USkeleton::StaticClass(), nullptr, *SmRef));
        }
        check(IsValid(ReferenseSM));
        return ReferenseSM;
    }
public:
    UPROPERTY()
    USkeletalMesh* ReferenseSM;
    UAvatarSDKMetaperson2ImportUtils() {
        static ConstructorHelpers::FObjectFinder<USkeleton> SkeletonObj(*SkeletonRef);

        Skeleton = SkeletonObj.Object;
        ReferenseSM = nullptr;
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
    void SkeletalMeshSetRequiredBones(USkeletalMesh* Mesh, int32 LODIndex, TArray<FBoneIndexType> RequiredBones);
    void SkeletalMeshSetActiveBoneIndices(USkeletalMesh* Mesh, int32 LODIndex, TArray<FBoneIndexType> ActiveIndices);
    void SkeletalMeshSetSoftVertices(USkeletalMesh* Mesh, TArray<FSoftSkinVertex>& SoftVertices, int LODIndex, int SectionIndex);
    void SkeletalMeshSetSkeleton(USkeletalMesh* Mesh, USkeleton* Skeleton);
    void FixMesh(USkeletalMesh* Mesh);
    void FixSkeletalMeshImportData(USkeletalMesh* Mesh, USkeleton* OldSkeleton);
    void FixBonesInfluences(USkeletalMesh* Mesh, USkeleton* OldSkeleton);
    int32_t GetUpdatedBoneIndex(const USkeleton* asold_skeleton, const USkeleton* new_skeleton, const TArray<uint16>& old_bone_map, TArray<uint16>& new_bone_map, int32_t index);

};