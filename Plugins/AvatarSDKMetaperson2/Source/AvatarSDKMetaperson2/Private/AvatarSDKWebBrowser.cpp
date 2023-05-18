#include "AvatarSDKWebBrowser.h"
//#include "SWebBrowser.h"

void UAvatarSDKWebBrowser::Init()
{
    //TODO: Remove This:
    ClientId = TEXT("fO3tpBCOGfEWM5CqbKUAeJ3OKiZjA4i8knIbjSMg");
    ClientSecret = TEXT("qEZoR0x0tTIpxjN1S351Xvzyo62cAdVahi3Z7xfOBZtWXyvN1P7hMU54mUiVzo9zRRNEk1VqNX7PlthQfm9iAV0SOM1BTT835MQ7dVBHIvmbHURxBgcvRHxOGGREuIY7");

    const FString ProxyObjectName = TEXT("avatarsdk_proxy");
    if (!CallbackProxy) {
        CallbackProxy = NewObject<UAvatarSDKBrowserCallbackProxy>(this, *ProxyObjectName);
        CallbackProxy->SetOnAvatarExportedDelegate(OnAvatarExported);
    }
    WebBrowserWidget->BindUObject(ProxyObjectName, CallbackProxy);
	ExecuteJavascript(GetJavascriptCode());
}

FString UAvatarSDKWebBrowser::GetJavascriptCode() const
{
    FString JsCode = FString::Printf(TEXT(
        "const CLIENT_ID = '%s';"
        "const CLIENT_SECRET = '%s';"

        "function onWindowMessage(evt) {"
        "if (evt.type === 'message') {"
        "if (evt.data?.source === 'metaperson_editor') {"
        "let data = evt.data;"
        "let evtName = data?.eventName;"
        "if (evtName === 'unity_loaded') {"
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

        "}"
        "window.addEventListener('message', onWindowMessage);"
    ), *ClientId, *ClientSecret);
    return JsCode;
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
