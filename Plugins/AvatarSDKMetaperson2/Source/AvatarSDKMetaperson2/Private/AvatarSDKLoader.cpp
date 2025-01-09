/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKLoader.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/Material.h"


void UAvatarSDKLoader::LoadAvatarAsync(const FString& GlbPath, USkeletalMeshComponent* InSkeletalMeshComponent, USkeletalMeshComponent* InFbxMeshComponent,  FOnAvatarLoaded AvatarLoadedDelegate)
{
	LoadSkeleton();
	SkeletalMeshComponent = InSkeletalMeshComponent;
	FbxMeshComponent = InFbxMeshComponent;
	OnAvatarLoaded = AvatarLoadedDelegate;

	FglTFRuntimeConfig Config;
	FRotator rot(180.0f, 0.0f, 90.0f);
	FVector tran(0.0f, 0.0f, 0.0f);
	FVector scale(-1.0f, 1.0f, 1.0f);

	FTransform transform(rot, tran, scale);
	Config.BaseTransform = transform;
	Config.TransformBaseType = EglTFRuntimeTransformBaseType::Transform;
	Config.SceneScale = 100.0f;

	GltfRuntimeAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(GlbPath, false, Config);
	if (!GltfRuntimeAsset)
	{
		UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKLoader: LoadAvatar: GltfRuntimeAsset is NULL"));
		return;
	}
	TArray<FString> ExcludeNodes;
	GlTFRuntimeSkeletalMeshDelegate.BindDynamic(this, &UAvatarSDKLoader::GltfRuntimeSkeletalMeshCallback);

	FglTFRuntimeSkeletonConfig SkeletonConfig;
	SkeletonConfig.bAddRootBone = true;
	
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
	SkeletalMeshConfig.MorphTargetsDuplicateStrategy = EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;
	SkeletalMeshConfig.SkeletonConfig = SkeletonConfig;	
	SkeletalMeshConfig.NormalsGenerationStrategy = EglTFRuntimeNormalsGenerationStrategy::IfMissing;
	SkeletalMeshConfig.Skeleton = Skeleton;
	

	SkeletalMeshConfig.MaterialsConfig = GetRuntimeMaterialsConfig();
	//SkeletalMeshConfig.SaveToPackage = TEXT("/Game/ExportedMesh");
	FglTFRuntimeImagesConfig ImagesConfig;

	GltfRuntimeAsset->LoadSkeletalMeshRecursiveAsync("", ExcludeNodes, GlTFRuntimeSkeletalMeshDelegate, SkeletalMeshConfig);
}

void UAvatarSDKLoader::GltfRuntimeSkeletalMeshCallback(USkeletalMesh* InSkeletalMesh)
{
	if (!IsValid(SkeletalMeshComponent)) {
		UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKLoader: GltfRuntimeSkeletalMeshCallback: SkeletalMeshComponent is not valid"));
	}
	if (!IsValid(InSkeletalMesh)) {
		UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKLoader: GltfRuntimeSkeletalMeshCallback: InSkeletalMesh is not valid"));
	}
	FbxMeshComponent->SetVisibility(false);
	SkeletalMeshComponent->SetSkeletalMesh(InSkeletalMesh);
	SkeletalMeshComponent->SetVisibility(true);
	if (OnAvatarLoaded.IsBound()) {
		OnAvatarLoaded.Broadcast("");
	}
}

void UAvatarSDKLoader::LoadSkeleton()
{
	if (!IsValid(Skeleton)) {
		Skeleton = Cast<USkeleton>(StaticLoadObject(USkeleton::StaticClass(), nullptr, *FString("/AvatarSDKMetaperson2/Skeleton/Metaperson2_Template_Skeleton.Metaperson2_Template_Skeleton")));
	}
	check(IsValid(Skeleton));
}

FglTFRuntimeMaterialsConfig UAvatarSDKLoader::GetRuntimeMaterialsConfig()
{
	MaterialsOverrideByNameMap.Empty();
	FglTFRuntimeMaterialsConfig Result;

	Result.bMaterialsOverrideMapInjectParams = true;
	FString HairMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/Hair/AvatarHair");
	auto HairMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *HairMaterialPath));
	check(IsValid(HairMaterial));
	MaterialsOverrideByNameMap.Add(TEXT("haircut"), HairMaterial);
	
	TArray<FString> AssoiciatedImages;
	
	auto alphaMode = GetMaterialProperty(GltfRuntimeAsset, "AvatarLeftCornea", "alphaMode");

	if (alphaMode == "BLEND") //recolor eyes
	{
		FString CorneaMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/Eyes/Cornea");
		auto CorneaMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *CorneaMaterialPath));
		check(IsValid(CorneaMaterial));
		MaterialsOverrideByNameMap.Add(TEXT("AvatarLeftCornea"), CorneaMaterial);
		MaterialsOverrideByNameMap.Add(TEXT("AvatarRightCornea"), CorneaMaterial);
	}
	
	Result.MaterialsOverrideByNameMap = MaterialsOverrideByNameMap;

	return Result;
}

FString UAvatarSDKLoader::GetMaterialProperty(UglTFRuntimeAsset* InGltfRuntimeAsset, const FString &MatName, const FString &PropName)
{
	TArray<FglTFRuntimePathItem> Path;
	FglTFRuntimePathItem PathElement0;
	FglTFRuntimePathItem PathElement1;
	FglTFRuntimePathItem PathElement2;
	FglTFRuntimePathItem PathElement3;

	PathElement0.Path = "materials";
	bool IsFound;
	Path.Add(PathElement0);

	if (!InGltfRuntimeAsset) {
		return "";
	}
	int ElementsNum = InGltfRuntimeAsset->GetArraySizeFromPath(Path, IsFound);
	if (!IsFound) {
		return "";
	}
 
	Path.Empty();
	PathElement1.Path = "materials";
	PathElement1.Index = 0;
	PathElement2.Path = "name";
	Path.Add(PathElement1);
	Path.Add(PathElement2);
	
	int MaterialIndex = -1;
	int MaterialPropertiesNum = -1;
	for (int ElementIdx = 0; ElementIdx < ElementsNum; ElementIdx++)
	{
		Path[0].Index = ElementIdx;
		auto MaterialStr = InGltfRuntimeAsset->GetStringFromPath(Path, IsFound);
		if (IsFound && MaterialStr.Contains(MatName)) {
			MaterialIndex = ElementIdx;
			Path.Empty();
			PathElement1.Path = "materials";
			PathElement1.Index = ElementIdx;
			Path.Add(PathElement1);
			PathElement2.Path = PropName;
			Path.Add(PathElement2);
			return InGltfRuntimeAsset->GetStringFromPath(Path, IsFound);
		}
	}
	return "";
}
