/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKComponent.h"
#include "Components/SkeletalMeshComponent.h"


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
