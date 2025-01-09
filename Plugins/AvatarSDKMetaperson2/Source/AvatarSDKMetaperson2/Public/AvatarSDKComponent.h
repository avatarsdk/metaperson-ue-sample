/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

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

	UFUNCTION(BlueprintCallable, Category = "AvatarSDK|Metaperson 2")
	void DownloadAvatar(const FString& Url);

	UPROPERTY(BlueprintAssignable)
	FOnAvatarDownloaded OnAvatarDownloaded;
	
	UPROPERTY(BlueprintAssignable)
	FOnAvatarDownloadProgress OnAvatarDownloadProgress;

	UPROPERTY(BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	USkeletalMeshComponent* FbxMeshComponent;

protected:
	bool CheckSkeletalMesh();
	UPROPERTY()
	UAvatarSDKDownloader* Downloader;
	UPROPERTY()
	UAvatarSDKLoader* Loader;
};