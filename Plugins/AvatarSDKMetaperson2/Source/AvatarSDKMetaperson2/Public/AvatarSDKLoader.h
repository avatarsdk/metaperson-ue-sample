/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#pragma once

#include "CoreMinimal.h"
#include "AvatarSDKData.h"
#include "AvatarSDKMetaperson2.h"
#include "glTFRuntimeParser.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "AvatarSDKLoader.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarLoaded, const FString&, Url);


UCLASS()
class AVATARSDKMETAPERSON2_API UAvatarSDKLoader : public UObject
{
	GENERATED_BODY()
public:
	void LoadAvatarAsync(const FString& GlbPath, USkeletalMeshComponent* InSkeletalMeshComponent, USkeletalMeshComponent* InFbxMeshComponent, FOnAvatarLoaded AvatarLoadedDelegate);
	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoaded OnAvatarLoaded;
protected:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY()
	USkeletalMeshComponent* FbxMeshComponent;
	UPROPERTY()
	class UglTFRuntimeAsset* GltfRuntimeAsset;
	FglTFRuntimeSkeletalMeshAsync GlTFRuntimeSkeletalMeshDelegate;
	UFUNCTION()
	void GltfRuntimeSkeletalMeshCallback(USkeletalMesh* InSkeletalMesh);
	UFUNCTION()
	void LoadSkeleton();
	UPROPERTY()
	USkeleton* Skeleton;
protected:
	FglTFRuntimeMaterialsConfig GetRuntimeMaterialsConfig();
	FString GetMaterialProperty(UglTFRuntimeAsset* InGltfRuntimeAsset, const FString& MatName, const FString& PropName);


	UPROPERTY()
	TMap<FString, UMaterialInterface*> MaterialsOverrideByNameMap;
};