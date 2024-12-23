#include "AvatarSDKMetaperson2ImportUtils.h"
#include "AssetImportTask.h" 
#include <AssetToolsModule.h>
#include "Factories/FbxImportUI.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "glTFRuntimeFBXFunctionLibrary.h"
#include <glTFRuntimeFunctionLibrary.h>
#include "AssetRegistry/AssetRegistryModule.h"
#include "IMeshBuilderModule.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Materials/MaterialInstanceConstant.h"
#include "TransientObjectSaverLibrary.h"
#include "RuntimeSkeletalMeshGenerator/RuntimeSkeletalMeshGenerator.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
#include "Engine/SkinnedAssetCommon.h"
#endif
#include "LODUtilities.h"
#include <MeshUtilitiesCommon.h>
#include <Factories/FbxFactory.h>
#include <ObjectTools.h>


void UAvatarSDKMetaperson2ImportUtils::FixSkeletalMeshImportData(USkeletalMesh* mesh, USkeleton* oldSkeleton) {
    FSkeletalMeshImportData ImportData;
    mesh->LoadLODImportedData(0, ImportData);
    SkeletalMeshImportData::FBone NewRootBone;
    NewRootBone.Name = TEXT("root");
    NewRootBone.ParentIndex = INDEX_NONE; // No parent, it's the root
    NewRootBone.NumChildren = ImportData.RefBonesBinary.Num();
    NewRootBone.Flags = 0;
    NewRootBone.BonePos.Transform.SetIdentity();

    
    ImportData.RefBonesBinary.Insert(NewRootBone, 0);
    for (int i = 1; i < ImportData.RefBonesBinary.Num(); i++) {
        auto oldBoneIndex = ImportData.RefBonesBinary[i].ParentIndex;
        if (oldBoneIndex == INDEX_NONE) {
            ImportData.RefBonesBinary[i].ParentIndex = 0.;
        }
        else {
            auto oldBoneName = oldSkeleton->GetReferenceSkeleton().GetBoneName(oldBoneIndex);
            auto newBoneIndex = mesh->Skeleton->GetReferenceSkeleton().FindBoneIndex(oldBoneName);
            ImportData.RefBonesBinary[i].ParentIndex = newBoneIndex;
        }
        
    }

    USkeleton* newSkeleton = mesh->Skeleton;
    for (int i = 0; i < ImportData.Influences.Num(); i++) {
        auto oldBoneIndex = ImportData.Influences[i].BoneIndex;
        auto oldBoneName = oldSkeleton->GetReferenceSkeleton().GetBoneName(oldBoneIndex);
        auto newBoneIndex = newSkeleton->GetReferenceSkeleton().FindBoneIndex(oldBoneName); 
        ImportData.Influences[i].BoneIndex = newBoneIndex;
    }


    mesh->SaveLODImportedData(0, ImportData);
}

void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetSoftVertices(USkeletalMesh* mesh, TArray<FSoftSkinVertex>& soft_vertices, int lod_index, int section_index)
{
    FSkeletalMeshModel* resource = mesh->GetImportedModel();


    //if (lod_index < 0 || lod_index >= resource->LODModels.Num())
    //	return PyErr_Format(PyExc_Exception, "invalid LOD index, must be between 0 and %d", resource->LODModels.Num() - 1);

    FSkeletalMeshLODModel& model = resource->LODModels[lod_index];


    /*if (section_index < 0 || section_index >= model.Sections.Num())
        return PyErr_Format(PyExc_Exception, "invalid Section index, must be between 0 and %d", model.Sections.Num() - 1);*/

        // temporarily disable all USkinnedMeshComponent's
    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;
    mesh->PreEditChange(nullptr);
    mesh->ReleaseResources();
    mesh->ReleaseResourcesFence.Wait();

    
    model.Sections[section_index].SoftVertices = soft_vertices;
    model.Sections[section_index].NumVertices = soft_vertices.Num();
    model.Sections[section_index].CalcMaxBoneInfluences();
    
    mesh->RefBasesInvMatrix.Empty();
    mesh->CalculateInvRefMatrices();
    mesh->Build();
    mesh->AssetImportData = nullptr;
#if WITH_EDITOR
    mesh->PostEditChange();
#endif

    mesh->InitResources();
    mesh->MarkPackageDirty();
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

void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetRequiredBones(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> required_bones)
{   

    if (!mesh)
        return;//yErr_Format(PyExc_Exception, "uobject is not a USkeletalMesh");


    FSkeletalMeshModel* resource = mesh->GetImportedModel();


    if (lod_index < 0 || lod_index >= resource->LODModels.Num())
        return;// PyErr_Format(PyExc_Exception, "invalid LOD index, must be between 0 and %d", resource->LODModels.Num() - 1);

    FSkeletalMeshLODModel& model = resource->LODModels[lod_index];

    // temporarily disable all USkinnedMeshComponent's
    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

    mesh->ReleaseResources();
    mesh->ReleaseResourcesFence.Wait();

    model.RequiredBones = required_bones;
    model.RequiredBones.Sort();

    mesh->RefBasesInvMatrix.Empty();
    mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    mesh->PostEditChange();
#endif
    mesh->InitResources();
    mesh->MarkPackageDirty();

}

void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetActiveBoneIndices(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> active_indices)
{
    FSkeletalMeshModel* resource = mesh->GetImportedModel();

    if (lod_index < 0 || lod_index >= resource->LODModels.Num())
        return;// PyErr_Format(PyExc_Exception, "invalid LOD index, must be between 0 and %d", resource->LODModels.Num() - 1);

    FSkeletalMeshLODModel& model = resource->LODModels[lod_index];

    // temporarily disable all USkinnedMeshComponent's
    TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

    mesh->ReleaseResources();
    mesh->ReleaseResourcesFence.Wait();

    model.ActiveBoneIndices = active_indices;
    model.ActiveBoneIndices.Sort();

    mesh->RefBasesInvMatrix.Empty();
    mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    mesh->PostEditChange();
#endif
    mesh->InitResources();
    mesh->MarkPackageDirty();

}

int32_t UAvatarSDKMetaperson2ImportUtils::GetUpdatedBoneIndex(const USkeleton* old_skeleton, const USkeleton* new_skeleton, const TArray<uint16>& old_bone_map, TArray<uint16>& new_bone_map, int32_t index)
{
    // Get the skeleton bone ID from the map
    int32_t true_bone_id = old_bone_map[index];

    // Get the bone name
    
    FName bone_name = old_skeleton->GetReferenceSkeleton().GetBoneName(index);

    // Get the new index
    int32_t new_bone_id = new_skeleton->GetReferenceSkeleton().FindBoneIndex(bone_name);

    // Check if a new mapping is available
    if (new_bone_map.Contains(new_bone_id))
    {
        return new_bone_map.IndexOfByKey(new_bone_id);
    }

    new_bone_map.Add(new_bone_id);
    return new_bone_map.Num() - 1;
}


void UAvatarSDKMetaperson2ImportUtils::SkeletalMeshSetSkeleton(USkeletalMesh* mesh, USkeleton* skeleton)
{
    mesh->ReleaseResources();
    mesh->ReleaseResourcesFence.Wait();

    mesh->Skeleton = skeleton;

    mesh->RefSkeleton = skeleton->GetReferenceSkeleton();

    mesh->RefBasesInvMatrix.Empty();
    mesh->CalculateInvRefMatrices();

#if WITH_EDITOR
    mesh->PostEditChange();
#endif
    mesh->InitResources();
    mesh->MarkPackageDirty();


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
    /*UAvatarSdkRaMaterialsManager* MaterialsManager = NewObject< UAvatarSdkRaMaterialsManager>();

    MaterialsManager->Initialize(this, bRtMaterilas);
    MaterialsManager->SetMaterialsToMesh(Result, FPaths::GetPath(DstPath));*/

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
