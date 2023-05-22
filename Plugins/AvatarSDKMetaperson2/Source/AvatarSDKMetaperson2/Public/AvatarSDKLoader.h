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
	void LoadAvatarAsync(const FString& GlbPath, USkeletalMeshComponent* InSkeletalMeshComponent, FOnAvatarLoaded AvatarLoadedDelegate);
	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoaded OnAvatarLoaded;
protected:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY()
	class UglTFRuntimeAsset* GltfRuntimeAsset;
	FglTFRuntimeSkeletalMeshAsync GlTFRuntimeSkeletalMeshDelegate;
	UFUNCTION()
	void GltfRuntimeSkeletalMeshCallback(USkeletalMesh* InSkeletalMesh);
};