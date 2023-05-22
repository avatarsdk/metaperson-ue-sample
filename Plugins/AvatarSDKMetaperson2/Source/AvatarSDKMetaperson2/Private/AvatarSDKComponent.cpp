#include "AvatarSDKComponent.h"


UAvatarSDKComponent::UAvatarSDKComponent()
{
}

void UAvatarSDKComponent::LoadAvatar(const FString& ModelPath)
{
	if (!CheckSkeletalMesh()) {
		UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKComponent: LoadAvatar: SkeletalMeshComponent is not valid"));
		return;
	}
	Loader = NewObject<UAvatarSDKLoader>(this, TEXT("Loader"));
	Loader->LoadAvatarAsync(ModelPath, SkeletalMeshComponent, OnAvatarLoaded);
}

void UAvatarSDKComponent::DownloadAvatar(const FString& Url)
{
	Downloader = NewObject<UAvatarSDKDownloader>(this, TEXT("Downloader"));
	Downloader->DownloadFileByUrl(Url, OnAvatarDownloaded, OnAvatarDownloadProgress);
}


bool UAvatarSDKComponent::CheckSkeletalMesh()
{
	if (!IsValid(SkeletalMeshComponent)) {
		AActor* OwnerActor = GetOwner();		
		SkeletalMeshComponent = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
		if (!IsValid(SkeletalMeshComponent)) {
			return false;
		}
	}
	return true;
}
