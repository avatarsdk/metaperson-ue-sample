/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, November 2024
*/

#include "AvatarSdkRaMaterialsManager.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "AssetToolsModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Materials/MaterialInstanceConstant.h"
#include "HAL/FileManagerGeneric.h"
#include <Factories/TextureFactory.h>
#include "AssetRegistry/AssetRegistryModule.h"

#include "UObject/SavePackage.h"

void UAvatarSdkRaMaterialsManager::ImportTextures(TArray<FString> SrcTexturesPath, TArray<FString> DstTexturesPath)
{
	check(SrcTexturesPath.Num() == DstTexturesPath.Num());
	auto AutomatedData = NewObject<UAutomatedAssetImportData>();
	AutomatedData->bReplaceExisting = false;
	auto TextureFactory = NewObject<UTextureFactory>();
	TextureFactory->NoCompression = true;
	TextureFactory->AutomatedImportData = AutomatedData;
	FScopedSlowTask ImportTask(SrcTexturesPath.Num(), FText::FromString("Importing Textures"));
	ImportTask.MakeDialog(true);
	auto ActorIdx = -1;
	for(int i = 0; i < SrcTexturesPath.Num(); i++)
	{
		auto tx = SrcTexturesPath[i];
		ActorIdx++;
		FString TexGamePath, TexName;
		tx.Split(TEXT("/"), &TexGamePath, &TexName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);		
		FString PathForTextures = DstTexturesPath[i];
		auto TexPackage = CreatePackage(*PathForTextures);
		TexPackage->FullyLoad();
		auto bCancelled = false;
		auto NewTxName = TexName.Replace(TEXT(".jpg"), TEXT("")).Replace(TEXT(".png"), TEXT(""));
		auto CreatedTexture = TextureFactory->FactoryCreateFile(UTexture2D::StaticClass(), TexPackage, FName(*NewTxName), RF_Public | RF_Standalone, tx, NULL, GWarn, bCancelled);
		if (CreatedTexture == nullptr)
		{
			continue;
		}
		auto Tex = CastChecked<UTexture2D>(CreatedTexture);
		/// tx 
		auto CompressionSetting = Tex->CompressionSettings;
		if (NewTxName.EndsWith("MRA") && CompressionSetting != TC_Masks)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Masks;
		}
		if (NewTxName.EndsWith("NM") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Normalmap;
		}
		if (NewTxName.EndsWith("DF") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = true;
			Tex->CompressionSettings = TC_Default;
		}
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Importing Texture : %d of %d: %s"), ActorIdx + 1, SrcTexturesPath.Num() + 1, *NewTxName));
		ImportTask.EnterProgressFrame();

		Tex->UpdateResource();
		FAssetRegistryModule::AssetCreated(Tex);
		auto LongPackageName = TexPackage->GetName();
		auto Extension = FPackageName::GetAssetPackageExtension();
		const FString PackageFileName = FPackageName::LongPackageNameToFilename(LongPackageName, Extension);
		FSavePackageArgs SaveArgs;
		UPackage::SavePackage(TexPackage, Tex, *PackageFileName, SaveArgs);
	}
}

UAvatarSdkRaMaterialsManager::UAvatarSdkRaMaterialsManager()
{
	
}

void UAvatarSdkRaMaterialsManager::Initialize(bool bUseRtMaterials)
{
	Materials.Add(TEXT("outfit_top"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *UpperOutfitMaterialRef)));
	Materials.Add(TEXT("outfit"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *UpperOutfitMaterialRef)));
	Materials.Add(TEXT("AvatarHead"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *HeadMaterialRef)));
	Materials.Add(TEXT("AvatarBody"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *BodyMaterialRef)));
	Materials.Add(TEXT("outfit_shoes"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *ShoesMaterialRef)));
	Materials.Add(TEXT("outfit_bottom"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *BottomOutfitMaterialRef)));

	if (bUseRtMaterials) {
		Materials.Add(TEXT("haircut"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *RtHairMaterialRef)));
	}
	else {
		Materials.Add(TEXT("haircut"), Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *HairMaterialRef)));
	}
}
void UAvatarSdkRaMaterialsManager::ReadScalpTexture(FTextureParamsData& Data, USkeletalMesh* Mesh, const FString& DstDir)
{
	auto MeshSourceFile = Mesh->AssetImportData->SourceData.SourceFiles[0].RelativeFilename;
	auto MeshSourceDir = FPaths::GetPath(MeshSourceFile);
	auto ScalpTexturePath = FPaths::Combine(MeshSourceDir, TEXT("scalp.png"));
	if (FPaths::FileExists(ScalpTexturePath)) {
		auto ScalpTextureAsset = DstDir / TEXT("Scalp");
		ImportTextures({ ScalpTexturePath }, { ScalpTextureAsset });
		auto ScalpTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *ScalpTextureAsset));
		if (ScalpTexture) {
			Data.Textures.Add(ETextureParameterType::Scalp, ScalpTexture);			
		}
	}
}

bool  UAvatarSdkRaMaterialsManager::FindTexture(FTextureParamsData& Data, const FString& DirectoryToSearch, const FString& TextureName, const FString& ImportDstDir, ETextureParameterType TextureType) {

	
	
	TArray<FString> PathVariants;
	PathVariants.Add(FPaths::Combine(DirectoryToSearch, TextureName.Replace(TEXT(".jpg"), TEXT(".png"))));
	PathVariants.Add(FPaths::Combine(DirectoryToSearch, TextureName.Replace(TEXT(".png"), TEXT(".jpg"))));
	PathVariants.Add(FPaths::Combine(DirectoryToSearch, GetTextureNameWithoutPrefix(TextureName).Replace(TEXT(".jpg"), TEXT(".png"))));
	PathVariants.Add(FPaths::Combine(DirectoryToSearch, GetTextureNameWithoutPrefix(TextureName).Replace(TEXT(".png"), TEXT(".jpg"))));

	for (const auto& PathToTxt : PathVariants) {
		if (FPaths::FileExists(PathToTxt))
		{
			FString BaseName = FPaths::GetBaseFilename(PathToTxt);
			FString TxtDst = FPaths::Combine(ImportDstDir, BaseName);
			UE_LOG(LogTemp, Log, TEXT("ExtractTextureData: Found texture %s"), *PathToTxt);
			ImportTextures({ PathToTxt }, { TxtDst });
			auto Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *TxtDst));
			if (Texture) {
				Data.Textures.Add(TextureType, Texture);
				return true;
			}
		}
	}
	return false;
}

FString UAvatarSdkRaMaterialsManager::GetTextureNameWithoutPrefix(const FString& TxtName) {
	auto Result = TxtName;
	if (!FChar::IsUpper(Result[0])) {
		int32 Idx;		
		if (Result.FindChar('_', Idx)) {
            Result = Result.RightChop(Idx + 1);
        }
	}
	return Result;

}


MaterialTexturesData UAvatarSdkRaMaterialsManager::ExtractTextureData(USkeletalMesh* Mesh)
{
	TArray<FSkeletalMaterial>MeshMaterials = Mesh->GetMaterials();
	MaterialTexturesData Result;

	for (int i = 0; i < MeshMaterials.Num(); i++) {
		TArray<UTexture*> Textures;
		FTextureParamsData Data;
		Data.SlotName = MeshMaterials[i].MaterialSlotName.ToString();
		MeshMaterials[i].MaterialInterface->GetUsedTextures(Textures, EMaterialQualityLevel::Epic, 1, ERHIFeatureLevel::SM6, true);
		for (int j = 0; j < Textures.Num(); j++) {
			FString TextureName = Textures[j]->GetName();
			UE_LOG(LogTemp, Log, TEXT("ExtractTextureData: Handling Texture %s"), *TextureName);

			if (TextureName.Contains(TEXT("Normal"))) {
				Data.Textures.Add(ETextureParameterType::Normal, Textures[j]);
			}
			else if (TextureName.Contains(TEXT("Color"))) {
				Data.Textures.Add(ETextureParameterType::Color, Textures[j]);	

				FString PathToTxt = Textures[j]->AssetImportData->SourceData.SourceFiles[0].RelativeFilename;
				FString ImportDstDir = FPaths::GetPath(Textures[j]->AssetImportData.GetPath());
				if (Data.SlotName == TEXT("AvatarHead")) {
					//Scalp is not mentioned among fbx materials textures, but placed in the same directory as mesh
					ReadScalpTexture(Data, Mesh, ImportDstDir);
				}
				
				//Roughnesss is not mentioned among fbx materials textures, but placed in the same directory as other textures				
				{	
					auto BaseFileName = FPaths::GetCleanFilename(PathToTxt);
					TArray<FString> PathsToTry;
					auto PathToRough = BaseFileName.Replace(TEXT("Color"), TEXT("Roughness"));
					UE_LOG(LogTemp, Log, TEXT("ExtractTextureData: Trying to import roughness texture %s"), *PathToRough);
					if (FindTexture(Data, FPaths::GetPath(PathToTxt), PathToRough, ImportDstDir, ETextureParameterType::Roughness)) {
						UE_LOG(LogTemp, Warning, TEXT("ExtractTextureData: Roughness texture found! %s"), *PathToRough);
					} else {
						UE_LOG(LogTemp, Warning, TEXT("ExtractTextureData: Roughness texture not found %s"), *PathToRough);
					}
				}
				
                //Metallic is not mentioned among fbx materials textures, but placed in the same directory as other textures
                {
					auto BaseFileName = FPaths::GetCleanFilename(PathToTxt);
					
					auto PathToMetallic = BaseFileName.Replace(TEXT("Color"), TEXT("Metallic"));
                    auto PathToMetallicSmoothness = FPaths::Combine(FPaths::GetPath(PathToTxt), BaseFileName.Replace(TEXT("Color"), TEXT("UnityMetallicSmoothness")));

                    UE_LOG(LogTemp, Log, TEXT("ExtractTextureData: Trying to import metallic texture %s"), *PathToMetallic);
                    if (FindTexture(Data, FPaths::GetPath(PathToTxt),  PathToMetallic, ImportDstDir, ETextureParameterType::Metallic)) {
                        UE_LOG(LogTemp, Warning, TEXT("ExtractTextureData: Metallic texture found! %s"), *PathToMetallic);
                        break;
                    } else if (FindTexture(Data, FPaths::GetPath(PathToTxt), PathToMetallicSmoothness, ImportDstDir, ETextureParameterType::Metallic)) {
						UE_LOG(LogTemp, Warning, TEXT("ExtractTextureData: Metallic texture found! %s"), *PathToMetallicSmoothness);
					} else {
                        UE_LOG(LogTemp, Warning, TEXT("ExtractTextureData: Metallic texture not found %s"), *PathToMetallic);
                    }
                }
				
				if (Data.SlotName == TEXT("haircut")) {
					FString PathToTxtDir = FPaths::GetPath(PathToTxt);
					TSet<FString> HairTextures = GetHairTextures(PathToTxtDir, Data, FPaths::GetPath(Textures[j]->AssetImportData.GetPath()));
				}
			}
			else if (TextureName.Contains(TEXT("Roughness"))) {
				Data.Textures.Add(ETextureParameterType::Roughness, Textures[j]);
			}
			else if (TextureName.Contains(TEXT("Metallic"))) {
				Data.Textures.Add(ETextureParameterType::Metallic, Textures[j]);
			}
		}
		check(!Result.Contains(Data.SlotName));
		Result.Add(Data.SlotName, Data);
	}
	return Result;
}

void UAvatarSdkRaMaterialsManager::SetMaterialsToMesh(USkeletalMesh* Mesh, const FString& DstDir)
{
	auto TextureData = ExtractTextureData(Mesh);
	auto MaterialsToSet = Mesh->GetMaterials();

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	for (FSkeletalMaterial& Mat : MaterialsToSet) {
		FTextureParamsData* TextureParamsData = TextureData.Find(Mat.MaterialSlotName.ToString());
		UMaterialInterface** ParentMaterialPtr = Materials.Find(Mat.MaterialSlotName.ToString());
		if (TextureParamsData && ParentMaterialPtr) {
			UMaterialInterface* ParentMaterial = *ParentMaterialPtr;
			FString InstanceName = TEXT("MI_") + Mat.MaterialSlotName.ToString();
			FString PackageName = FPaths::Combine(DstDir, InstanceName);
			UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			Factory->InitialParent = ParentMaterial;
			auto MaterialInstance = CastChecked<UMaterialInstanceConstant>(
				AssetToolsModule.Get().CreateAsset(InstanceName,
					FPackageName::GetLongPackagePath(PackageName),
					UMaterialInstanceConstant::StaticClass(),
					Factory));
			for (auto Texture : TextureParamsData->Textures) {
				auto TextureParamName = FName(MaterialParamsNames[Texture.Key]);
				UTexture* TextureOrigin;
				MaterialInstance->GetTextureParameterValue(TextureParamName, TextureOrigin);
				Texture.Value->PreEditChange(nullptr);
				if (TextureOrigin) {
					Texture.Value->SRGB = TextureOrigin->SRGB;
					Texture.Value->CompressionSettings = TextureOrigin->CompressionSettings;
				}
				
				Texture.Value->PostEditChange();
				
				MaterialInstance->SetTextureParameterValueEditorOnly(TextureParamName, Texture.Value);
			}
			Mat.MaterialInterface = MaterialInstance;
		}
	}
	Mesh->SetMaterials(MaterialsToSet);
	Mesh->Modify();
	Mesh->PostEditChange();
}

void UAvatarSdkRaMaterialsManager::AddTexture(FTextureParamsData& Data, const FString& Name, const FString& SrcPath, const FString& DstDirectory, ETextureParameterType Type)
{
	if (Data.Textures.Contains(Type)) { return; }
	auto TextDst = FPaths::Combine(DstDirectory, Name);
	ImportTextures({ SrcPath }, { TextDst });
	auto Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *TextDst));
	if (Texture) {
		Data.Textures.Add(Type, Texture);
	}	
}

TSet<FString> UAvatarSdkRaMaterialsManager::GetHairTextures(const FString& Directory, FTextureParamsData& Data, const FString& DstDirectory)
{
	TSet<FString> Result;
	if (!FPaths::DirectoryExists(Directory))
	{
		return Result;
	}

	TArray<FString> FoundFiles;
	FFileManagerGeneric::Get().FindFiles(FoundFiles, *Directory, TEXT(".png"));
	auto Filtered = FoundFiles.FilterByPredicate([](const FString& Str) {
		return Str.Contains(TEXT("Hair"));
		});
	bool bImportSuccess = false;
	for (auto& Txt : Filtered) {
		auto TxtNoExt = FPaths::GetBaseFilename(Txt);
		if (Txt.Contains(TEXT("Alpha"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairAlpha);
		}
		else if (Txt.Contains(TEXT("AO"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairAO);
		}
		else if (Txt.Contains(TEXT("Color"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairColor);
		}
		else if (Txt.Contains(TEXT("Depth"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairDepth);
		}
		else if (Txt.Contains(TEXT("GltfMetallicRoughness"))) {
		}
		else if (Txt.Contains(TEXT("Normal"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairNormal);
		}
		else if (Txt.Contains(TEXT("Root"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairRoot);
		}
		else if (Txt.Contains(TEXT("Roughness"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairRoughness);
		}
		else if (Txt.Contains(TEXT("Shade"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairShade);
		}
		else if (Txt.Contains(TEXT("UniqueID"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairUniqueID);
		}
		else if (Txt.Contains(TEXT("UnityMetallicSmoothness"))) {
		}
		else if (Txt.Contains(TEXT("Scalp"))) {
			AddTexture(Data, TxtNoExt, FPaths::Combine(Directory, Txt), DstDirectory, ETextureParameterType::HairScalp);
		}
	}
	Result = TSet <FString>(FoundFiles);
	return Result;


}
