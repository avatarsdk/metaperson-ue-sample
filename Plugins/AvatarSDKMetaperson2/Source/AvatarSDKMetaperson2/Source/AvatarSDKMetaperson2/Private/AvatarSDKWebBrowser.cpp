/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKWebBrowser.h"
#include "AvatarSDKMetaperson2.h"
#include "AvatarSDKRuntimeSettings.h"
#include <AndroidUtils.h>

UAvatarSDKWebBrowser::UAvatarSDKWebBrowser() {
    ReadParametersFromSettings = true;
}
void UAvatarSDKWebBrowser::Init()
{
    if (ReadParametersFromSettings) {
        const UAvatarSDKRuntimeSettings* AvatarSDKRuntimeSettings = GetDefault<UAvatarSDKRuntimeSettings>();
        if (AvatarSDKRuntimeSettings)
        {
            ClientId = AvatarSDKRuntimeSettings->ClientId;
            ClientSecret = AvatarSDKRuntimeSettings->ClientSecret;
        }
        else {			
            UE_LOG(LogMetaperson2, Error, TEXT("UAvatarSDKWebBrowser: Init: Could not read settings"));
        }
    }

    if (ClientId.IsEmpty() || ClientSecret.IsEmpty()) {
        if (OnBrowserError.IsBound()) {
            OnBrowserError.Broadcast(TEXT("ClientId/ClientSecret parameters are empty. Please check your settings in Edit->Project Settings->Plugins->Avatar SDK Metaperson 2"));
        }
    }
    
    const FString ProxyObjectName = TEXT("avatarsdk_proxy");
    if (!CallbackProxy) {
        CallbackProxy = NewObject<UAvatarSDKBrowserCallbackProxy>(this, *ProxyObjectName);
        CallbackProxy->SetOnAvatarExportedDelegate(OnAvatarExported);
    }
    
    WebBrowserWidget->BindUObject(ProxyObjectName, CallbackProxy);
    this->bSupportsTransparency = true;


    
#if PLATFORM_ANDROID    
    {        
        ExecuteJavascript(GetJavascriptCodeMobile());
    }    
#else 
    ExecuteJavascript(GetJavascriptCodeDesktop());
#endif
}

FString UAvatarSDKWebBrowser::GetJavascriptCodeMobile() const
{
    return GetJavascriptCode(TEXT("mobile_loaded"));
}

FString UAvatarSDKWebBrowser::GetJavascriptCode(const FString& EventName) const
{
    FString JsCode = FString::Printf(TEXT(
        "const CLIENT_ID = '%s';"
        "const CLIENT_SECRET = '%s';"

        "function onWindowMessage(evt) {"
        "if (evt.type === 'message') {"
        "if (evt.data?.source === 'metaperson_creator') {"
        "let data = evt.data;"
        "let evtName = data?.eventName;"
        "if (evtName === '%s') {"
        "onUnityLoaded(evt, data);"
        "} else if (evtName === 'model_exported') {"
        "window.ue.avatarsdk_proxy.avatarexportcallback(event.data.url);"
        "}"
        "}"
        "}"
        "}"

        "function onUnityLoaded(evt, data) {"
        "let authenticationMessage = {"
        "'eventName': 'authenticate',"
        "'clientId': CLIENT_ID,"
        "'clientSecret': CLIENT_SECRET,"
        "'exportTemplateCode': '',"
        "};"
        "evt.source.postMessage(authenticationMessage, '*');"
        "let exportParametersMessage = {"
        "'eventName': 'set_export_parameters',"
        "'format' : 'glb',"
        "'lod' : 1,"
        "'textureProfile' : '1K.png'"
        "};"
        "evt.source.postMessage(exportParametersMessage, '*');"

        "let uiMessage = {"
        "'eventName': 'set_ui_parameters',"
        "'theme': 'light'"
        "};"
        "evt.source.postMessage(uiMessage, '*');"

        "}"
        "window.addEventListener('message', onWindowMessage);"
    ), *ClientId, *ClientSecret, *EventName);
    return JsCode;
}

FString UAvatarSDKWebBrowser::GetJavascriptCodeDesktop() const
{
    return GetJavascriptCode(TEXT("unity_loaded"));   
}

TSharedRef<SWidget> UAvatarSDKWebBrowser::RebuildWidget()
{
	InitialURL = StartUrl;
	OnUrlChanged.AddUniqueDynamic(this, &UAvatarSDKWebBrowser::OnUrlChangedHandler);
	auto Result = Super::RebuildWidget();
	return Result;
}

void UAvatarSDKWebBrowser::OnUrlChangedHandler(const FText& Text)
{
	Init();
}

void UAvatarSDKBrowserCallbackProxy::AvatarExportCallback(FString Url)
{
    if (OnAvatarExported.IsBound()) {
        OnAvatarExported.Broadcast(Url);
    }
}

void UAvatarSDKBrowserCallbackProxy::SetOnAvatarExportedDelegate(FOnAvatarExported Delegate)
{
    OnAvatarExported = Delegate;
}
