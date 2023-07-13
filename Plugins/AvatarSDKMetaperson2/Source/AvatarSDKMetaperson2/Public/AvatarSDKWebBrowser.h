/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2022
 */

#pragma once

#include "CoreMinimal.h"
#include "WebBrowser.h"
#include "SWebBrowser.h"
#include "AvatarSDKWebBrowser.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarExported, const FString&, Url);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBrowserError, const FString&, Message);


UCLASS()
class AVATARSDKMETAPERSON2_API UAvatarSDKBrowserCallbackProxy : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AvatarSDK|Metaperson 2")
	void AvatarExportCallback(FString Url);
	void SetOnAvatarExportedDelegate(FOnAvatarExported Delegate);
protected:
	UPROPERTY()
	FOnAvatarExported OnAvatarExported;
};

UCLASS()
class AVATARSDKMETAPERSON2_API UAvatarSDKWebBrowser : public UWebBrowser
{
	GENERATED_BODY()
public:
	UAvatarSDKWebBrowser();
	UFUNCTION(BlueprintCallable, Category = "AvatarSDK|Metaperson 2")
	void Init();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	FString ClientId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	FString ClientSecret;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AvatarSDK|Metaperson 2")
	bool ReadParametersFromSettings;

	UPROPERTY(BlueprintAssignable)
	FOnAvatarExported OnAvatarExported;
	UPROPERTY(BlueprintAssignable)
	FOnBrowserError OnBrowserError;
protected:
	UPROPERTY()
	UAvatarSDKBrowserCallbackProxy* CallbackProxy;
	//const FString StartUrl = TEXT("https://metaperson.avatarsdk.com/iframe.html");
	const FString StartUrl = TEXT("https://metaperson.avatarsdk.com/dev/new_haircuts/v3/iframe.html");

	FString GetJavascriptCode() const;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	UFUNCTION()
	void OnUrlChangedHandler(const FText& Text);
};