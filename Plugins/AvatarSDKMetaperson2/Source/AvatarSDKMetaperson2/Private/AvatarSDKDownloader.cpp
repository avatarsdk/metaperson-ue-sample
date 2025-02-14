/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKDownloader.h"
#include "AvatarSDKUtilsLibrary.h"
#include "Http.h"
#include "ZipHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Materials/Material.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/EngineVersionComparison.h"

void UAvatarSDKDownloader::DownloadFileByUrl(const FString& Url, FOnAvatarDownloaded OnAvatarDonwnloaded, FOnAvatarDownloadProgress OnAvatarDonwnloadProgress) {
	UE_LOG(LogMetaperson2, Log, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: Url: "), *Url);
	FHttpModule* Http = &FHttpModule::Get();
	auto Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL(Url);
	if (OnAvatarDonwnloadProgress.IsBound())
	{
		UE_LOG(LogMetaperson2, Log, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: Progress will be handled"));
        #if UE_VERSION_OLDER_THAN(5, 4, 0)
        auto& Delegate = Request->OnRequestProgress();        
        #else
        auto& Delegate = Request->OnRequestProgress64();
        #endif
		Delegate.BindLambda([this, OnAvatarDonwnloadProgress](FHttpRequestPtr HttpRequest, int32 BytesSend, int32 InBytesReceived) {
			int32 receivedSize = InBytesReceived;
			FHttpResponsePtr HttpResponse = HttpRequest->GetResponse();
			if (HttpResponse.IsValid())
			{
				int32 totalSize = HttpResponse->GetContentLength();
				float progress = ((float)receivedSize / (float)totalSize) * 100;
                UE_LOG(LogMetaperson2, Warning, TEXT("Download progress = %.2f%%"), progress);
                OnAvatarDonwnloadProgress.Broadcast((int32)progress);
			}
			});
	}
    
    Request->OnProcessRequestComplete().BindLambda([this, Url, OnAvatarDonwnloaded](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            FAvatarData Result = UAvatarSDKUtilsLibrary::GetAvatarData(Url);

            IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
            check(FileManager.CreateDirectoryTree(*Result.DirectoryPath));

            UE_LOG(LogMetaperson2, Log, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: Handling request result..."));
            int32 ReturnCode = HandleResponseCode(Response);
            if (ReturnCode)
            {
                FString Content;
                if (Response) {
                    Content = Response->GetContentAsString();
                }
                UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: Error code %d, Content: %s"), ReturnCode, *Content);
                OnAvatarDonwnloaded.Broadcast(Result);
                return;
            }
            FString Content = Response->GetContentAsString();
            TArray<uint8> ByteData = Response->GetContent();
            if (FPaths::FileExists(Result.ArchivePath))
            {
                UE_LOG(LogMetaperson2, Warning, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: file already exists %s"), *Result.ArchivePath);
                IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
                PlatformFile.DeleteFile(*Result.ArchivePath);
            }
            if (!FFileHelper::SaveArrayToFile(ByteData, *Result.ArchivePath))
            {
                UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: cannot write file %s"), *Result.ArchivePath);
                OnAvatarDonwnloaded.Broadcast(Result);
                return;
            }

            ZipHelper Zip;
            Zip.Unzip(Result.ArchivePath, Result.DirectoryPath, false, true);

            UE_LOG(LogMetaperson2, Log, TEXT("UAvatarSDKDownloader: DownloadFileByUrl: Complete!"));
            OnAvatarDonwnloaded.Broadcast(Result);
        });
    Request->ProcessRequest();
}

int32 UAvatarSDKDownloader::HandleResponseCode(FHttpResponsePtr Response)
{
    const int DEFAULT_ERR_CODE = -1;
    const int NO_RESPONSE = -1;
    if (!Response) {
        return NO_RESPONSE;
    }
    int32 responseCode = Response->GetResponseCode();
    if (!Response.IsValid() || !responseCode)
    {
        return DEFAULT_ERR_CODE;
    }
    else if (responseCode != 200 && responseCode != 201)
    {
        return Response->GetResponseCode();
    }
    return 0;
}
