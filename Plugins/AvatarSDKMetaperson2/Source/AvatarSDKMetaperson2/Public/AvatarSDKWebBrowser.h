#pragma once

#include "CoreMinimal.h"
#include "WebBrowser.h"
#include "SWebBrowser.h"
#include "AvatarSDKWebBrowser.generated.h"

UCLASS()
class UAvatarSDKBrowserCallbackProxy : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Avatar SDK|Metaperson 2")
	void AvatarExportCallback(FString Url);
};

UCLASS()
class UAvatarSDKWebBrowser : public UWebBrowser
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Avatar SDK|Metaperson 2")
	void Init();
	UPROPERTY(EditAnywhere)
	FString ClientId;
	UPROPERTY(EditAnywhere)
	FString ClientSecret;
protected:
	UPROPERTY()
	UAvatarSDKBrowserCallbackProxy* CallbackProxy;
	const FString StartUrl = TEXT("https://metaperson.avatarsdk.com/iframe.html");	

	FString GetJavascriptCode() const;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	UFUNCTION()
	void OnUrlChangedHandler(const FText& Text);
	

	
};