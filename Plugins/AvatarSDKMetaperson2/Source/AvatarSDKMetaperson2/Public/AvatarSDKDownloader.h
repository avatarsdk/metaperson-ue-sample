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
#include "AvatarSDKData.h"
#include "AvatarSDKMetaperson2.h"
#include "Interfaces/IHttpRequest.h"
#include "AvatarSDKDownloader.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarDownloaded, const FAvatarData&, AvatarData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarDownloadProgress, int32, Progress);


UCLASS()
class AVATARSDKMETAPERSON2_API UAvatarSDKDownloader : public UObject
{
	GENERATED_BODY()
public:
	void DownloadFileByUrl(const FString& Url, FOnAvatarDownloaded OnAvatarDonwnloaded, FOnAvatarDownloadProgress OnAvatarDonwnloadProgress);

protected:
	int32 HandleResponseCode(FHttpResponsePtr Response);
};