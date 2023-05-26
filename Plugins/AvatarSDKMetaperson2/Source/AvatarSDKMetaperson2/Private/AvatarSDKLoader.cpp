#include "AvatarSDKLoader.h"


void UAvatarSDKLoader::LoadAvatarAsync(const FString& GlbPath, USkeletalMeshComponent* InSkeletalMeshComponent, FOnAvatarLoaded AvatarLoadedDelegate)
{
	LoadSkeleton();
	SkeletalMeshComponent = InSkeletalMeshComponent;
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
	auto NumImages = GltfRuntimeAsset->GetNumImages();
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
	SkeletalMeshConfig.Skeleton = Skeleton;
	
	SkeletalMeshConfig.MaterialsConfig = GetRuntimeMaterialsConfig();
	//SkeletalMeshConfig.SaveToPackage = TEXT("/Game/Ext");
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
	SkeletalMeshComponent->SetSkeletalMesh(InSkeletalMesh);
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
	FString HairMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/AvatarHair");
	auto HairMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *HairMaterialPath));
	MaterialsOverrideByNameMap.Add(TEXT("haircut"), HairMaterial);
	Result.MaterialsOverrideByNameMap = MaterialsOverrideByNameMap;

	return Result;
	/*FString CorneaMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/CorneaBaseMaterial");
	auto CorneaMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *CorneaMaterialPath));

	MaterialsOverrideByNameMap.Add(TEXT("AvatarLeftCornea"), CorneaMaterial);
	MaterialsOverrideByNameMap.Add(TEXT("AvatarRightCornea"), CorneaMaterial);

	FString EyelashesMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/EyelashesBaseMaterial");
	MaterialsOverrideByNameMap.Add(TEXT("AvatarEyelashes"), Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *EyelashesMaterialPath)));

	FString EyesMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/EyeBaseMaterial");
	auto EyeBallMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *EyesMaterialPath));

	MaterialsOverrideByNameMap.Add(TEXT("AvatarLeftEyeball"), EyeBallMaterial);
	MaterialsOverrideByNameMap.Add(TEXT("AvatarRightEyeball"), EyeBallMaterial);

	FString TeethMaterialPath = TEXT("/AvatarSDKMetaperson2/Materials/TeethBaseMaterial");
	auto TeethMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *TeethMaterialPath));

	MaterialsOverrideByNameMap.Add(TEXT("AvatarTeethLower"), TeethMaterial);
	MaterialsOverrideByNameMap.Add(TEXT("AvatarTeethUpper"), TeethMaterial);
	
	Result.MaterialsOverrideByNameMap = MaterialsOverrideByNameMap;
	return Result;*/
}
