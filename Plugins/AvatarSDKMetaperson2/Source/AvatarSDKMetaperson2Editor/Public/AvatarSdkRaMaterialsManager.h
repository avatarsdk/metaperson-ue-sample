/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, November 2024
*/

#pragma once
#include "CoreMinimal.h"
#include "AvatarSdkRaMaterialsManager.generated.h"

UENUM()
enum class ETextureParameterType : uint8 {
	Color = 0,
	Roughness,
	Metallic,
	Normal,
	Scalp,

	HairAlpha,
	HairAO,
	HairColor,
	HairDepth,
	HairGltfMetallicRoughness,
	HairNormal,
	HairRoot,
	HairRoughness,
	HairShade,
	HairUniqueID,
	HairUnityMetallicSmoothness,
	HairScalp
};
USTRUCT()
struct FTextureParamsData {
	GENERATED_USTRUCT_BODY();
	UPROPERTY()
		TMap< ETextureParameterType, UTexture*> Textures;
	FString SlotName;
};

typedef TMap<FString, FTextureParamsData> MaterialTexturesData;
class UAvatarSdkRaAssetImporter;

UCLASS()
class UAvatarSdkRaMaterialsManager : public UObject {

	GENERATED_BODY()
protected:
	UPROPERTY()
	TMap<FString, UMaterialInterface*> Materials;
	TMap<ETextureParameterType, FString> MaterialParamsNames{
		{ETextureParameterType::Color, TEXT("Diffuse")},
		{ETextureParameterType::Metallic, TEXT("MetallMap")},
		{ETextureParameterType::Normal, TEXT("NormalMap")},
		{ETextureParameterType::Roughness, TEXT("RoughnessMap")},
		{ETextureParameterType::Scalp, TEXT("Scalp")},

		{ETextureParameterType::HairAlpha, TEXT("Alpha")},
		{ETextureParameterType::HairAO, TEXT("AmbientOcclusion")},
		{ETextureParameterType::HairColor, TEXT("Diffuse")},
		{ETextureParameterType::HairDepth, TEXT("Depth")},
		{ETextureParameterType::HairNormal, TEXT("NormalMap")},
		{ETextureParameterType::HairRoughness, TEXT("RoughnessMap")},
		{ETextureParameterType::HairScalp, TEXT("Scalp")},
		{ETextureParameterType::HairShade, TEXT("Shade")},
		{ETextureParameterType::HairUniqueID, TEXT("ID")},
		{ETextureParameterType::HairRoot, TEXT("Roots")},

	};
	void ImportTextures(TArray<FString> SrcTexturesPath, TArray<FString> DstTexturesPath);
public:
	UAvatarSdkRaMaterialsManager();
	void Initialize(bool bUseRtMaterials);
	const FString BodyMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/M_BodySkin_Alt.M_BodySkin_Alt");
	const FString HeadMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/M_HeadSkin_Alt.M_HeadSkin_Alt");
	const FString UpperOutfitMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/MI_rv_POLO.MI_rv_POLO");	
	const FString BottomOutfitMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/OutfitBaseMaterial.OutfitBaseMaterial");
	const FString ShoesMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/OutfitBaseMaterial.OutfitBaseMaterial");	
	const FString RtHairMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/M_HairBase02.M_HairBase02");
	const FString HairMaterialRef = TEXT("/AvatarSDKMetaperson2/Materials/M_HairBase03.M_HairBase03");	
protected:
	MaterialTexturesData ExtractTextureData(USkeletalMesh* Mesh);
public:
	void SetMaterialsToMesh(USkeletalMesh* Mesh, const FString& DstDir);
	/* Try to find scalp.png in the Mesh's directory. If found, import it to DstDir and write the inforation about it to the Data*/
	void ReadScalpTexture(FTextureParamsData& Data, USkeletalMesh* Mesh, const FString& DstDir);
protected:
	void AddTexture(FTextureParamsData& Data, const FString& Name, const FString& SrcPath, const FString& DstDirectory, ETextureParameterType Type);
	TSet<FString>GetHairTextures(const FString& Directory, FTextureParamsData& Data, const FString& DstDirectory);
	bool FindTexture(FTextureParamsData& Data, const FString& PathToTxt, const FString& ImportDstDir, ETextureParameterType TextureType);
};
