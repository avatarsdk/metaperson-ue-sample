// Copyright 2023-2024 - Roberto De Ioris

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFBXFunctionLibrary.h"
#include "glTFRuntimeFBXAssetActor.generated.h"

UENUM(BlueprintType)
enum class EglTFRuntimeFBXAssetActorDefaultAnimation : uint8
{
	Default,
	First,
	Last,
	Shortest,
	Longest,
	Random,
	None
};

UCLASS()
class GLTFRUNTIMEFBX_API AglTFRuntimeFBXAssetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AglTFRuntimeFBXAssetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	template<typename T>
	FName GetSafeNodeName(const FglTFRuntimeFBXNode& Node)
	{
		return MakeUniqueObjectName(this, T::StaticClass(), *Node.Name);
	}

	void ProcessNode(USceneComponent* CurrentParentComponent, const FglTFRuntimeFBXNode& FBXNode, const FName SocketName);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	UglTFRuntimeAsset* Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	FglTFRuntimeSkeletalAnimationConfig SkeletalAnimationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	EglTFRuntimeFBXAssetActorDefaultAnimation DefaultAnimation;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "glTFRuntime|FBX")
	TArray<FglTFRuntimeFBXAnim> GetFBXAnimations() const;

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|FBX")
	void PlayFBXAnimation(const FglTFRuntimeFBXAnim& FBXAnim, const bool bLoop);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|FBX")
	bool bDefaultAnimationLoop;


protected:
	TArray<TPair<USkeletalMeshComponent*, FglTFRuntimeFBXNode>> DiscoveredSkeletalMeshes;

	TArray<TPair<USceneComponent*, FName>> DiscoveredAttachments;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "glTFRuntime|FBX")
	USceneComponent* AssetRoot;
};
