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

bool SaveUObject(UObject* Object, const FString& Path)
{
    if (!FPackageName::IsValidObjectPath(Path))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid UPackage path %s"), *Path);
        return false;
    }

    if (FindPackage(nullptr, *Path) || LoadPackage(nullptr, *Path, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone))
    {
        UE_LOG(LogTemp, Error, TEXT("UPackage %s already exists"), *Path);
        return false;
    }

    UPackage* NewPackage = CreatePackage(*Path);
    if (!NewPackage)
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to create UPackage %s"), *Path);
        return false;
    }

    UPackage* Package = Object->GetPackage();

    if (!Object->Rename(nullptr, NewPackage, REN_DontCreateRedirectors))
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to move UObject %s into UPackage %s"), *Object->GetFullName(), *Path);
        return false;
    }

    if (Package)
    {
        Package->ClearDirtyFlag();
    }

    Object->SetFlags(Object->GetFlags() | RF_Public | RF_Standalone);

    FString NewName = FPackageName::GetShortName(Path);

    if (!Object->Rename(*NewName, nullptr, REN_DontCreateRedirectors))
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to rename UObject %s to %s"), *Object->GetFullName(), *NewName);
        return false;
    }

    if (!UPackage::SavePackage(NewPackage, Object, EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *FPackageName::LongPackageNameToFilename(Path, FPackageName::GetAssetPackageExtension())))
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to save UPackage %s"), *NewPackage->GetPathName());
        return false;
    }

    return true;
}

void FixSkeletalMeshImportData(USkeletalMesh* mesh, USkeleton* oldSkeleton) {
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

void SkeletalMeshSetSoftVertices(USkeletalMesh* mesh, TArray<FSoftSkinVertex>& soft_vertices, int lod_index, int section_index)
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

void AddRootBoneToSkeletalMesh(USkeletalMesh* SkeletalMesh, USkeleton* Skeleton)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid SkeletalMesh"));
        return;
    }

    // Get the skeletal mesh import data
    FSkeletalMeshImportData ImportData;
    SkeletalMesh->LoadLODImportedData(0, ImportData);

    // Create a new root bone
    SkeletalMeshImportData::FBone NewRootBone;
    NewRootBone.Name = TEXT("root");
    NewRootBone.ParentIndex = INDEX_NONE; // No parent, it's the root
    NewRootBone.NumChildren = ImportData.RefBonesBinary.Num();
    NewRootBone.Flags = 0;
    NewRootBone.BonePos.Transform.SetIdentity();

    // Add the new root bone to the import data
    ImportData.RefBonesBinary.Insert(NewRootBone, 0);

    // Update the parent indices of the existing bones
    for (int32 BoneIndex = 1; BoneIndex < ImportData.RefBonesBinary.Num(); ++BoneIndex)
    {
        ImportData.RefBonesBinary[BoneIndex].ParentIndex++;
    }
    //SkeletalMesh->SetSkeleton(Skeleton);
    // Save the modified import data back to the skeletal mesh
    SkeletalMesh->SaveLODImportedData(0, ImportData);

    //...............................................

    // Update the skeletal mesh LOD model
    FSkeletalMeshModel* SkeletalMeshModel = SkeletalMesh->GetImportedModel();
    if (SkeletalMeshModel && SkeletalMeshModel->LODModels.Num() > 0)
    {
        FSkeletalMeshLODModel& LODModel = SkeletalMeshModel->LODModels[0];

        // Add the new root bone to the reference skeleton
        FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();

        TArray<FMeshBoneInfo> boneInfos = RefSkeleton.GetRefBoneInfo();
        FTransformArrayA2 boneTransforms = RefSkeleton.GetRefBonePose();
        RefSkeleton.Empty();
        FReferenceSkeletonModifier RefSkeletonModifier(RefSkeleton, nullptr);

        FMeshBoneInfo NewRootBoneInfo(FName(TEXT("root")), TEXT("root"), INDEX_NONE);
        FTransform NewRootBoneTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector);
        RefSkeletonModifier.Add(NewRootBoneInfo, NewRootBoneTransform);



        //RefSkeleton.RebuildRefSkeleton(Skeleton, true);
        for (int i = 0; i < boneInfos.Num(); i++) {
            boneInfos[i].ParentIndex++;
            RefSkeletonModifier.Add(boneInfos[i], boneTransforms[i]);
        }

        //...............................................
        SkeletalMesh->GetRefBasesInvMatrix().Empty(1);
        SkeletalMesh->CalculateInvRefMatrices();
        SkeletalMesh->RetargetBasePose.Empty();
        //...............................................		

        //// Update the bone map
        TArray<uint16> NewBoneMap;
        NewBoneMap.Add(0); // Root bone index
        for (uint16 BoneIndex = 0; BoneIndex < LODModel.RequiredBones.Num(); ++BoneIndex)
        {
            NewBoneMap.Add(LODModel.RequiredBones[BoneIndex] + 1);
        }


        LODModel.RequiredBones = NewBoneMap;

        // Update the active bone indices
        TArray<uint16> NewActiveBoneIndices;
        NewActiveBoneIndices.Add(0); // Root bone index
        for (uint16 BoneIndex : LODModel.ActiveBoneIndices)
        {
            NewActiveBoneIndices.Add(BoneIndex + 1);
        }
        LODModel.ActiveBoneIndices = NewActiveBoneIndices;

        TArray<TArray<FSoftSkinVertex>> NewSoftVertices;
        int SectionIdx = 0;
        for (FSkelMeshSection& Section : LODModel.Sections) {
            NewSoftVertices.Add(Section.SoftVertices);
            for (FSoftSkinVertex& Vertex : NewSoftVertices[SectionIdx]) {
                for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex) {
                    if (Vertex.InfluenceWeights[InfluenceIndex] > 0) {
                        Vertex.InfluenceBones[InfluenceIndex]++;
                    }
                }
            }
            SkeletalMeshSetSoftVertices(SkeletalMesh, NewSoftVertices[SectionIdx], 0, SectionIdx);
            SectionIdx++;
        }
        //TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

        //SkeletalMesh->ReleaseResources();
        //SkeletalMesh->ReleaseResourcesFence.Wait();

        //SectionIdx = 0;
        ////Update the vertex influences
        //    for (FSkelMeshSection& Section : LODModel.Sections)
        //    {
        //        Section.SoftVertices = NewSoftVertices[SectionIdx++];

        //        for (FSoftSkinVertex& Vertex : Section.SoftVertices)
        //        {
        //            for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex)
        //            {
        //                if (Vertex.InfluenceWeights[InfluenceIndex] > 0 && Vertex.InfluenceBones[InfluenceIndex] != INDEX_NONE)
        //                {
        //                    Vertex.InfluenceBones[InfluenceIndex]++;
        //                    //Vertex.InfluenceWeights[InfluenceIndex] = 0;
        //                }
        //            }
        //        }
        //        Section.CalcMaxBoneInfluences();
        //    }
        //SkeletalMesh->InitResources();
        //SkeletalMesh->RefBasesInvMatrix.Empty();
        //SkeletalMesh->CalculateInvRefMatrices();
        //SkeletalMesh->Build();
        //
        //// Rebuild the skeletal mesh
        //SkeletalMesh->PostEditChange();
        
        //SkeletalMesh->MarkPackageDirty();
    }
}

void ImportAndModifySkeletalMesh(const FString& FbxFilePath, USkeleton* Skeleton)
{
    // Load the FBX file
    UFbxFactory* FbxFactory = NewObject<UFbxFactory>(UFbxFactory::StaticClass());
    FbxFactory->AddToRoot();
    FbxFactory->ImportUI->bImportAsSkeletal = true;
    bool Canceled;
    UObject* ImportedObject = FbxFactory->FactoryCreateFile(
        USkeletalMesh::StaticClass(),
        nullptr,
        FName(*FPaths::GetBaseFilename(FbxFilePath)),
        RF_Public | RF_Standalone,
        FbxFilePath,
        nullptr,
        GWarn,
        Canceled
    );

    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ImportedObject);
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to import SkeletalMesh from FBX file"));
        return;
    }

    // Add a root bone to the skeletal mesh
    AddRootBoneToSkeletalMesh(SkeletalMesh, Skeleton);

    // Save the modified skeletal mesh
    FAssetRegistryModule::AssetCreated(SkeletalMesh);
    SkeletalMesh->MarkPackageDirty();
    SaveUObject(SkeletalMesh, FPaths::Combine(FPaths::GetPath(FbxFilePath), SkeletalMesh->GetName()));
}

void UpdateOverlappingVertices(FSkeletalMeshLODModel& InLODModel)
{
    // clear first
    for (int32 SectionIdx = 0; SectionIdx < InLODModel.Sections.Num(); SectionIdx++)
    {
        FSkelMeshSection& CurSection = InLODModel.Sections[SectionIdx];
        CurSection.OverlappingVertices.Reset();
    }

    for (int32 SectionIdx = 0; SectionIdx < InLODModel.Sections.Num(); SectionIdx++)
    {
        FSkelMeshSection& CurSection = InLODModel.Sections[SectionIdx];
        const int32 NumSoftVertices = CurSection.SoftVertices.Num();

        // Create a list of vertex Z/index pairs
        TArray<FIndexAndZ> VertIndexAndZ;
        VertIndexAndZ.Reserve(NumSoftVertices);
        for (int32 VertIndex = 0; VertIndex < NumSoftVertices; ++VertIndex)
        {
            FSoftSkinVertex& SrcVert = CurSection.SoftVertices[VertIndex];
            new(VertIndexAndZ)FIndexAndZ(VertIndex, SrcVert.Position);
        }
        VertIndexAndZ.Sort(FCompareIndexAndZ());

        // Search for duplicates, quickly!
        for (int32 i = 0; i < VertIndexAndZ.Num(); ++i)
        {
            const uint32 SrcVertIndex = VertIndexAndZ[i].Index;
            const float Z = VertIndexAndZ[i].Z;
            FSoftSkinVertex& SrcVert = CurSection.SoftVertices[SrcVertIndex];

            // only need to search forward, since we add pairs both ways
            for (int32 j = i + 1; j < VertIndexAndZ.Num(); ++j)
            {
                if (FMath::Abs(VertIndexAndZ[j].Z - Z) > THRESH_POINTS_ARE_SAME)
                    break; // can't be any more dups

                const uint32 IterVertIndex = VertIndexAndZ[j].Index;
                FSoftSkinVertex& IterVert = CurSection.SoftVertices[IterVertIndex];
                if (PointsEqual(SrcVert.Position, IterVert.Position))
                {
                    // if so, we add to overlapping vert
                    TArray<int32>& SrcValueArray = CurSection.OverlappingVertices.FindOrAdd(SrcVertIndex);
                    SrcValueArray.Add(IterVertIndex);

                    TArray<int32>& IterValueArray = CurSection.OverlappingVertices.FindOrAdd(IterVertIndex);
                    IterValueArray.Add(SrcVertIndex);
                }
            }
        }
    }
}

void CopyFSkeletalMeshLODModel(FSkeletalMeshLODModel& A, FSkeletalMeshLODModel& B) {




    // Copy Sections
    //B.Sections.Empty();
    for (int32 i = 0; i < A.Sections.Num(); i++) {
        FSkelMeshSection& SectionA = A.Sections[i];
        FSkelMeshSection& SectionB = B.Sections[i];

        SectionB.BoneMap = SectionA.BoneMap;
        //SectionB.OverlappingVertices = SectionA.OverlappingVertices;
        SectionB.SoftVertices = SectionA.SoftVertices;
        SectionB.NumVertices = SectionA.NumVertices;
        SectionB.NumTriangles = SectionA.NumTriangles;

        SectionB.MaterialIndex = SectionA.MaterialIndex;
        SectionB.BaseIndex = SectionA.BaseIndex;
        SectionB.BaseVertexIndex = SectionA.BaseVertexIndex;
        SectionB.SoftVertices.SetNum(SectionA.SoftVertices.Num());
        SectionB.ClothMappingDataLODs.SetNum(SectionA.ClothMappingDataLODs.Num());
        /*SectionB.BoneMap.SetNum(SectionA.BoneMap.Num());
        SectionB.NumVertices = SectionA.NumVertices;
        SectionB.MaxBoneInfluences = SectionA.MaxBoneInfluences;
        SectionB.bUse16BitBoneIndex = SectionA.bUse16BitBoneIndex;
        SectionB.CorrespondClothAssetIndex = SectionA.CorrespondClothAssetIndex;
        SectionB.ClothingData.AssetGuid = SectionA.ClothingData.AssetGuid;
        SectionB.ClothingData.AssetLodIndex = SectionA.ClothingData.AssetLodIndex;
        SectionB.OverlappingVertices = SectionA.OverlappingVertices;
        SectionB.bDisabled = SectionA.bDisabled;
        SectionB.GenerateUpToLodIndex = SectionA.GenerateUpToLodIndex;
        SectionB.OriginalDataSectionIndex = SectionA.OriginalDataSectionIndex;
        SectionB.ChunkedParentSectionIndex = SectionA.ChunkedParentSectionIndex;*/
    }

    // Copy ImportedMeshInfos
    //B.ImportedMeshInfos.Empty();
    //for (int32 i = 0; i < A.ImportedMeshInfos.Num(); i++) {
    //    const FSkelMeshImportedMeshInfo& MeshInfoA = A.ImportedMeshInfos[i];
    //    FSkelMeshImportedMeshInfo MeshInfoB;

    //    MeshInfoB.Name = MeshInfoA.Name;
    //    MeshInfoB.NumVertices = MeshInfoA.NumVertices;
    //    MeshInfoB.StartImportedVertex = MeshInfoA.StartImportedVertex;

    //    B.ImportedMeshInfos.Add(MeshInfoB);
    //}

    //// Copy UserSectionsData
    //B.UserSectionsData.Empty();
    //for (const auto& PairA : A.UserSectionsData) {
    //    const int32 SectionIndex = PairA.Key;
    //    const FSkelMeshSourceSectionUserData& UserDataA = PairA.Value;
    //    FSkelMeshSourceSectionUserData UserDataB;

    //    UserDataB.bRecomputeTangent = UserDataA.bRecomputeTangent;
    //    UserDataB.RecomputeTangentsVertexMaskChannel = UserDataA.RecomputeTangentsVertexMaskChannel;
    //    UserDataB.bCastShadow = UserDataA.bCastShadow;
    //    UserDataB.bVisibleInRayTracing = UserDataA.bVisibleInRayTracing;
    //    UserDataB.CorrespondClothAssetIndex = UserDataA.CorrespondClothAssetIndex;
    //    UserDataB.ClothingData.AssetGuid = UserDataA.ClothingData.AssetGuid;
    //    UserDataB.ClothingData.AssetLodIndex = UserDataA.ClothingData.AssetLodIndex;
    //    UserDataB.bDisabled = UserDataA.bDisabled;
    //    UserDataB.GenerateUpToLodIndex = UserDataA.GenerateUpToLodIndex;

    //    B.UserSectionsData.Add(SectionIndex, UserDataB);
    //}

    // Copy other fields
    B.NumVertices = A.NumVertices;
    // B.NumTexCoords = A.NumTexCoords;
     //B.IndexBuffer = A.IndexBuffer;
     //B.ActiveBoneIndices = A.ActiveBoneIndices;
    B.RequiredBones = A.RequiredBones;
    B.MeshToImportVertexMap = A.MeshToImportVertexMap;
    //B.MaxImportVertex = A.MaxImportVertex;
    //B.RawSkeletalMeshBulkDataID = A.RawSkeletalMeshBulkDataID;
    //B.bIsBuildDataAvailable = A.bIsBuildDataAvailable;
    //B.bIsRawSkeletalMeshBulkDataEmpty = A.bIsRawSkeletalMeshBulkDataEmpty;
    //B.BuildStringID = A.BuildStringID;
}

#if WITH_EDITOR

/**
 * Compare two FSkeletalMeshLODModel objects field by field.
 * Throws an exception if the values are not equal.
 *
 * @param A - The first FSkeletalMeshLODModel object to compare.
 * @param B - The second FSkeletalMeshLODModel object to compare.
 */
void CompareFSkeletalMeshLODModel(FSkeletalMeshLODModel& A, FSkeletalMeshLODModel& B)
{
    // Compare Sections
    if (A.Sections.Num() != B.Sections.Num())
    {
        check(false);
    }
    for (int32 i = 0; i < A.Sections.Num(); i++)
    {
        FSkelMeshSection& SectionA = A.Sections[i];
        FSkelMeshSection& SectionB = B.Sections[i];

        SectionB.BoneMap = SectionA.BoneMap;
        SectionB.OverlappingVertices = SectionA.OverlappingVertices;
        SectionB.SoftVertices = SectionA.SoftVertices;
        SectionB.NumVertices = SectionA.NumVertices;

        if (SectionA.MaterialIndex != SectionB.MaterialIndex ||
            SectionA.BaseIndex != SectionB.BaseIndex ||
            SectionA.NumTriangles != SectionB.NumTriangles ||
            SectionA.bSelected != SectionB.bSelected ||
            SectionA.bRecomputeTangent != SectionB.bRecomputeTangent ||
            SectionA.RecomputeTangentsVertexMaskChannel != SectionB.RecomputeTangentsVertexMaskChannel ||
            SectionA.bCastShadow != SectionB.bCastShadow ||
            SectionA.bVisibleInRayTracing != SectionB.bVisibleInRayTracing ||
            SectionA.bLegacyClothingSection_DEPRECATED != SectionB.bLegacyClothingSection_DEPRECATED ||
            SectionA.CorrespondClothSectionIndex_DEPRECATED != SectionB.CorrespondClothSectionIndex_DEPRECATED ||
            SectionA.BaseVertexIndex != SectionB.BaseVertexIndex ||
            SectionA.SoftVertices.Num() != SectionB.SoftVertices.Num() ||
            SectionA.ClothMappingDataLODs.Num() != SectionB.ClothMappingDataLODs.Num() ||
            SectionA.BoneMap.Num() != SectionB.BoneMap.Num() ||
            SectionA.NumVertices != SectionB.NumVertices ||
            SectionA.MaxBoneInfluences != SectionB.MaxBoneInfluences ||
            SectionA.bUse16BitBoneIndex != SectionB.bUse16BitBoneIndex ||
            SectionA.CorrespondClothAssetIndex != SectionB.CorrespondClothAssetIndex ||
            SectionA.ClothingData.AssetGuid != SectionB.ClothingData.AssetGuid ||
            SectionA.ClothingData.AssetLodIndex != SectionB.ClothingData.AssetLodIndex ||
            SectionA.OverlappingVertices.Num() != SectionB.OverlappingVertices.Num() ||
            SectionA.bDisabled != SectionB.bDisabled ||
            SectionA.GenerateUpToLodIndex != SectionB.GenerateUpToLodIndex ||
            SectionA.OriginalDataSectionIndex != SectionB.OriginalDataSectionIndex ||
            SectionA.ChunkedParentSectionIndex != SectionB.ChunkedParentSectionIndex)
        {

            //check(false);
        }
    }
    return;
    // Compare ImportedMeshInfos
    if (A.ImportedMeshInfos.Num() != B.ImportedMeshInfos.Num())
    {
        //check(false);
    }
    for (int32 i = 0; i < A.ImportedMeshInfos.Num(); i++)
    {
        //const FSkelMeshImportedMeshInfo& MeshInfoA = A.ImportedMeshInfos[i];
        //const FSkelMeshImportedMeshInfo& MeshInfoB = B.ImportedMeshInfos[i];
        //if (MeshInfoA.Name != MeshInfoB.Name ||
        //	MeshInfoA.NumVertices != MeshInfoB.NumVertices ||
        //	MeshInfoA.StartImportedVertex != MeshInfoB.StartImportedVertex)
        //{
        //	//check(false);

        //}
    }

    // Compare UserSectionsData
    if (A.UserSectionsData.Num() != B.UserSectionsData.Num())
    {
        //check(false);

    }
    for (const auto& PairA : A.UserSectionsData)
    {
        const int32 SectionIndex = PairA.Key;
        const FSkelMeshSourceSectionUserData& UserDataA = PairA.Value;
        const FSkelMeshSourceSectionUserData* UserDataB = B.UserSectionsData.Find(SectionIndex);
        if (!UserDataB ||
            UserDataA.bRecomputeTangent != UserDataB->bRecomputeTangent ||
            UserDataA.RecomputeTangentsVertexMaskChannel != UserDataB->RecomputeTangentsVertexMaskChannel ||
            UserDataA.bCastShadow != UserDataB->bCastShadow ||
            UserDataA.bVisibleInRayTracing != UserDataB->bVisibleInRayTracing ||
            UserDataA.CorrespondClothAssetIndex != UserDataB->CorrespondClothAssetIndex ||
            UserDataA.ClothingData.AssetGuid != UserDataB->ClothingData.AssetGuid ||
            UserDataA.ClothingData.AssetLodIndex != UserDataB->ClothingData.AssetLodIndex ||
            UserDataA.bDisabled != UserDataB->bDisabled ||
            UserDataA.GenerateUpToLodIndex != UserDataB->GenerateUpToLodIndex)
        {
            //check(false);

        }
    }

    // Compare other fields
    if (A.NumVertices != B.NumVertices ||
        A.NumTexCoords != B.NumTexCoords ||
        A.IndexBuffer.Num() != B.IndexBuffer.Num() ||
        A.ActiveBoneIndices.Num() != B.ActiveBoneIndices.Num() ||
        A.RequiredBones.Num() != B.RequiredBones.Num() ||
        A.SkinWeightProfiles.Num() != B.SkinWeightProfiles.Num() ||
        A.MeshToImportVertexMap.Num() != B.MeshToImportVertexMap.Num() ||
        A.MaxImportVertex != B.MaxImportVertex ||
        A.RawSkeletalMeshBulkDataID != B.RawSkeletalMeshBulkDataID ||
        A.bIsBuildDataAvailable != B.bIsBuildDataAvailable ||
        A.bIsRawSkeletalMeshBulkDataEmpty != B.bIsRawSkeletalMeshBulkDataEmpty ||
        A.BuildStringID != B.BuildStringID)
    {
        //check(false);
        B.NumVertices = A.NumVertices;
        B.IndexBuffer = A.IndexBuffer;
        B.ActiveBoneIndices = A.ActiveBoneIndices;
        B.RequiredBones = A.RequiredBones;
        B.MeshToImportVertexMap = A.MeshToImportVertexMap;
    }
}
#endif // WITH_EDITOR


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


UObject* Duplicate(UObject* object, char* package_name, char* object_name, bool overwrite)
{

    ObjectTools::FPackageGroupName pgn;
    pgn.ObjectName = UTF8_TO_TCHAR(object_name);
    pgn.GroupName = FString("");
    pgn.PackageName = UTF8_TO_TCHAR(package_name);

    TSet<UPackage*> refused;

    UObject* new_asset = nullptr;

    new_asset = ObjectTools::DuplicateSingleObject(object, pgn, refused, overwrite);

    if (!new_asset)
        return nullptr;

    return new_asset;
}



void SkeletalMeshSetRequiredBones(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> required_bones)
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

void SkeletalMeshSetActiveBoneIndices(USkeletalMesh* mesh, int lod_index, TArray<FBoneIndexType> active_indices)
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

int32_t GetUpdatedBoneIndex(const USkeleton* old_skeleton, const USkeleton* new_skeleton, const TArray<uint16>& old_bone_map, TArray<uint16>& new_bone_map, int32_t index)
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


void SkeletalMeshSetSkeleton(USkeletalMesh* mesh, USkeleton* skeleton)
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

void FixBonesInfluences(USkeletalMesh* Mesh, USkeleton* OldSkeleton)
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


void FixMesh(USkeletalMesh* mesh, USkeleton* Skeleton) {

    //USkeletalMesh* newMesh = DuplicateSkeletalMesh(mesh, TEXT("new_skeletal_mesh"), TEXT("/Game/MetapersonAvatars"));
    USkeletalMesh* newMesh = Cast<USkeletalMesh>( Duplicate(mesh, "/Game/MetapersonAvatars/new_skeletal_mesh", "new_skeletal_mesh", true));

    USkeleton* skeleton = mesh->Skeleton;
    FString NewPackageName = TEXT("/Game/MetapersonAvatars/new_skeleton");
    FString NewSkeletonName = TEXT("new_skeleton");
    UPackage* NewPackage = CreatePackage(*NewPackageName);
    USkeleton* NewSkeleton = NewObject<USkeleton>(NewPackage, *NewSkeletonName, RF_Public | RF_Standalone);

    auto boneTransforms = skeleton->GetReferenceSkeleton().GetRefBonePose();
    auto hipsBoneTransform = boneTransforms[0];
    boneTransforms[0] = FTransform::Identity;
    SkeletonAddBone(NewSkeleton, "root", -1, hipsBoneTransform);

    
    
    for (int32 index = 0; index < skeleton->GetReferenceSkeleton().GetNum(); index++)
    {
        FName boneName = skeleton->GetReferenceSkeleton().GetBoneName(index);
        int32 boneParent = skeleton->GetReferenceSkeleton().GetParentIndex(index);
        
        auto boneTransform = boneTransforms[index];

        FName boneParentName;
        if (boneParent == -1)
        {
            boneParentName = FName(TEXT("root"));
        }
        else
        {
            boneParentName = skeleton->GetReferenceSkeleton().GetBoneName(boneParent);
        }

        
        int32 newBoneParent = NewSkeleton->GetReferenceSkeleton().FindBoneIndex(boneParentName);
        SkeletonAddBone(NewSkeleton, TCHAR_TO_UTF8(*boneName.ToString()), newBoneParent, boneTransform);        
    }

    SkeletalMeshSetSkeleton(newMesh, Skeleton);
    FixSkeletalMeshImportData(newMesh, mesh->Skeleton);
    FixBonesInfluences(newMesh, mesh->Skeleton);
    /*TArray<FSoftSkinVertex> soft_vertices;
    for (int i = 0; i < 14; i++) {
        SkeletalMeshSetSoftVertices(mesh, soft_vertices, 0, i);
    }*/
}

USkeletalMesh* UAvatarSDKMetaperson2ImportUtils::ImportSkeletalMesh(FString& SrcPath, FString& DstPath, bool& bOutSuccess, bool bRtMaterilas)
{
    FglTFRuntimeConfig Config;
    FRotator rot(180.0f, 0.0f, 90.0f);
    FVector tran(0.0f, 0.0f, 0.0f);
    FVector scale(-1.0f, 1.0f, 1.0f);

    FTransform transform(rot, tran, scale);
    Config.BaseTransform = transform;
    Config.TransformBaseType = EglTFRuntimeTransformBaseType::Transform;
    Config.SceneScale = 100.0f;

    FixMesh(ReferenseSM, Skeleton);
    return ReferenseSM;
    auto pkg = TEXT("/Game/MetapersonAvatars/model2");
    auto pkga = TEXT("/Game/MetapersonAvatars/model_a");
    auto pkg_fbx = TEXT("/Game/MetapersonAvatars/model_fbx");
    auto pkg_fbx2 = TEXT("/Game/MetapersonAvatars/model_fbx2");

    FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
    FglTFRuntimeSkeletonConfig SkeletonConfig;
    SkeletonConfig.bAddRootBone = true;
    //SkeletonConfig.ig = true;
    SkeletalMeshConfig.MorphTargetsDuplicateStrategy = EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;

    SkeletalMeshConfig.SkeletonConfig = SkeletonConfig;
    //SkeletalMeshConfig.bIgnoreSkin = true;
    //SkeletonConfig.bFallbackToNodesTree = true;
    SkeletalMeshConfig.NormalsGenerationStrategy = EglTFRuntimeNormalsGenerationStrategy::IfMissing;

    //SkeletalMeshConfig.Skeleton = Skeleton;
    //SkeletalMeshConfig.SaveToPackage = pkg;

    //auto GltfRuntimeAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(SrcPath, false, Config);
    FTransientObjectSaverMaterialNameGenerator MaterialNameGenerator;
    MaterialNameGenerator.BindUFunction(this, "GetMaterialName");
    FTransientObjectSaverTextureNameGenerator TextureNameGenerator;
    TextureNameGenerator.BindUFunction(this, "GetTextureName");
    //////////////////////////////////////////FBX-0
    AddRootBoneToSkeletalMesh(ReferenseSM, Skeleton);
    ReferenseSM->SetSkeleton(Skeleton);
    TArray<FMeshSurface> surfaces;
    TArray<int32> vertexOffsets;
    TArray<int32> indexOffsets;
    TArray<UMaterialInterface*> materials;
    /*USkeletalMesh* newSm = NewObject<USkeletalMesh>();
    newSm->SetSkeleton(Skeleton);
    FRuntimeSkeletalMeshGenerator::DecomposeSkeletalMesh(ReferenseSM, surfaces, vertexOffsets, indexOffsets, materials);
    bool res = FRuntimeSkeletalMeshGenerator::GenerateSkeletalMesh(newSm, surfaces, materials, 1);
    newSm->GetRefBasesInvMatrix().Empty(1);
    newSm->CalculateInvRefMatrices();*/
    UTransientObjectSaverLibrary::SaveTransientSkeletalMesh(ReferenseSM, pkg_fbx2, TEXT(""), TEXT(""), MaterialNameGenerator, TextureNameGenerator);
    //SaveUObject(newSm, pkg_fbx);
    return ReferenseSM;
    ////////////////////////////////////////////FBX
    bool isSkeletal;
    FglTFRuntimeMaterialsConfig materialConfig;
    //SkeletalMeshConfig.Skeleton = Skeleton;
    //SkeletonConfig.bAddRootBone = true;
    //Config.bAsBlob = true;	

    //auto fbxRuntimeAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(fbxPath, false, Config);
    //auto fbxNodes = UglTFRuntimeFBXFunctionLibrary::GetFBXNodesMeshes(fbxRuntimeAsset);
    //TArray<FglTFRuntimeMeshLOD> fbxRuntimeLods;


    //for (int i = 0; i < fbxNodes.Num(); i++) {
    //	fbxRuntimeLods.Add(FglTFRuntimeMeshLOD());
    //	bool fbxSuccess = UglTFRuntimeFBXFunctionLibrary::LoadFBXAsRuntimeLODByNode(fbxRuntimeAsset,
    //		fbxNodes[i],
    //		fbxRuntimeLods[i],
    //		isSkeletal,
    //		materialConfig,
    //		materialConfig);
    //	check(fbxSuccess);
    //}
    //
    //
    //auto fbxSm = fbxRuntimeAsset->LoadSkeletalMeshFromRuntimeLODs(fbxRuntimeLods, -1, SkeletalMeshConfig);
    ////SaveUObject(fbxSm, pkg);
    //UTransientObjectSaverLibrary::SaveTransientSkeletalMesh(fbxSm, pkg_fbx, TEXT(""), TEXT(""), MaterialNameGenerator, TextureNameGenerator);

    //////////////////////////////////////////FBX

    FString glbPath = TEXT("I:\\tasks\\z3d\\task20241003-mpc2ue\\model.glb");
    auto GltfRuntimeAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(glbPath, false, Config);
    GltfRuntimeAsset->AddToRoot();

    TArray<FString> ExcludeNodes;

    //auto skeletalMesh = GltfRuntimeAsset->LoadSkeletalMeshRecursive("", ExcludeNodes, SkeletalMeshConfig);
    auto skeletalMesh = GltfRuntimeAsset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);
    return skeletalMesh;



    //FRuntimeSkeletalMeshGenerator::DecomposeSkeletalMesh(skeletalMesh, surfaces, vertexOffsets, indexOffsets, materials);
    //materials.Empty();

    auto skm2 = ReferenseSM;
    /*USkeletalMesh* skm2 = NewObject<USkeletalMesh>();
    //skm2->SetSkeleton(Skeleton);
    skm2->SetSkeleton(skeletalMesh->GetSkeleton());
    bool res = FRuntimeSkeletalMeshGenerator::GenerateSkeletalMesh(skm2, surfaces, materials, 1);
    check(res);*/

    /*UPackage* Package = Cast<UPackage>(skeletalMesh->GetOuter());
    if (Package && Package != GetTransientPackage())
    {
        const FString Filename = FPackageName::LongPackageNameToFilename(pkg, FPackageName::GetAssetPackageExtension());
        if (UPackage::SavePackage(Package, nullptr, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *Filename))
        {
            FAssetRegistryModule::AssetCreated(skeletalMesh);
            skeletalMesh->MarkPackageDirty();
        }
    }*/

    if (!skeletalMesh->LODSettings) {
        skeletalMesh->LODSettings = NewObject<USkeletalMeshLODSettings>(skeletalMesh);
    }
    ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();

    //SaveUObject(skm2, pkg);

    UTransientObjectSaverLibrary::SaveTransientSkeletalMesh(skeletalMesh, pkga, TEXT(""), TEXT(""), MaterialNameGenerator, TextureNameGenerator);

    auto& dst = skeletalMesh->GetImportedModel()->LODModels[0];
    auto& src = skm2->GetImportedModel()->LODModels[0];

    //CompareFSkeletalMeshLODModel(src, dst);
    //FSkeletalMeshLODModel::CopyStructure(&dst, &src);

    //CopyFSkeletalMeshLODModel(src, dst);
    //UpdateOverlappingVertices(dst);
    FSkeletalMeshImportData LODImportedData;

    skm2->LoadLODImportedData(0, LODImportedData);
    //LODImportedData.RefBonesBinary.Empty();
    //skeletalMesh->SaveLODImportedData(0, LODImportedData);
    //skeletalMesh->Build();

    //SaveUObject(skeletalMesh, pkg);
    //return skeletalMesh;
    UTransientObjectSaverLibrary::SaveTransientSkeletalMesh(skeletalMesh, pkg, TEXT(""), TEXT(""), MaterialNameGenerator, TextureNameGenerator);
    //FLODUtilities::RegenerateLOD(skeletalMesh, TPM.GetRunningTargetPlatform(), 0);
    GltfRuntimeAsset->RemoveFromRoot();
    return skeletalMesh;


    FglTFRuntimeFBXNode RootFBXNode = UglTFRuntimeFBXFunctionLibrary::GetFBXRootNode(GltfRuntimeAsset);
    FglTFRuntimeMeshLOD glTFRuntimeMeshLOD;

    //FglTFRuntimeMaterialsConfig materialConfig;
    //FgltfruntimeStat
    UglTFRuntimeFBXFunctionLibrary::LoadFBXAsRuntimeLODByNode(GltfRuntimeAsset, RootFBXNode, glTFRuntimeMeshLOD, isSkeletal, materialConfig, materialConfig);
    //UglTFRuntimeFBXFunctionLibrary::LoadFBXAsRuntimeLODByNode
    return nullptr;
    //UFbxImportUI* Options = NewObject<UFbxImportUI>();
    //Options->bAutomatedImportShouldDetectType = false;
    //Options->MeshTypeToImport = EFBXImportType::FBXIT_SkeletalMesh;
    //Options->bImportMesh = true;
    //Options->bImportAsSkeletal = true;
    //Options->bCreatePhysicsAsset = true;


    //Options->bImportAnimations = false;
    //Options->bImportTextures = true;
    //Options->bImportMaterials = true;
    //Options->bResetToFbxOnMaterialConflict = true;
    //Options->LodNumber = 0;

    //Options->Skeleton = Skeleton;


    //Options->SkeletalMeshImportData->ImportTranslation = FVector(0.0f);
    //Options->SkeletalMeshImportData->ImportRotation = FRotator(0.0f);
    //Options->SkeletalMeshImportData->ImportUniformScale = 1.0f;
    //Options->SkeletalMeshImportData->bConvertScene = true;
    //Options->SkeletalMeshImportData->bForceFrontXAxis = false;
    //Options->SkeletalMeshImportData->bConvertSceneUnit = true;
    //Options->SkeletalMeshImportData->bImportMorphTargets = true;

    //Options->SkeletalMeshImportData->bTransformVertexToAbsolute = true;
    //Options->SkeletalMeshImportData->bBakePivotInVertex = false;
    //Options->SkeletalMeshImportData->bImportMeshLODs = 0;
    //Options->SkeletalMeshImportData->NormalImportMethod = EFBXNormalImportMethod::FBXNIM_ImportNormals;
    //Options->SkeletalMeshImportData->bReorderMaterialToFbxOrder = false;


    //UAssetImportTask* ImportTask = UAvatarSDKMetaperson2ImportUtils::CreateImportTask(SrcPath, DstPath, nullptr, Options, bOutSuccess);
    //if (!bOutSuccess) {
    //	return nullptr;
    //}
    //USkeletalMesh* Result = Cast<USkeletalMesh>(ProcessImportTask(ImportTask, bOutSuccess));

    //if (!bOutSuccess) {
    //	bOutSuccess = false;
    //	return nullptr;
    //}
    //if (!Result) {
    //	bOutSuccess = false;
    //	return nullptr;
    //}

    //bOutSuccess = true;
    //return Result;
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
