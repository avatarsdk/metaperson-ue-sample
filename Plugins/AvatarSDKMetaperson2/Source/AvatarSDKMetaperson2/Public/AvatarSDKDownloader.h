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