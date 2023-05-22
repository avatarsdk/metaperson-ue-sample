#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AvatarSDKData.h"
#include "AvatarSDKMetaperson2.h"
#include "AvatarSDKDownloader.h"
#include "AvatarSDKLoader.h"
#include "AvatarSDKComponent.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AVATARSDKMETAPERSON2_API UAvatarSDKComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UAvatarSDKComponent();

	UFUNCTION(BlueprintCallable, Category = "AvatarSDK|Metaperson 2")
	void LoadAvatar(const FString& ModelPath);

	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoaded OnAvatarLoaded;

	UFUNCTION(BlueprintCallable)
	void DownloadAvatar(const FString& Url);

	UPROPERTY(BlueprintAssignable)
	FOnAvatarDownloaded OnAvatarDownloaded;
	
	UPROPERTY(BlueprintAssignable)
	FOnAvatarDownloadProgress OnAvatarDownloadProgress;

	UPROPERTY(BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	USkeletalMeshComponent* SkeletalMeshComponent;

protected:
	bool CheckSkeletalMesh();
	UPROPERTY()
	UAvatarSDKDownloader* Downloader;
	UPROPERTY()
	UAvatarSDKLoader* Loader;
};