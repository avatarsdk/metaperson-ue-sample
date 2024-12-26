#include "AvatarSDKMetaperson2ImportUtils.h"
#include "AssetImportTask.h" 
#include <AssetToolsModule.h>
#include "Factories/FbxImportUI.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include <glTFRuntimeFunctionLibrary.h>
#include "AssetRegistry/AssetRegistryModule.h"
#include "IMeshBuilderModule.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AvatarSdkRaMaterialsManager.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
#include "Engine/SkinnedAssetCommon.h"
#endif
#include "LODUtilities.h"
#include <MeshUtilitiesCommon.h>
#include <Factories/FbxFactory.h>
#include <ObjectTools.h>

void UAvatarSDKMetaperson2ImportUtils::FixSkeletalMeshImportData(USkeletalMesh* Mesh, USkeleton* OldSkeleton)
{
    FSkeletalMeshImportData ImportData;
    Mesh->LoadLODImportedData(0, ImportData);

    SkeletalMeshImportData::FBone NewRootBone;
    NewRootBone.Name = TEXT("root");
    NewRootBone.ParentIndex = INDEX_NONE; // No parent, it's the root
    NewRootBone.NumChildren = ImportData.RefBonesBinary.Num();
    NewRootBone.Flags = 0;
    NewRootBone.BonePos.Transform.SetIdentity();

    ImportData.RefBonesBinary.Insert(NewRootBone, 0);

    for (int i = 1; i < ImportData.RefBonesBinary.Num(); i++)
    {
        auto OldBoneIndex = ImportData.RefBonesBinary[i].ParentIndex;

        if (OldBoneIndex == INDEX_NONE)
        {
            ImportData.RefBonesBinary[i].ParentIndex = 0;
        }
        else
        {
            auto OldBoneName = OldSkeleton->GetReferenceSkeleton().GetBoneName(OldBoneIndex);
            auto NewBoneIndex = Mesh->Skeleton->GetReferenceSkeleton().FindBoneIndex(OldBoneName);
            ImportData.RefBonesBinary[i].ParentIndex = NewBoneIndex;
        }
    }

    USkeleton* newSkeleton = Mesh->Skeleton;

    for (int i = 0; i < ImportData.Influences.Num(); i++)
    {
        auto OldBoneIndex = ImportData.Influences[i].BoneIndex;
        auto OldBoneName = OldSkeleton->GetReferenceSkeleton().GetBoneName(OldBoneIndex);
        auto NewBoneIndex = newSkeleton->GetReferenceSkeleton().FindBoneIndex(OldBoneName);
        ImportData.Influences[i].BoneIndex = NewBoneIndex;
    }
    Mesh->SaveLODImportedData(0, ImportData);
}


void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetSoftVertices(USkeletalMesh* Mesh, TArray<FSoftSkinVertex>& SoftVertices, int LODIndex, int SectionIndex)
{
    FSkeletalMeshModel* Resource = Mesh->GetImportedModel();

    FSkeletalMeshLODModel& Model = Resource->LODModels[LODIndex];

    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;
    Mesh->PreEditChange(nullptr);
    Mesh->ReleaseResources();
    Mesh->ReleaseResourcesFence.Wait();

    Model.Sections[SectionIndex].SoftVertices = SoftVertices;
    Model.Sections[SectionIndex].NumVertices = SoftVertices.Num();
    Model.Sections[SectionIndex].CalcMaxBoneInfluences();

    Mesh->RefBasesInvMatrix.Empty();
    Mesh->CalculateInvRefMatrices();
    Mesh->Build();
    Mesh->AssetImportData = nullptr;

#if WITH_EDITOR
    Mesh->PostEditChange();
#endif

    Mesh->InitResources();
    Mesh->MarkPackageDirty();

}

UAssetImportTask* UAvatarSDKMetaperson2ImportUtils::CreateImportTask(const FString& SrcPath, const FString& DstPath, UFactory* ExtraFactory, UObject* ExtraOptions, bool& bOutSuccess)
{
    UAssetImportTask* Task = NewObject<UAssetImportTask>();
    if (Task == nullptr) {
        bOutSuccess = false;
        return nullptr;
    }
    Task->Filename = SrcPath;
    Task->DestinationPath = FPaths::GetPath(DstPath);
    Task->DestinationName = FPaths::GetCleanFilename(DstPath);

    Task->bSave = true;
    Task->bAutomated = true;
    Task->bAsync = false;
    Task->bReplaceExisting = true;
    Task->bReplaceExistingSettings = false;

    Task->Factory = ExtraFactory;
    Task->Options = ExtraOptions;

    bOutSuccess = true;
    return Task;
}

UObject* UAvatarSDKMetaperson2ImportUtils::ProcessImportTask(UAssetImportTask* InTask, bool& bOutSuccess)
{
    if (!InTask) {
        bOutSuccess = false;
        return nullptr;
    }
    FAssetToolsModule* AssetTools = FModuleManager::LoadModulePtr< FAssetToolsModule>("AssetTools");
    if (!AssetTools) {
        bOutSuccess = false;
        return nullptr;
    }
    AssetTools->Get().ImportAssetTasks({ InTask });
    if (InTask->GetObjects().Num() == 0) {
        bOutSuccess = false;
        return nullptr;
    }
    UObject* ImportedAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *FPaths::Combine(InTask->DestinationPath, InTask->DestinationName));
    bOutSuccess = true;

    return ImportedAsset;
}


void SkeletonAddBone(USkeleton* skeleton,  char* name, int parent_index, FTransform transform)
{
    if (!skeleton)
        return;//PyErr_Format(PyExc_Exception, "uobject is not a USkeleton");
      
    if (skeleton->GetReferenceSkeleton().FindBoneIndex(FName(UTF8_TO_TCHAR(name))) > -1)
    {
        return;
    }

#if WITH_EDITOR
    skeleton->PreEditChange(nullptr);
#endif

    {
        const FReferenceSkeleton& ref = skeleton->GetReferenceSkeleton();
        // horrible hack to modify the skeleton in place
        FReferenceSkeletonModifier modifier((FReferenceSkeleton&)ref, skeleton);

        TCHAR* bone_name = UTF8_TO_TCHAR(name);

        modifier.Add(FMeshBoneInfo(FName(bone_name), FString(bone_name), parent_index), transform);
    }

    skeleton->PostEditChange();
    skeleton->MarkPackageDirty();
}

void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetRequiredBones(USkeletalMesh* Mesh, int32 LODIndex, TArray<FBoneIndexType> RequiredBones)
{
    if (!Mesh)
        return;

    FSkeletalMeshModel* Resource = Mesh->GetImportedModel();

    if (LODIndex < 0 || LODIndex >= Resource->LODModels.Num())
        return;

    FSkeletalMeshLODModel& Model = Resource->LODModels[LODIndex];

    // Temporarily disable all USkinnedMeshComponent's
    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

    Mesh->ReleaseResources();
    Mesh->ReleaseResourcesFence.Wait();

    Model.RequiredBones = RequiredBones;
    Model.RequiredBones.Sort();

    Mesh->RefBasesInvMatrix.Empty();
    Mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    Mesh->PostEditChange();
#endif

    Mesh->InitResources();
    Mesh->MarkPackageDirty();
}

void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetActiveBoneIndices(USkeletalMesh* Mesh, int32 LODIndex, TArray<FBoneIndexType> ActiveIndices)
{
    FSkeletalMeshModel* Resource = Mesh->GetImportedModel();

    if (LODIndex < 0 || LODIndex >= Resource->LODModels.Num())
        return;

    FSkeletalMeshLODModel& Model = Resource->LODModels[LODIndex];

    // Temporarily disable all USkinnedMeshComponent's
    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

    Mesh->ReleaseResources();
    Mesh->ReleaseResourcesFence.Wait();

    Model.ActiveBoneIndices = ActiveIndices;
    Model.ActiveBoneIndices.Sort();

    Mesh->RefBasesInvMatrix.Empty();
    Mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    Mesh->PostEditChange();
#endif

    Mesh->InitResources();
    Mesh->MarkPackageDirty();
}

int32_t UAvatarSDKMetaperson2ImportUtils::GetUpdatedBoneIndex(const USkeleton* OldSkeleton, const USkeleton* NewSkeleton, const TArray<uint16>& OldBoneMap, TArray<uint16>& NewBoneMap, int32_t Index)
{
    // Get the skeleton bone ID from the map
    int32_t TrueBoneID = OldBoneMap[Index];

    // Get the bone name
    FName BoneName = OldSkeleton->GetReferenceSkeleton().GetBoneName(Index);

    // Get the new index
    int32_t NewBoneID = NewSkeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);

    // Check if a new mapping is available
    if (NewBoneMap.Contains(NewBoneID))
    {
        return NewBoneMap.IndexOfByKey(NewBoneID);
    }

    NewBoneMap.Add(NewBoneID);
    return NewBoneMap.Num() - 1;
}


void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetSkeleton(USkeletalMesh* Mesh, USkeleton* InSkeleton)
{
    Mesh->ReleaseResources();
    Mesh->ReleaseResourcesFence.Wait();

    Mesh->Skeleton = InSkeleton;
    Mesh->RefSkeleton = InSkeleton->GetReferenceSkeleton();
    Mesh->RefBasesInvMatrix.Empty();
    Mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    Mesh->PostEditChange();
#endif

    Mesh->InitResources();
    Mesh->MarkPackageDirty();
}

void UAvatarSDKMetaperson2ImportUtils::FixBonesInfluences(USkeletalMesh* Mesh, USkeleton* OldSkeleton)
{
    if (!Mesh || !OldSkeleton)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Mesh or OldSkeleton"));
        return;
    }

    TArray<uint16> ActiveBones;

    // Iterate through each section of the skeletal mesh
    for (int32 SectionIndex = 0; SectionIndex < Mesh->GetImportedModel()->LODModels[0].Sections.Num(); ++SectionIndex)
    {
        // Get the vertices for the current section
        TArray<FSoftSkinVertex> Vertices = Mesh->GetImportedModel()->LODModels[0].Sections[SectionIndex].SoftVertices;
        //UE_LOG(LogTemp, Warning, TEXT("Number of vertices: %d"), Vertices.Num());

        TArray<FSoftSkinVertex> NewVertices;
        //Mesh->GetResourceForRendering()->LODRenderData[0].RenderSections[SectionIndex].BoneMap;
        TArray<uint16> OldBoneMap = Mesh->GetImportedModel()->LODModels[0].Sections[SectionIndex].BoneMap;//Mesh->GetResourceForRendering()->LODRenderData[0].RenderSections[SectionIndex].BoneMap;
       
        TArray<uint16> NewBoneMap;
        NewBoneMap.Reserve(OldBoneMap.Num()+1);
        
        

        //for (FSoftSkinVertex& Vertex : Vertices)
        for(int i = 0; i < Vertices.Num(); i++)
        {
            auto & Vertex = Vertices[i];
            TArray<int32> BoneIDs;
            for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex)
            {
                if (Vertex.InfluenceWeights[InfluenceIndex] > 0)
                {
                    int32 BoneID = Vertex.InfluenceBones[InfluenceIndex];
                    int32 NewBoneID = GetUpdatedBoneIndex(OldSkeleton, Mesh->Skeleton, OldBoneMap, NewBoneMap, BoneID);
                    BoneIDs.Add(NewBoneID);
        
                    if (!ActiveBones.Contains(NewBoneMap[NewBoneID]))
                    {
                        ActiveBones.Add(NewBoneMap[NewBoneID]);
                    }
                }
                else
                {
                    BoneIDs.Add(INDEX_NONE);
                }
            }

            for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex) {
                if (Vertex.InfluenceWeights[InfluenceIndex] > 0) {
                    Vertex.InfluenceBones[InfluenceIndex] = BoneIDs[InfluenceIndex];
                    //Vertex.InfluenceWeights[InfluenceIndex] = 0;
                }
            }
            
            NewVertices.Add(Vertex);
        }

        // Assign new vertices
        //Mesh->GetImportedModel()->LODModels[0].Sections[SectionIndex].SoftVertices = NewVertices;
        SkeletalMeshSetSoftVertices(Mesh, NewVertices, 0, SectionIndex);

        // Add the new bone mapping
        //Mesh->GetImportedModel()->LODModels[0].Sections[SectionIndex].BoneMap = NewBoneMap;        
        SkeletalMeshSetRequiredBones(Mesh, 0, NewBoneMap);
    }

    // Specify which bones are active and required (ensure root is added to required bones)
    //Mesh->GetImportedModel()->LODModels[0].ActiveBoneIndices = ActiveBones;
    SkeletalMeshSetActiveBoneIndices(Mesh, 0, ActiveBones);

    // Mark all the bones as required (eventually you can be more selective)
    TArray<uint16> RequiredBones;
    for (int32 BoneIndex = 0; BoneIndex < Mesh->Skeleton->GetReferenceSkeleton().GetNum(); ++BoneIndex)
    {
        RequiredBones.Add(BoneIndex);
    }
    //Mesh->GetImportedModel()->LODModels[0].RequiredBones = RequiredBones;
    SkeletalMeshSetRequiredBones(Mesh, 0, RequiredBones);

    // Rebuild the skeletal mesh
    Mesh->PostEditChange();
    Mesh->MarkPackageDirty();
}

 
void UAvatarSDKMetaperson2ImportUtils::FixMesh(USkeletalMesh* mesh) {
    USkeleton* OldSkeleton = mesh->GetSkeleton();
    SkeletalMeshSetSkeleton(mesh, Skeleton);
    FixSkeletalMeshImportData(mesh, OldSkeleton);
    FixBonesInfluences(mesh, OldSkeleton);
}

USkeletalMesh* UAvatarSDKMetaperson2ImportUtils::ImportSkeletalMesh(FString& SrcPath, FString& DstPath, bool& bOutSuccess, bool bRtMaterilas)
{
    UFbxImportUI* Options = NewObject<UFbxImportUI>();
    Options->bAutomatedImportShouldDetectType = false;
    Options->MeshTypeToImport = EFBXImportType::FBXIT_SkeletalMesh;
    Options->bImportMesh = true;
    Options->bImportAsSkeletal = true;
    Options->bCreatePhysicsAsset = true;

    Options->bImportAnimations = false;
    Options->bImportTextures = true;
    Options->bImportMaterials = true;
    Options->bResetToFbxOnMaterialConflict = true;
    Options->LodNumber = 0;

    //Options->Skeleton = Skeleton;


    Options->SkeletalMeshImportData->ImportTranslation = FVector(0.0f);
    Options->SkeletalMeshImportData->ImportRotation = FRotator(0.0f);
    Options->SkeletalMeshImportData->ImportUniformScale = 1.0f;
    Options->SkeletalMeshImportData->bConvertScene = true;
    Options->SkeletalMeshImportData->bForceFrontXAxis = false;
    Options->SkeletalMeshImportData->bConvertSceneUnit = true;
    Options->SkeletalMeshImportData->bImportMorphTargets = true;

    Options->SkeletalMeshImportData->bTransformVertexToAbsolute = true;
    Options->SkeletalMeshImportData->bBakePivotInVertex = false;
    Options->SkeletalMeshImportData->bImportMeshLODs = 0;
    Options->SkeletalMeshImportData->NormalImportMethod = EFBXNormalImportMethod::FBXNIM_ImportNormals;
    Options->SkeletalMeshImportData->bReorderMaterialToFbxOrder = false;


    UAssetImportTask* ImportTask = UAvatarSDKMetaperson2ImportUtils::CreateImportTask(SrcPath, DstPath, nullptr, Options, bOutSuccess);
    if (!bOutSuccess) {
        return nullptr;
    }
    USkeletalMesh* Result = Cast<USkeletalMesh>(ProcessImportTask(ImportTask, bOutSuccess));

    if (!bOutSuccess) {
        bOutSuccess = false;
        return nullptr;
    }
    if (!Result) {
        UE_LOG(LogTemp, Error, TEXT("Unable to import skeletal mesh"));
        bOutSuccess = false;
        return nullptr;
    }
    UAvatarSdkRaMaterialsManager* MaterialsManager = NewObject< UAvatarSdkRaMaterialsManager>();

    MaterialsManager->Initialize(false);
    MaterialsManager->SetMaterialsToMesh(Result, FPaths::GetPath(DstPath));

    //SetUpMeshMaterials(Result);

    bOutSuccess = true;
    FixMesh(Result);

    return Result;
}

FString UAvatarSDKMetaperson2ImportUtils::GetMaterialName(UMaterialInterface* Material, const int32 MaterialIndex, const FString& SlotName)
{
    FString Res = FString::Printf(TEXT("/Game/MetapersonAvatars/Mat_%d_%s"), MaterialIndex, *SlotName);
    return Res;
}

FString UAvatarSDKMetaperson2ImportUtils::GetTextureName(UTexture* Texture, UMaterialInterface* Material, const FString& MaterialPath, const FString& ParamName)
{
    FString Res;
    if (Texture) {
        Res = Texture->GetName();
    }
    else {
        Res = FString::Printf(TEXT("/Game/MetapersonAvatars/Texture_%s_%s"), *MaterialPath, *ParamName);
    }
    return Res;
}
