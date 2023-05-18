#include "AvatarSDKComponent.h"

UAvatarSDKComponent::UAvatarSDKComponent()
{
}

void UAvatarSDKComponent::LoadAvatar(const FString& ModelPath)
{
	if (!CheckSkeletalMesh()) {
		//log
		return;
	}

}

void UAvatarSDKComponent::DownloadAvatar(const FString& Url)
{
	UAvatarSDKDownloader* Downloader = NewObject<UAvatarSDKDownloader>(this, TEXT("Downloader"));
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
